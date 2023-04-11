/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GNN_H
#define GNN_H

// Tool includes
#include "FlavorTagDiscriminants/FlipTagEnums.h"
#include "FlavorTagDiscriminants/AssociationEnums.h"
#include "FlavorTagDiscriminants/FTagDataDependencyNames.h"

#include "FlavorTagDiscriminants/DataPrepUtilities.h"

// EDM includes
#include "xAODBTagging/BTaggingFwd.h"
#include "xAODJet/JetContainer.h"

#include <memory>
#include <string>
#include <map>

namespace FlavorTagDiscriminants {

  class OnnxUtil;
  //
  // Tool to to flavor tag jet/btagging object
  // using GNN based taggers
  class GNN
  {
  public:
    GNN(const std::string& nnFile,
        const FlipTagConfig& flip_config = FlipTagConfig::STANDARD,
        const std::map<std::string, std::string>& variableRemapping = {},
        const TrackLinkType trackLinkType = TrackLinkType::TRACK_PARTICLE,
        float defaultOutputValue = NAN);
    GNN(GNN&&);
    GNN(const GNN&);
    virtual ~GNN();

    virtual void decorate(const xAOD::BTagging& btag) const;
    virtual void decorate(const xAOD::Jet& jet) const;
    virtual void decorateWithDefaults(const xAOD::Jet& jet) const;
    void decorate(const xAOD::Jet& jet, const SG::AuxElement& decorated) const;

    virtual std::set<std::string> getDecoratorKeys() const;
    virtual std::set<std::string> getAuxInputKeys() const;
    virtual std::set<std::string> getConstituentAuxInputKeys() const;

    std::shared_ptr<const OnnxUtil> m_onnxUtil;

  private:

    GNNConfig::Config m_config_gnn;

    SG::AuxElement::ConstAccessor<ElementLink<xAOD::JetContainer>> m_jetLink;
    std::string m_input_node_name;
    std::vector<internal::VarFromBTag> m_varsFromBTag;
    std::vector<internal::VarFromJet> m_varsFromJet;
    std::vector<internal::TrackSequenceBuilder> m_trackSequenceBuilders;
    internal::OutNode m_decorators_float;
    internal::OutNodeVecChar m_decorators_vecchar;
    internal::OutNodeVecFloat m_decorators_vecfloat;
    internal::OutNodeTrackLinks m_decorators_tracklinks;
    float m_defaultValue;

    FTagDataDependencyNames m_dataDependencyNames;
  };
}
#endif
