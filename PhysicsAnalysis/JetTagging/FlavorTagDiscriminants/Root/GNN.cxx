/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/GNN.h"
#include "FlavorTagDiscriminants/BTagTrackIpAccessor.h"
#include "FlavorTagDiscriminants/OnnxUtil.h"

#include "xAODBTagging/BTagging.h"
#include "xAODJet/JetContainer.h"

#include "PathResolver/PathResolver.h"
#include "lwtnn/parse_json.hh"

#include <fstream>

namespace {
  const std::string jetLinkName = "jetLink";
  const std::string unsafeEnvVar = "ALLOW_FTAG_TO_BREAK_THE_EDM";
  const std::string envVarTrue = "Yes please.";
}

namespace FlavorTagDiscriminants {

  GNN::GNN(const std::string& nnFile,
           const FlipTagConfig& flip_config,
           const std::map<std::string, std::string>& variableRemapping,
           const TrackLinkType trackLinkType,
           float defaultOutputValue,
           bool decorate_tracks):
    m_onnxUtil(nullptr),
    m_jetLink(jetLinkName),
    m_defaultValue(defaultOutputValue),
    m_decorate_tracks(decorate_tracks)
  {
    // track decoration is allowed only for non-production builds
    if (m_decorate_tracks) {
      const char* break_edm = std::getenv(unsafeEnvVar.c_str());
      if (break_edm == nullptr || std::string(break_edm) != envVarTrue) {
        throw std::runtime_error(
          "Flavor tagging is trying to break the EDM!\n\n"
          "You are trying to break the EDM!!! "
          "We'll let you do this, if you set an environement variable. "
          "Which one? Not telling. "
          "Now go think about what you just did.\n\n");
      }
    }

    std::string fullPathToOnnxFile = PathResolverFindCalibFile(nnFile);
    m_onnxUtil = std::make_shared<OnnxUtil>(fullPathToOnnxFile);

    std::string gnn_config_str = m_onnxUtil->getMetaData("gnn_config");
    m_config_gnn = m_onnxUtil->get_config();

    std::stringstream gnn_config_stream;

    // adding "outputs" to the config if it is not present or lwtnn will fail
    if (m_onnxUtil->getOnnxModelVersion() != OnnxModelVersion::V0){
      nlohmann::json j = nlohmann::json::parse(gnn_config_str);
      j["outputs"] = nlohmann::json::object();

      // dump the json to a stringstream
      gnn_config_stream << j.dump();
    } else {
      gnn_config_stream << gnn_config_str;
    }
    auto config = lwt::parse_json_graph(gnn_config_stream);

    auto [inputs, track_sequences, options] = dataprep::createGetterConfig(
        config, flip_config, variableRemapping, trackLinkType);

    auto [vb, vj, ds] = dataprep::createBvarGetters(inputs);
    m_varsFromBTag = vb;
    m_varsFromJet = vj;
    m_dataDependencyNames = ds;

    auto [tsb, td, rt] = dataprep::createTrackGetters(
      track_sequences, options);
    m_trackSequenceBuilders = tsb;
    m_dataDependencyNames += td;

    FlavorTagDiscriminants::FTagDataDependencyNames dd;
    std::set<std::string> rd;

    std::tie(m_decorators_float, m_decorators_vecchar, m_decorators_vecfloat, m_decorators_tracklinks, m_decorators_track_char, m_decorators_track_float, dd, rd) = dataprep::createGNDecorators(
      m_config_gnn, options);

    m_dataDependencyNames += dd;

    rd.merge(rt);
    dataprep::checkForUnusedRemaps(options.remap_scalar, rd);
  }

  GNN::GNN(GNN&&) = default;
  GNN::GNN(const GNN&) = default;
  GNN::~GNN() = default;

  void GNN::decorate(const xAOD::BTagging& btag) const {
    auto jetLink = m_jetLink(btag);
    if (!jetLink.isValid()) {
      throw std::runtime_error("invalid jetLink");
    }
    const xAOD::Jet& jet = **jetLink;
    decorate(jet, btag);
  }
  void GNN::decorate(const xAOD::Jet& jet) const {
    decorate(jet, jet);
  }
  void GNN::decorateWithDefaults(const xAOD::Jet& jet) const {
    for (const auto& dec: m_decorators_float) {
      dec.second(jet) = m_defaultValue;
    }
  }

  void GNN::decorate(const xAOD::Jet& jet, const SG::AuxElement& btag) const {

    using namespace internal;

    std::map<std::string, input_pair> gnn_input;

    std::vector<float> jet_feat;
    for (const auto& getter: m_varsFromBTag) {
      jet_feat.push_back(getter(btag).second);
    }
    for (const auto& getter: m_varsFromJet) {
      jet_feat.push_back(getter(jet).second);
    }
    std::vector<int64_t> jet_feat_dim = {1, static_cast<int64_t>(jet_feat.size())};

    input_pair jet_info (jet_feat, jet_feat_dim);
    gnn_input.insert({"jet_features", jet_info});

    // Only one track sequence is allowed because the tracks are declared
    // outside the loop over sequences.
    // Having more than one sequence would overwrite them.
    // These are only used outside the loop to write the track links.
    if (m_trackSequenceBuilders.size() > 1) {
      throw std::runtime_error("Only one track sequence is supported");
    }
    Tracks input_tracks;
    for (const auto& builder: m_trackSequenceBuilders) {
      std::vector<float> track_feat; // (#tracks, #feats).flatten
      int num_track_vars = static_cast<int>(builder.sequencesFromTracks.size());
      int num_tracks = 0;

      Tracks sorted_tracks = builder.tracksFromJet(jet, btag);
      input_tracks = builder.flipFilter(sorted_tracks, jet);

      int track_var_idx=0;
      for (const auto& seq_builder: builder.sequencesFromTracks) {
        auto double_vec = seq_builder(jet, input_tracks).second;

        if (track_var_idx==0){
          num_tracks = static_cast<int>(double_vec.size());
          track_feat.resize(num_tracks * num_track_vars);
        }

        // need to transpose + flatten
        for (unsigned int track_idx=0; track_idx<double_vec.size(); track_idx++){
          track_feat.at(track_idx*num_track_vars + track_var_idx)
            = double_vec.at(track_idx);
        }
        track_var_idx++;
      }
      std::vector<int64_t> track_feat_dim = {num_tracks, num_track_vars};

      input_pair track_info (track_feat, track_feat_dim);
      gnn_input.insert({"track_features", track_info});
    }

    auto [out_f, out_vc, out_vf]
      = m_onnxUtil->runInference(gnn_input);

    // with old metadata
    if (m_onnxUtil->getOnnxModelVersion() == OnnxModelVersion::V0){
      for (const auto& dec: m_decorators_float) {
        if (out_vf.at(dec.first).size() != 1){
          throw std::logic_error(
            "expected vectors of length 1 for float decorators");
        }
        dec.second(btag) = out_vf.at(dec.first).at(0);
      }
    }

    else {
      for (const auto& dec: m_decorators_float) {
        dec.second(btag) = out_f.at(dec.first);
      }

      for (const auto& dec: m_decorators_vecchar) {
        dec.second(btag) = out_vc.at(dec.first);
      }

      for (const auto& dec: m_decorators_vecfloat) {
        dec.second(btag) = out_vf.at(dec.first);
      }

      for (const auto& dec: m_decorators_tracklinks) {
        internal::TrackLinks links;
        for (const xAOD::TrackParticle* it: input_tracks) {
          TrackLinks::value_type link;

          const auto* itc = dynamic_cast<const xAOD::TrackParticleContainer*>(
            it->container());
          link.toIndexedElement(*itc, it->index());
          links.push_back(link);
        }
        dec.second(btag) = links;
      }

      if (m_decorate_tracks) {
        for (const auto& dec: m_decorators_track_char) {
          std::vector<char>& values = out_vc.at(dec.first);
          if (values.size() != input_tracks.size()) {
            throw std::logic_error("Track aux task output size doesn't match the size of track list");
          }
          Tracks::const_iterator it = input_tracks.begin();
          std::vector<char>::const_iterator ival = values.begin();
          for (; it != input_tracks.end() && ival != values.end(); it++, ival++) {
            dec.second(**it) = *ival;
          }
        }

        for (const auto& dec: m_decorators_track_float) {
          std::vector<float>& values = out_vf.at(dec.first);
          if (values.size() != input_tracks.size()) {
            throw std::logic_error("Track aux task output size doesn't match the size of track list");
          }
          Tracks::const_iterator it = input_tracks.begin();
          std::vector<float>::const_iterator ival = values.begin();
          for (; it != input_tracks.end() && ival != values.end(); it++, ival++) {
            dec.second(**it) = *ival;
          }
        }
      }
    }
  } // end of decorate()

  // Dependencies
  std::set<std::string> GNN::getDecoratorKeys() const {
    return m_dataDependencyNames.bTagOutputs;
  }
  std::set<std::string> GNN::getAuxInputKeys() const {
    return m_dataDependencyNames.bTagInputs;
  }
  std::set<std::string> GNN::getConstituentAuxInputKeys() const {
    return m_dataDependencyNames.trackInputs;
  }

}
