/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ELECTRON_TRUTH_DECORATOR_ALG_HH
#define ELECTRON_TRUTH_DECORATOR_ALG_HH

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthContainers/AuxElement.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"

#include "AthLinks/ElementLink.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginTool.h"
#include "xAODTruth/TruthEventContainer.h"

#include <set>

namespace FlavorTagDiscriminants {

  class SoftElectronTruthDecoratorAlg: public AthReentrantAlgorithm {
  public:
    SoftElectronTruthDecoratorAlg(const std::string& name,
                          ISvcLocator* pSvcLocator );

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ) const override;

  private:
    // Input Containers
    SG::ReadHandleKey< xAOD::ElectronContainer > m_ElectronContainerKey {
      this, "electronContainer", "Electrons",
        "Key for the input electron collection"};
    SG::ReadHandleKey< xAOD::TruthEventContainer > m_TruthEventsKey {
      this, "truthEvents", "TruthEvents",
        "Key for the input truth event collection"};

    // Decorators for electrons
    SG::WriteDecorHandleKey< xAOD::ElectronContainer > m_dec_origin_label {
      this, "ftag_truthOriginLabel", "ftag_truthOriginLabel", 
        "Exclusive origin label of the electron"};
    SG::WriteDecorHandleKey< xAOD::ElectronContainer > m_dec_type_label {
      this, "ftag_truthTypeLabel", "ftag_truthTypeLabel", 
        "Exclusive truth type label of the electron"};
    SG::WriteDecorHandleKey< xAOD::ElectronContainer > m_dec_vertex_index {
      this, "ftag_truthVertexIndex", "ftag_truthVertexIndex", 
        "Truth vertex index of the electron"};

    // truth origin tool
    ToolHandle<InDet::InDetTrackTruthOriginTool> m_trackTruthOriginTool {
      this, "trackTruthOriginTool", "InDet::InDetTrackTruthOriginTool", 
        "track truth origin tool"};

    Gaudi::Property<float> m_truthVertexMergeDistance {
      this, "truthVertexMergeDistance", 0.1, 
        "Merge any truth vertices within this distance [mm]"};

    // Electron types from 
    // PhysicsAnalysis/MCTruthClassifier/MCTruthClassifier/MCTruthClassifierDefs.h#L28
    const std::set<int> m_valid_types{1, 2, 3, 4};    

    SG::AuxElement::ConstAccessor<unsigned int> m_classifierParticleType{"classifierParticleType"};
    SG::AuxElement::ConstAccessor<ElementLink<xAOD::TruthParticleContainer>> m_truthParticleLink{"truthParticleLink"};
    
  };
}

#endif
