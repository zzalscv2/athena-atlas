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

    // Decorators for electrons
    SG::WriteDecorHandleKey< xAOD::ElectronContainer > m_dec_origin_label {
      this, "ftagTruthOriginLabel", "ftagTruthOriginLabel", 
        "Exclusive origin label of the electron"};
    SG::WriteDecorHandleKey< xAOD::ElectronContainer > m_dec_type_label {
      this, "ftagTruthTypeLabel", "ftagTruthTypeLabel", 
        "Exclusive truth type label of the electron"};
    SG::WriteDecorHandleKey< xAOD::ElectronContainer > m_dec_vertex_index {
      this, "ftagTruthVertexIndex", "ftagTruthVertexIndex", 
        "Truth vertex index of the electron"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_barcode {
      this, "ftagTruthBarcode", "ftagTruthBarcode", 
        "Barcode of linked truth particle"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_parent_barcode {
      this, "ftagTruthParentBarcode", "ftagTruthParentBarcode", 
        "Barcode of parent of linked truth particle"};

    // truth origin tool
    ToolHandle<InDet::InDetTrackTruthOriginTool> m_trackTruthOriginTool {
      this, "trackTruthOriginTool", "InDet::InDetTrackTruthOriginTool", 
        "track truth origin tool"};

    // Electron types are defined in
    // PhysicsAnalysis/MCTruthClassifier/MCTruthClassifier/MCTruthClassifierDefs.h#L28
    const std::set<int> m_valid_types{1, 2, 3, 4};

    // Accessors
    template <typename T> using Acc = SG::AuxElement::ConstAccessor<T>;
    Acc<unsigned int> m_classifierParticleType{"classifierParticleType"};
    Acc<ElementLink<xAOD::TruthParticleContainer>> m_truthParticleLink{"truthParticleLink"};
    Acc<int> m_acc_truthOriginLabel{"ftagTruthOriginLabel"};
    Acc<int> m_acc_truthTypeLabel{"ftagTruthTypeLabel"};
    Acc<int> m_acc_truthVertexIndex{"ftagTruthVertexIndex"};
    Acc<int> m_acc_truthParentBarcode{"ftagTruthParentBarcode"};
  };
}

#endif
