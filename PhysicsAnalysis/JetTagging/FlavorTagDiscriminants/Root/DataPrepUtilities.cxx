/*
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/DataPrepUtilities.h"
#include "FlavorTagDiscriminants/BTagTrackIpAccessor.h"
#include "FlavorTagDiscriminants/customGetter.h"

#include "xAODBTagging/BTaggingUtilities.h"

namespace {

  // define a regex literal operator
  std::regex operator "" _r(const char* c, size_t /* length */) {
    return std::regex(c);
  }

  using FlavorTagDiscriminants::EDMType;
  using FlavorTagDiscriminants::SortOrder;
  using FlavorTagDiscriminants::TrackSelection;
  using FlavorTagDiscriminants::FTagTrackSequenceConfig;
  using FlavorTagDiscriminants::FTagInputConfig;
  using FlavorTagDiscriminants::FTagTrackInputConfig;
  using FlavorTagDiscriminants::FlipTagConfig;
  // ____________________________________________________________________
  // High level adapter stuff
  //
  // We define a few structures to map variable names to type, default
  // value, etc. These are only used by the high level interface.
  //
  typedef std::vector<std::pair<std::regex, EDMType> > TypeRegexes;
  typedef std::vector<std::pair<std::regex, std::string> > StringRegexes;
  typedef std::vector<std::pair<std::regex, SortOrder> > SortRegexes;
  typedef std::vector<std::pair<std::regex, TrackSelection> > TrkSelRegexes;

  // Function to map the regular expressions + the list of inputs to a
  // list of variable configurations.
  std::vector<FTagInputConfig> get_input_config(
    const std::vector<std::string>& variable_names,
    const TypeRegexes& type_regexes,
    const StringRegexes& default_flag_regexes);

  // Since the names of the inputs are stored in the NN config, we
  // also allow some user-configured remapping. Items in replaced_vars
  // are removed as they are used.
  void remap_inputs(std::vector<lwt::Input>& nn,
                    std::map<std::string, std::string>& replaced_vars,
                    std::map<std::string, double>& defaults);

  // Function to map the regex + list of inputs to variable config,
  // this time for sequence inputs.
  std::vector<FTagTrackSequenceConfig> get_track_input_config(
    const std::vector<std::pair<std::string, std::vector<std::string>>>& names,
    const TypeRegexes& type_regexes,
    const SortRegexes& sort_regexes,
    const TrkSelRegexes& select_regexes,
    const std::regex& flip_re,
    const FlipTagConfig& flip_config);

  // replace strings for flip taggers
  void rewriteFlipConfig(lwt::GraphConfig&, const StringRegexes&);


  //_______________________________________________________________________
  // Implementation of the above functions
  //

  template <typename T>
  T match_first(const std::vector<std::pair<std::regex, T> >& regexes,
                const std::string& var_name,
                const std::string& context) {
    for (const auto& pair: regexes) {
      if (std::regex_match(var_name, pair.first)) {
        return pair.second;
      }
    }
    throw std::logic_error(
      "no regex match found for input variable '" + var_name + "' in "
      + context);
  }


  // functions to rewrite input names
  std::string sub_first(const StringRegexes& res,
                        const std::string& var_name,
                        const std::string& context) {
    for (const auto& pair: res) {
      const std::regex& re = pair.first;
      const std::string& fmt = pair.second;
      if (std::regex_match(var_name, re)) {
        return std::regex_replace(var_name, re, fmt);
      }
    }
    throw std::logic_error(
      "no regex match found for variable '" + var_name + "' while " + context);
  }

  std::vector<FTagInputConfig> get_input_config(
    const std::vector<std::string>& variable_names,
    const TypeRegexes& type_regexes,
    const StringRegexes& default_flag_regexes)
  {
    std::vector<FTagInputConfig> inputs;
    for (const auto& var: variable_names) {
      FTagInputConfig input;
      input.name = var;
      input.type = match_first(type_regexes, var, "type matching");
      input.default_flag = sub_first(default_flag_regexes, var,
                                     "default matching");

      inputs.push_back(input);
    }
    return inputs;
  }


  // do some input variable magic in case someone asked
  void remap_inputs(std::vector<lwt::Input>& nn,
                    std::map<std::string, std::string>& replaced_vars,
                    std::map<std::string, double>& defaults) {
    // keep track of the new default values, and which values they
    // were moved from
    std::map<std::string, double> new_defaults;
    std::set<std::string> moved_defaults;
    for (lwt::Input& input: nn) {
      std::string nn_name = input.name;
      auto replacement_itr = replaced_vars.find(nn_name);
      if (replacement_itr != replaced_vars.end()) {
        std::string new_name = replacement_itr->second;
        input.name = new_name;
        if (defaults.count(nn_name)) {
          new_defaults[new_name] = defaults.at(nn_name);
          moved_defaults.insert(nn_name);
        }
        replaced_vars.erase(replacement_itr);
      }
    }
    for (const auto& new_default: new_defaults) {
      defaults[new_default.first] = new_default.second;
      // if something was a new default we don't want to delete it
      // below.
      moved_defaults.erase(new_default.first);
    }
    // delete anything that was moved but wasn't assigned to
    for (const auto& moved: moved_defaults) {
      defaults.erase(moved);
    }
  }


  std::vector<FTagTrackSequenceConfig> get_track_input_config(
    const std::vector<std::pair<std::string, std::vector<std::string>>>& names,
    const TypeRegexes& type_regexes,
    const SortRegexes& sort_regexes,
    const TrkSelRegexes& select_regexes,
    const std::regex& flip_re,
    const FlipTagConfig& flip_config) {
    std::vector<FTagTrackSequenceConfig> nodes;
    for (const auto& name_node: names) {
      FTagTrackSequenceConfig node;
      node.name = name_node.first;
      node.order = match_first(sort_regexes, name_node.first,
                               "track order matching");
      node.selection = match_first(select_regexes, name_node.first,
                                   "track selection matching");
      for (const auto& varname: name_node.second) {
        FTagTrackInputConfig input;
        input.name = varname;
        input.type = match_first(type_regexes, varname,
                                 "track type matching");

        input.flip_sign=false;
        if ((flip_config != FlipTagConfig::STANDARD) && std::regex_match(varname, flip_re)){
          input.flip_sign=true;
        }
        
        node.inputs.push_back(input);
      }
      nodes.push_back(node);
    }
    return nodes;
  }


  void rewriteFlipConfig(lwt::GraphConfig& config,
                         const StringRegexes& res){
    std::string context = "building negative tag b-btagger";
    for (auto& node: config.inputs) {
      for (auto& var: node.variables) {
        var.name = sub_first(res, var.name, context);
      }
      std::map<std::string, double> new_defaults;
      for (auto& pair: node.defaults) {
        new_defaults[sub_first(res, pair.first, context)] = pair.second;
      }
      node.defaults = new_defaults;
    }
    std::map<std::string, lwt::OutputNodeConfig> new_outputs;
    for (auto& pair: config.outputs) {
      new_outputs[sub_first(res, pair.first, context)] = pair.second;
    }
    config.outputs = new_outputs;
  }


  StringRegexes getFlipConverters(const FlipTagConfig& flip_config) {

    // determine name based on flip config
    std::string flip_name = "";
    if (flip_config == FlipTagConfig::FLIP_SIGN) {
      flip_name = "Flip";
    }
    if (flip_config == FlipTagConfig::NEGATIVE_IP_ONLY) {
      flip_name = "Neg";
    }
    if (flip_config == FlipTagConfig::SIMPLE_FLIP) {
      flip_name = "Simple";
    }

    // we rewrite the inputs if we're using flip taggers
    StringRegexes flip_converters {
      {"(GN1[^_]*|GN2[^_]*)"_r, "$1" + flip_name},
      {"(GN1[^_]*|GN2[^_]*)_(.*)"_r, "$1" + flip_name + "_$2"},
      {"(IP[23]D)_(.*)"_r, "$1Neg_$2"},
      {"(rnnip|dips[^_]*)_(.*)"_r, "$1flip_$2"},
      {"(JetFitter|SV1|JetFitterSecondaryVertex)_(.*)"_r, "$1Flip_$2"},
      {"(rnnip|dips[^_]*)"_r, "$1flip"},
      {"^(DL1|DL1r[^_]*|DL1rmu|DL1d[^_]*)$"_r, "$1" + flip_name},
      {"pt|abs_eta|eta"_r, "$&"},
      {"softMuon.*|smt.*"_r, "$&"}
    };

    return flip_converters;
  }

}

// __________________________________________________________________________
// Start of functions that are used outside this file


namespace FlavorTagDiscriminants {

  FTagOptions::FTagOptions()
    : track_prefix ("btagIp_"),
      flip (FlipTagConfig::STANDARD),
      track_link_name ("BTagTrackToJetAssociator"),
      track_link_type (TrackLinkType::TRACK_PARTICLE),
      default_output_value (NAN),
      invalid_ip_key ("invalidIp")
  {
  }

  // ________________________________________________________________________
  // Internal code
  namespace internal {

    // Track Getter Class
    TracksFromJet::TracksFromJet(
      SortOrder order,
      TrackSelection selection,
      const FTagOptions& options):
      m_trackSortVar(get::trackSortVar(order, options)),
      m_trackFilter(get::trackFilter(selection, options).first){

      // We have several ways to get tracks: either we retrieve an
      // IParticleContainer and cast the pointers to TrackParticle, or
      // we retrieve a TrackParticleContainer directly. Unfortunately
      // the way tracks are stored isn't consistent across the EDM, so
      // we allow configuration for both setups.
      //
      if (options.track_link_type == TrackLinkType::IPARTICLE) {
        SG::AuxElement::ConstAccessor<PartLinks> acc(options.track_link_name);
        m_associator = [acc](const SG::AuxElement& btag) -> TPV {
          TPV tracks;
          for (const ElementLink<IPC>& link: acc(btag)) {
            if (!link.isValid()) {
              throw std::logic_error("invalid particle link");
            }
            const auto* trk = dynamic_cast<const xAOD::TrackParticle*>(*link);
            if (!trk) {
              throw std::logic_error("iparticle does not cast to Track");
            }
            tracks.push_back(trk);
          }
          return tracks;
        };
      } else if (options.track_link_type == TrackLinkType::TRACK_PARTICLE){
        SG::AuxElement::ConstAccessor<TrackLinks> acc(options.track_link_name);
        m_associator = [acc](const SG::AuxElement& btag) -> TPV {
          TPV tracks;
          for (const ElementLink<TPC>& link: acc(btag)) {
            if (!link.isValid()) {
              throw std::logic_error("invalid track link");
            }
            tracks.push_back(*link);
          }
          return tracks;
        };
      } else {
        throw std::logic_error("Unknown TrackLinkType");
      }
    }

    Tracks TracksFromJet::operator()(const xAOD::Jet& jet,
                                     const SG::AuxElement& btagging) const {

      std::vector<std::pair<double, const xAOD::TrackParticle*>> tracks;
      for (const xAOD::TrackParticle *tp : m_associator(btagging)) {
        if (m_trackFilter(tp)) {
          tracks.push_back({m_trackSortVar(tp, jet), tp});
        };
      }
      std::sort(tracks.begin(), tracks.end(), std::greater<>());
      std::vector<const xAOD::TrackParticle*> only_tracks;
      only_tracks.reserve(tracks.size());
      for (const auto& trk: tracks) {
        only_tracks.push_back(trk.second);
      }
      return only_tracks;
    }

    // constructor
    internal::TrackSequenceBuilder::TrackSequenceBuilder(
      SortOrder sort,
      TrackSelection selection,
      const FTagOptions& options):
      tracksFromJet(sort, selection, options),
      flipFilter(internal::get::flipFilter(options).first){
    }

    // ______________________________________________________________________
    // Internal utility functions
    //

    // The 'get' namespace is for factories that build std::function
    // objects
    namespace get {
      // factory for functions that get variables out of the b-tagging
      // object
      VarFromBTag varFromBTag(const std::string& name, EDMType type,
                            const std::string& default_flag) {
        if(default_flag.size() == 0 || name==default_flag)
        {
          switch (type) {
            case EDMType::INT: return BVarGetterNoDefault<int>(name);
            case EDMType::FLOAT: return BVarGetterNoDefault<float>(name);
            case EDMType::DOUBLE: return BVarGetterNoDefault<double>(name);
            case EDMType::CHAR: return BVarGetterNoDefault<char>(name);
            case EDMType::UCHAR: return BVarGetterNoDefault<unsigned char>(name);
            default: {
              throw std::logic_error("Unknown EDM type");
            }
          }
        }
        else{
          switch (type) {
            case EDMType::INT: return BVarGetter<int>(name, default_flag);
            case EDMType::FLOAT: return BVarGetter<float>(name, default_flag);
            case EDMType::DOUBLE: return BVarGetter<double>(name, default_flag);
            case EDMType::CHAR: return BVarGetter<char>(name, default_flag);
            case EDMType::UCHAR: return BVarGetter<unsigned char>(name, default_flag);
            default: {
              throw std::logic_error("Unknown EDM type");
            }
          }
        }
      }

      // factory for functions which return the sort variable we
      // use to order tracks
      TrackSortVar trackSortVar(SortOrder config, const FTagOptions& options)
      {
        typedef xAOD::TrackParticle Tp;
        typedef xAOD::Jet Jet;
        BTagTrackIpAccessor aug(options.track_prefix);
        switch(config) {
        case SortOrder::ABS_D0_SIGNIFICANCE_DESCENDING:
          return [aug](const Tp* tp, const Jet&) {
                   return std::abs(aug.d0(*tp) / aug.d0Uncertainty(*tp));
                 };
        case SortOrder::D0_SIGNIFICANCE_DESCENDING:
          return [aug](const Tp* tp, const Jet& j) {
                   return aug.getSignedIp(*tp, j).ip3d_signed_d0_significance;
                 };
        case SortOrder::PT_DESCENDING:
          return [](const Tp* tp, const Jet&) {return tp->pt();};
        case SortOrder::ABS_D0_DESCENDING:
          return [aug](const Tp* tp, const Jet&) {
            return std::abs(aug.d0(*tp));
          };

        default: {
          throw std::logic_error("Unknown sort function");
        }
        }
      } // end of track sort getter

      // factory for functions that return true for tracks we want to
      // use, false for those we don't want
      std::pair<TrackFilter,std::set<std::string>> trackFilter(
        TrackSelection config, const FTagOptions& options) {

        typedef xAOD::TrackParticle Tp;
        typedef SG::AuxElement AE;
        BTagTrackIpAccessor aug(options.track_prefix);
        auto data_deps = aug.getTrackIpDataDependencyNames();

        // make sure we record accessors as data dependencies
        std::set<std::string> track_deps;
        auto addAccessor = [&track_deps](const std::string& n) {
                             AE::ConstAccessor<unsigned char> a(n);
                             track_deps.insert(n);
                             return a;
                           };
        auto pix_hits = addAccessor("numberOfPixelHits");
        auto pix_holes = addAccessor("numberOfPixelHoles");
        auto pix_shared = addAccessor("numberOfPixelSharedHits");
        auto pix_dead = addAccessor("numberOfPixelDeadSensors");
        auto sct_hits = addAccessor("numberOfSCTHits");
        auto sct_holes = addAccessor("numberOfSCTHoles");
        auto sct_shared = addAccessor("numberOfSCTSharedHits");
        auto sct_dead = addAccessor("numberOfSCTDeadSensors");

        // data deps is all possible dependencies. We insert here to
        // avoid removing them from track_deps (as merge would).
        data_deps.insert(track_deps.begin(), track_deps.end());

        switch (config) {
        case TrackSelection::ALL: return {[](const Tp*) {return true;}, {} };
          // the following numbers come from Nicole, Dec 2018:
          // pt > 1 GeV
          // abs(d0) < 1 mm
          // abs(z0 sin(theta)) < 1.5 mm
          // >= 7 si hits
          // <= 2 si holes
          // <= 1 pix holes
        case TrackSelection::IP3D_2018:
          return {
            [=](const Tp* tp) {
              // from the track selector tool
              if (std::abs(tp->eta()) > 2.5) return false;
              double n_module_shared = (pix_shared(*tp) + sct_shared(*tp) / 2);
              if (n_module_shared > 1) return false;
              if (tp->pt() <= 1e3) return false;
              if (std::abs(aug.d0(*tp)) >= 1.0) return false;
              if (std::abs(aug.z0SinTheta(*tp)) >= 1.5) return false;
              if (pix_hits(*tp) + pix_dead(*tp) + sct_hits(*tp) + sct_dead(*tp) < 7) return false;
              if ((pix_holes(*tp) + sct_holes(*tp)) > 2) return false;
              if (pix_holes(*tp) > 1) return false;
              return true;
            }, data_deps
          };
          // Tight track selection for DIPS upgrade config
          // abs(eta) < 4 
          // pt > 1 GeV
          // abs(d0) < 1 mm
          // abs(z0 sin(theta)) < 1.5 mm
          // No cuts for si hits, si holes and pix holes - only reconstruction selection is applied
        case TrackSelection::DIPS_TIGHT_UPGRADE:
          return {
            [=](const Tp* tp) {
              // from the track selector tool
              if (std::abs(tp->eta()) > 4.0) return false;
              if (tp->pt() <= 1e3) return false;
              if (std::abs(aug.d0(*tp)) >= 1.0) return false;
              if (std::abs(aug.z0SinTheta(*tp)) >= 1.5) return false;
              return true;
            }, data_deps
          };
          // Loose track selection for DIPS upgrade config
          // abs(eta) < 4
          // pt > 0.5 GeV
          // abs(d0) < 3.5 mm
          // abs(z0 sin(theta)) < 5.0 mm
          // No cuts for si hits, si holes and pix holes - only reconstruction selection is applied
        case TrackSelection::DIPS_LOOSE_UPGRADE:
          return {
            [=](const Tp* tp) {
              // from the track selector tool
              if (std::abs(tp->eta()) > 4.0) return false;
              if (tp->pt() <= 0.5e3) return false;
              if (std::abs(aug.d0(*tp)) >= 3.5) return false;
              if (std::abs(aug.z0SinTheta(*tp)) >= 5.0) return false;
              return true;
            }, data_deps
          };
          // Loose track selection for DIPS
          // pt > 0.5 GeV
          // abs(d0) < 3.5 mm
          // abs(z0 sin(theta)) < 5.0 mm
          // >= 7 si hits
          // <= 2 si holes
          // <= 1 pix holes
        case TrackSelection::DIPS_LOOSE_202102:
          return {
            [=](const Tp* tp) {
              // from the track selector tool
              if (std::abs(tp->eta()) > 2.5) return false;
              double n_module_shared = (pix_shared(*tp) + sct_shared(*tp) / 2);
              if (n_module_shared > 1) return false;
              if (tp->pt() <= 0.5e3) return false;
              if (std::abs(aug.d0(*tp)) >= 3.5) return false;
              if (std::abs(aug.z0SinTheta(*tp)) >= 5.0) return false;
              if (pix_hits(*tp) + pix_dead(*tp) + sct_hits(*tp) + sct_dead(*tp) < 7) return false;
              if ((pix_holes(*tp) + sct_holes(*tp)) > 2) return false;
              if (pix_holes(*tp) > 1) return false;
              return true;
            }, data_deps
          };
        case TrackSelection::LOOSE_202102_NOIP:
          return {
            [=](const Tp* tp) {
              if (std::abs(tp->eta()) > 2.5) return false;
              double n_module_shared = (pix_shared(*tp) + sct_shared(*tp) / 2);
              if (n_module_shared > 1) return false;
              if (tp->pt() <= 0.5e3) return false;
              if (pix_hits(*tp) + pix_dead(*tp) + sct_hits(*tp) + sct_dead(*tp) < 7) return false;
              if ((pix_holes(*tp) + sct_holes(*tp)) > 2) return false;
              if (pix_holes(*tp) > 1) return false;
              return true;
            }, track_deps
          };
        // R22_DEFAULT is similar to DIPS_LOOSE_202102, but modifies the min Si hit cut to 8,
        // which is the default tracking CP recommendation for r22, see
        // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/TrackingCPRecsRun2R22#Selection_Criteria
        case TrackSelection::R22_DEFAULT:
          return {
            [=](const Tp* tp) {
              // from the track selector tool
              if (std::abs(tp->eta()) > 2.5) return false;
              double n_module_shared = (pix_shared(*tp) + sct_shared(*tp) / 2);
              if (n_module_shared > 1) return false;
              if (tp->pt() <= 0.5e3) return false;
              if (std::abs(aug.d0(*tp)) >= 3.5) return false;
              if (std::abs(aug.z0SinTheta(*tp)) >= 5.0) return false;
              if (pix_hits(*tp) + pix_dead(*tp) + sct_hits(*tp) + sct_dead(*tp) < 8) return false;
              if ((pix_holes(*tp) + sct_holes(*tp)) > 2) return false;
              if (pix_holes(*tp) > 1) return false;
              return true;
            }, data_deps
          };
        // R22_LOOSE is similar to R22_DEFAULT, but removes the shared module cut 
        // and loosens the d0 cut
        case TrackSelection::R22_LOOSE:
          return {
            [=](const Tp* tp) {
              // from the track selector tool
              if (std::abs(tp->eta()) > 2.5) return false;
              if (tp->pt() <= 0.5e3) return false;
              if (std::abs(aug.d0(*tp)) >= 5.0) return false;
              if (std::abs(aug.z0SinTheta(*tp)) >= 5.0) return false;
              if (pix_hits(*tp) + pix_dead(*tp) + sct_hits(*tp) + sct_dead(*tp) < 8) return false;
              if ((pix_holes(*tp) + sct_holes(*tp)) > 2) return false;
              if (pix_holes(*tp) > 1) return false;
              return true;
            }, data_deps
          };
        default:
          throw std::logic_error("unknown track selection function");
        }
      }

      // factory for functions that build std::vector objects from
      // track sequences
      std::pair<SeqFromTracks,std::set<std::string>> seqFromTracks(
        const FTagTrackInputConfig& cfg, const FTagOptions& options)
      {
        const std::string prefix = options.track_prefix;
        switch (cfg.type) {
          case EDMType::INT: return {
              SequenceGetter<int>(cfg.name), {cfg.name}
            };
          case EDMType::FLOAT: return {
              SequenceGetter<float>(cfg.name), {cfg.name}
            };
          case EDMType::CHAR: return {
              SequenceGetter<char>(cfg.name), {cfg.name}
            };
          case EDMType::UCHAR: return {
              SequenceGetter<unsigned char>(cfg.name), {cfg.name}
            };
          case EDMType::CUSTOM_GETTER: {
            return customNamedSeqGetterWithDeps(
              cfg.name, options.track_prefix);

          }
          default: {
            throw std::logic_error("Unknown EDM type for tracks");
          }
        }
      }

      // here we define filters for the "flip" taggers
      //
      // start by defining the raw functions, there's a factory
      // function below to convert the configuration enums to a
      // std::function
      Tracks negativeIpOnly(BTagTrackIpAccessor& aug,
                            const Tracks& tracks,
                            const xAOD::Jet& j) {
        Tracks filtered;
        // we want to reverse the order of the tracks as part of the
        // flipping
        for (auto ti = tracks.crbegin(); ti != tracks.crend(); ti++) {
          const xAOD::TrackParticle* tp = *ti;
          double sip = aug.getSignedIp(*tp, j).ip3d_signed_d0_significance;
          if (sip < 0) filtered.push_back(tp);
        }
        return filtered;
      }

      // factory function
      std::pair<TrackSequenceFilter,std::set<std::string>> flipFilter(
        const FTagOptions& options)
      {
        namespace ph = std::placeholders;  // for _1, _2, _3
        BTagTrackIpAccessor aug(options.track_prefix);
        switch(options.flip) {
        case FlipTagConfig::NEGATIVE_IP_ONLY:
          // flips order and removes tracks with negative IP
          return {
            std::bind(&negativeIpOnly, aug, ph::_1, ph::_2),
            aug.getTrackIpDataDependencyNames()
          };
        case FlipTagConfig::FLIP_SIGN:
          // Just flips the order
          return {
            [](const Tracks& tr, const xAOD::Jet& ) {
              return Tracks(tr.crbegin(), tr.crend());},
            {}
          };
        case FlipTagConfig::SIMPLE_FLIP:
          // Just flips the order
          return {
            [](const Tracks& tr, const xAOD::Jet& ) {
              return Tracks(tr.crbegin(), tr.crend());},
            {}
          };
        case FlipTagConfig::STANDARD:
          return {[](const Tracks& tr, const xAOD::Jet& ) { return tr; }, {}};
        default: {
          throw std::logic_error("Unknown flip config");
        }
        }
      }
    } // end of get namespace
  } // end of internal namespace



  // ______________________________________________________________________
  // High level configuration functions
  //
  // Most of the NN code should be a relatively thin wrapper on these
  // functions.

  namespace dataprep {


    // Translate string config to config objects
    //
    // This parses the saved NN configuration structure and translates
    // informaton encoded as strings into structures and enums to be
    // consumed by the code that actually constructs the NN.
    //
    std::tuple<
      std::vector<FTagInputConfig>,
      std::vector<FTagTrackSequenceConfig>,
      FTagOptions>
    createGetterConfig( lwt::GraphConfig& config,
      FlipTagConfig flip_config,
      std::map<std::string, std::string> remap_scalar,
      TrackLinkType track_link_type
    ){

      // get the regex to rewrite the inputs if we're using flip taggers
      StringRegexes flip_converters = getFlipConverters(flip_config);

      // some sequences also need to be sign-flipped. We apply this by
      // changing the input scaling and normalizations
      std::regex flip_sequences;
      if (flip_config == FlipTagConfig::FLIP_SIGN || flip_config == FlipTagConfig::NEGATIVE_IP_ONLY){
        flip_sequences=std::regex(".*signed_[dz]0.*");
      }
      if (flip_config == FlipTagConfig::SIMPLE_FLIP){
        flip_sequences=std::regex("(.*signed_[dz]0.*)|d0|z0SinTheta");
      }

      if (flip_config != FlipTagConfig::STANDARD) {
        rewriteFlipConfig(config, flip_converters);
      }

      // build the standard inputs

      // type and default value-finding regexes are hardcoded for now
      TypeRegexes type_regexes = {
        {".*_isDefaults"_r, EDMType::CHAR},
        // TODO: in the future we should migrate RNN and IPxD
        // variables to floats. This is outside the scope of the
        // current flavor tagging developments and AFT-438.
        {"IP[23]D(Neg)?_[pbc](b|c|u|tau)"_r, EDMType::FLOAT},
        {"SV1(Flip)?_[pbc](b|c|u|tau)"_r, EDMType::FLOAT},
        {"(rnnip|iprnn|dips[^_]*)(flip)?_p(b|c|u|tau)"_r, EDMType::FLOAT},
        {"(JetFitter|SV1|JetFitterSecondaryVertex)(Flip)?_[Nn].*"_r, EDMType::INT},
        {"(JetFitter|SV1|JetFitterSecondaryVertex).*"_r, EDMType::FLOAT},
        {"(log_)?pt|abs_eta|eta|phi|energy|mass"_r, EDMType::CUSTOM_GETTER},
        {"softMuon_p[bcu]"_r, EDMType::FLOAT},
        {"softMuon_.*"_r, EDMType::FLOAT},
      };

      StringRegexes default_flag_regexes{
        {"IP2D_.*"_r, "IP2D_isDefaults"},
        {"IP2DNeg_.*"_r, "IP2DNeg_isDefaults"},
        {"IP3D_.*"_r, "IP3D_isDefaults"},
        {"IP3DNeg_.*"_r, "IP3DNeg_isDefaults"},
        {"SV1_.*"_r, "SV1_isDefaults"},
        {"SV1Flip_.*"_r, "SV1Flip_isDefaults"},
        {"JetFitter_.*"_r, "JetFitter_isDefaults"},
        {"JetFitterFlip_.*"_r, "JetFitterFlip_isDefaults"},
        {"JetFitterSecondaryVertex_.*"_r, "JetFitterSecondaryVertex_isDefaults"},
        {"JetFitterSecondaryVertexFlip_.*"_r, "JetFitterSecondaryVertexFlip_isDefaults"},
        {"rnnip_.*"_r, "rnnip_isDefaults"},
        {"(dips[^_]*)_.*"_r, "$1_isDefaults"},
        {"rnnipflip_.*"_r, "rnnipflip_isDefaults"},
        {"iprnn_.*"_r, ""},
        {"smt_.*"_r, "softMuon_isDefaults"},
        {"softMuon_.*"_r, "softMuon_isDefaults"},
        {"((log_)?pt|abs_eta|eta|phi|energy|mass)"_r, ""}}; // no default for custom cases

      std::vector<FTagInputConfig> input_config;
      for (auto& node: config.inputs){
        // allow the user to remape some of the inputs
        remap_inputs(node.variables, remap_scalar,
               node.defaults);

        std::vector<std::string> input_names;
        for (const auto& var: node.variables) {
          input_names.push_back(var.name);
        }

        // check to make sure the next line doesn't overwrite something
        // TODO: figure out how to support multiple scalar input nodes
        if (!input_config.empty()) {
          throw std::logic_error(
            "We don't currently support multiple scalar input nodes");
        }
        input_config = get_input_config(
        input_names, type_regexes, default_flag_regexes);
      }

      // build the track inputs

      std::vector<std::pair<std::string, std::vector<std::string> > > trk_names;
      for (auto& node: config.input_sequences) {
        remap_inputs(node.variables, remap_scalar,
		     node.defaults);

        std::vector<std::string> names;
        for (const auto& var: node.variables) {
          names.push_back(var.name);
        }
        trk_names.emplace_back(node.name, names);
      }

      TypeRegexes trk_type_regexes {
        // Some innermost / next-to-innermost hit variables had a different
        // definition in 21p9, recomputed here with customGetter to reuse
        // existing training
        // EDMType picked correspond to the first matching regex
        {"numberOf.*21p9"_r, EDMType::CUSTOM_GETTER},
        {"numberOf.*"_r, EDMType::UCHAR},
        {"btagIp_(d0|z0SinTheta)Uncertainty"_r, EDMType::FLOAT},
        {"(numberDoF|chiSquared|qOverP|theta)"_r, EDMType::FLOAT},
        {"(^.*[_])?(d|z)0.*"_r, EDMType::CUSTOM_GETTER},
        {"(log_)?(ptfrac|dr|pt).*"_r, EDMType::CUSTOM_GETTER},
        {"(deta|dphi)"_r, EDMType::CUSTOM_GETTER},
        {"phi|theta|qOverP"_r, EDMType::FLOAT},
        {"(phi|theta|qOverP)Uncertainty"_r, EDMType::CUSTOM_GETTER},
        {"leptonID"_r, EDMType::CHAR}
      };
      // We have a number of special naming conventions to sort and
      // filter tracks. The track nodes should be named according to
      //
      // tracks_<selection>_<sort-order>
      //
      SortRegexes trk_sort_regexes {
        {".*absSd0sort"_r, SortOrder::ABS_D0_SIGNIFICANCE_DESCENDING},
        {".*sd0sort"_r, SortOrder::D0_SIGNIFICANCE_DESCENDING},
        {".*ptsort"_r, SortOrder::PT_DESCENDING},
        {".*absD0DescendingSort"_r, SortOrder::ABS_D0_DESCENDING},
      };
      TrkSelRegexes trk_select_regexes {
        {".*_ip3d_.*"_r, TrackSelection::IP3D_2018},
        {".*_dipsTightUpgrade_.*"_r, TrackSelection::DIPS_TIGHT_UPGRADE},
        {".*_dipsLooseUpgrade_.*"_r, TrackSelection::DIPS_LOOSE_UPGRADE},
        {".*_all_.*"_r, TrackSelection::ALL},
        {".*_dipsLoose202102_.*"_r, TrackSelection::DIPS_LOOSE_202102},
        {".*_loose202102NoIpCuts_.*"_r, TrackSelection::LOOSE_202102_NOIP},
        {".*_r22default_.*"_r, TrackSelection::R22_DEFAULT},
        {".*_r22loose_.*"_r, TrackSelection::R22_LOOSE},
      };

      auto trk_config = get_track_input_config(
        trk_names, trk_type_regexes, trk_sort_regexes, trk_select_regexes,flip_sequences,flip_config);

      // some additional options
      FTagOptions options;
      if (auto h = remap_scalar.extract(options.track_prefix)) {
        options.track_prefix = h.mapped();
      }
      if (auto h = remap_scalar.extract(options.track_link_name)) {
        options.track_link_name = h.mapped();
      }
      if (auto h = remap_scalar.extract(options.invalid_ip_key)) {
        options.invalid_ip_key = h.mapped();
      }
      options.flip = flip_config;
      options.remap_scalar = remap_scalar;
      options.track_link_type = track_link_type;
      return std::make_tuple(input_config, trk_config, options);
    } // end of getGetterConfig


    // Translate configuration to getter functions
    //
    // This focuses on the scalar inputs, i.e. the inputs for DL1d,
    // the code for the track inputs is below.
    std::tuple<
      std::vector<internal::VarFromBTag>,
      std::vector<internal::VarFromJet>,
      FTagDataDependencyNames>
    createBvarGetters(
      const std::vector<FTagInputConfig>& inputs)
    {
      FTagDataDependencyNames deps;
      std::vector<internal::VarFromBTag> varsFromBTag;
      std::vector<internal::VarFromJet> varsFromJet;

      for (const auto& input: inputs) {
        if (input.type != EDMType::CUSTOM_GETTER) {
          auto filler = internal::get::varFromBTag(input.name, input.type,
                                         input.default_flag);
          deps.bTagInputs.insert(input.name);
          varsFromBTag.push_back(filler);
        } else {
          varsFromJet.push_back(internal::customGetterAndName(input.name));
        }
        if (input.default_flag.size() > 0) {
          deps.bTagInputs.insert(input.default_flag);
        }
      }

      return std::make_tuple(varsFromBTag, varsFromJet, deps);
    }


    // Translate configuration to track getters
    //
    // This focuses on the track inputs, i.e. the inputs for dips, the
    // code for the track inputs is above.
    std::tuple<
      std::vector<internal::TrackSequenceBuilder>,
      FTagDataDependencyNames,
      std::set<std::string>>
    createTrackGetters(
      const std::vector<FTagTrackSequenceConfig>& track_sequences,
      const FTagOptions& options)
    {
      FTagDataDependencyNames deps;
      std::vector<internal::TrackSequenceBuilder> trackSequenceBuilders;
      std::map<std::string, std::string> remap = options.remap_scalar;
      std::set<std::string> used_remap;

      for (const FTagTrackSequenceConfig& cfg: track_sequences) {
        internal::TrackSequenceBuilder track_getter(
          cfg.order, cfg.selection, options);
        // add the tracking data dependencies
        auto track_data_deps = internal::get::trackFilter(
          cfg.selection, options).second;
        track_data_deps.merge(internal::get::flipFilter(options).second);

        track_getter.name = cfg.name;
        for (const FTagTrackInputConfig& input_cfg: cfg.inputs) {
          auto [seqGetter, deps] = internal::get::seqFromTracks(
            input_cfg, options);

          if(input_cfg.flip_sign){
            auto seqGetter_flip=[g=seqGetter](const xAOD::Jet&jet, const internal::Tracks& trks){
              auto [n,v] = g(jet,trks);
              std::for_each(v.begin(), v.end(), [](double &n){ n=-1.0*n; });
              return std::make_pair(n,v);
            };
            track_getter.sequencesFromTracks.push_back(seqGetter_flip);
          }
          else{
            track_getter.sequencesFromTracks.push_back(seqGetter);
          }
          track_data_deps.merge(deps);

	  if (auto h = remap.extract(input_cfg.name)){
            used_remap.insert(h.key());
          }

        }
        trackSequenceBuilders.push_back(track_getter);
        deps.trackInputs.merge(track_data_deps);
        deps.bTagInputs.insert(options.track_link_name);
      }

      return std::make_tuple(trackSequenceBuilders, deps, used_remap);
    }


    // Translate configuration to setter functions
    //
    // This returns the "decorators" that we use to save outputs to
    // the EDM.
    std::tuple<
      std::map<std::string, internal::OutNodeFloat>,
      FTagDataDependencyNames,
      std::set<std::string>>
    createDecorators(
      const lwt::GraphConfig& config,
      const FTagOptions& options)
    {
      FTagDataDependencyNames deps;
      std::map<std::string, internal::OutNodeFloat> decorators;
      std::map<std::string, std::string> remap = options.remap_scalar;
      std::set<std::string> used_remap;

      for (const auto& out_node: config.outputs) {
        std::string node_name = out_node.first;

        internal::OutNodeFloat node;
        for (const std::string& element: out_node.second.labels) {
          std::string name = node_name + "_" + element;

          // let user rename the output
          if (auto h = remap.extract(name)){
            name = h.mapped();
            used_remap.insert(h.key());
          }
          deps.bTagOutputs.insert(name);

          SG::AuxElement::Decorator<float> f(name);
          node.emplace_back(element, f);
        }
        decorators[node_name] = node;
      }

      return std::make_tuple(decorators, deps, used_remap);
    }

    std::tuple<
      internal::OutNodeFloat, internal::OutNodeVecChar,
      internal::OutNodeVecFloat, internal::OutNodeTrackLinks,
      internal::OutNodeChar, internal::OutNodeFloat,
      FTagDataDependencyNames, std::set<std::string>>
    createGNDecorators(
      const GNNConfig::Config& config,
      const FTagOptions& options)
    {
      FTagDataDependencyNames deps;
      internal::OutNodeFloat decorators_f;
      internal::OutNodeVecChar decorators_vc;
      internal::OutNodeVecFloat decorators_vf;
      internal::OutNodeTrackLinks decorators_tl;
      internal::OutNodeChar decorators_track_c;
      internal::OutNodeFloat decorators_track_f;

      std::map<std::string, std::string> remap = options.remap_scalar;
      std::set<std::string> used_remap;

      // get the regex to rewrite the outputs if we're using flip taggers
      StringRegexes flip_converters = getFlipConverters(options.flip);
      std::string context = "building negative tag b-btagger";

      // for each model output
      for (const auto& out_node: config.outputs) {

        std::string name = out_node.label;
        
        // modify the output name if we're using flip taggers
        if (options.flip != FlipTagConfig::STANDARD) {
          name = sub_first(flip_converters, name, context);  
        }

        // let user rename the output
        if (auto h = remap.extract(name)){
          name = h.mapped();
          used_remap.insert(h.key());
        }
        deps.bTagOutputs.insert(name);

        // create a decorator for this output depending on the rank and type
        switch (out_node.type) {
          case GNNConfig::OutputNodeType::FLOAT: {
            SG::AuxElement::Decorator<float> f(name);
            decorators_f.emplace_back(out_node.label, f);
            break;
          }
          case GNNConfig::OutputNodeType::VECCHAR: {
            if (out_node.target == GNNConfig::OutputNodeTarget::JET) {
              SG::AuxElement::Decorator<std::vector<char>> vc(name);
              decorators_vc.emplace_back(out_node.label, vc);
            } 
            else if (out_node.target == GNNConfig::OutputNodeTarget::TRACK) {
              SG::AuxElement::Decorator<char> c(name);
              decorators_track_c.emplace_back(out_node.label, c);
            }
            else {
              throw std::logic_error("unknown outputnode target");
            }
            break;
          }
          case GNNConfig::OutputNodeType::VECFLOAT: {
            if (out_node.target == GNNConfig::OutputNodeTarget::JET) {
              SG::AuxElement::Decorator<std::vector<float>> vf(name);
              decorators_vf.emplace_back(out_node.label, vf);
            } 
            else if (out_node.target == GNNConfig::OutputNodeTarget::TRACK) {
              SG::AuxElement::Decorator<float> f(name);
              decorators_track_f.emplace_back(out_node.label, f);
            }
            else {
              throw std::logic_error("unknown outputnode target");
            }
            break;
          }
          default:
            throw std::logic_error("unknown outputnode type");
        }
      }

      // create a decorator for the track links
      if (decorators_vc.size() > 0 || decorators_vf.size() > 0){
        std::string name = "TrackLinks";
        if (auto h = remap.extract(name)){
          name = h.mapped();
          used_remap.insert(h.key());
        }
        deps.bTagOutputs.insert(name);
        SG::AuxElement::Decorator<internal::TrackLinks> tl(name);
        decorators_tl.emplace_back("TrackLinks", tl);
      }

      return std::make_tuple(decorators_f, decorators_vc, decorators_vf, decorators_tl, decorators_track_c, decorators_track_f, deps, used_remap);
    }

    // return a function to check IP validity
    std::tuple<
      std::function<char(const internal::Tracks&)>,
      std::vector<SG::AuxElement::Decorator<char>>,
      FTagDataDependencyNames,
      std::set<std::string>>
    createIpChecker(
      const lwt::GraphConfig& gc, const FTagOptions& opts) {
      using namespace internal;
      FTagDataDependencyNames deps;
      std::map<std::string, std::string> remap = opts.remap_scalar;
      std::set<std::string> used_remap;
      // dummy if there's no invalid check key
      std::function checker = [](const Tracks&) -> char {return 0;};
      // if we do have a key, return 1 for invalid
      if (!opts.invalid_ip_key.empty()) {
        std::string ip_key = opts.track_prefix + opts.invalid_ip_key;
        SG::AuxElement::ConstAccessor<char> invalid_check(ip_key);
        checker = [invalid_check](const Tracks& trs){
          for (const xAOD::TrackParticle* trk: trs) {
            if (invalid_check(*trk)) return 1;
          }
          return 0;
        };
        deps.trackInputs.insert(ip_key);
      }
      std::vector<SG::AuxElement::Decorator<char>> default_decs;
      for (const auto& output: gc.outputs) {
        std::string basename = output.first;
        std::string dec_name = basename + "_isDefaults";
        if (auto h = remap.extract(dec_name)) {
          dec_name = h.mapped();
          used_remap.insert(h.key());
        }
        default_decs.emplace_back(dec_name);
        deps.bTagOutputs.insert(dec_name);
      }
      return {checker, default_decs, deps, used_remap};
    }

    void checkForUnusedRemaps(
      const std::map<std::string, std::string>& requested,
      const std::set<std::string>& used)
    {
      // we want to make sure every remapping was used
      std::set<std::string> unused;
      for (auto [k, v]: requested) {
        if (!used.count(k)) unused.insert(k);
      }
      if (unused.size() > 0) {
        std::string outputs;
        for (const auto& item: unused) {
          outputs.append(item);
          if (item != *unused.rbegin()) outputs.append(", ");
        }
        throw std::logic_error("found unused output remapping(s): " + outputs);
      }
    }
  } // end of datapre namespace

} // end of FlavorTagDiscriminants namespace

