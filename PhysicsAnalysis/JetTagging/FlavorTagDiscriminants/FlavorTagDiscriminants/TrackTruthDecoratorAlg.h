/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACK_TRUTH_DECORATOR_ALG_HH
#define TRACK_TRUTH_DECORATOR_ALG_HH

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthContainers/AuxElement.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"

#include "xAODTruth/TruthParticleContainerFwd.h"
#include "xAODTruth/TruthEventContainer.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginTool.h"


namespace FlavorTagDiscriminants {

  class TrackTruthDecoratorAlg: public AthReentrantAlgorithm {
  public:
    TrackTruthDecoratorAlg(const std::string& name,
                          ISvcLocator* pSvcLocator );

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ) const override;

  private:
    // Input Containers
    SG::ReadHandleKey< xAOD::TrackParticleContainer > m_TrackContainerKey {
      this,"trackContainer", "InDetTrackParticles",
        "Key for the input track collection"};

    // Decorators for tracks
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_origin_label {
      this, "ftagTruthOriginLabel", "ftagTruthOriginLabel", 
        "Exclusive origin label of the track"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_type_label {
      this, "ftagTruthTypeLabel", "ftagTruthTypeLabel", 
        "Exclusive truth type label of the track"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_vertex_index {
      this, "ftagTruthVertexIndex", "ftagTruthVertexIndex", 
        "ftagTruth vertex index of the track"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_barcode {
      this, "ftagTruthBarcode", "ftagTruthBarcode", 
        "Barcode of linked truth particle"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_dec_parent_barcode {
      this, "ftagTruthParentBarcode", "ftagTruthParentBarcode", 
        "Barcode of parent of linked truth particle"};

    // Truth origin tool
    ToolHandle<InDet::InDetTrackTruthOriginTool> m_trackTruthOriginTool {
      this, "trackTruthOriginTool", "InDet::InDetTrackTruthOriginTool", 
        "track truth origin tool"};

    // Accessors
    template <typename T> using Acc = SG::AuxElement::ConstAccessor<T>;
    Acc<int> m_acc_truthOriginLabel{"ftagTruthOriginLabel"};
    Acc<int> m_acc_truthTypeLabel{"ftagTruthTypeLabel"};
    Acc<int> m_acc_truthVertexIndex{"ftagTruthVertexIndex"};
    Acc<int> m_acc_truthParentBarcode{"ftagTruthParentBarcode"};

  };
}

#endif
