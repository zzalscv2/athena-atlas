/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGTAUREC_TRIGTAURECMERGED_H
#define TRIGTAUREC_TRIGTAURECMERGED_H

#include "GaudiKernel/ToolHandle.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include "tauRecTools/ITauToolBase.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauTrackContainer.h"

class TrigTauRecMerged: public AthReentrantAlgorithm {

 public:

  TrigTauRecMerged(const std::string& name, ISvcLocator* pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:

  template<class T, class U, class V> StatusCode deepCopy(T*& containerOut, U*& containerStoreOut, const V* dummyContainerType,
                                 const T*& oldContainer);
  template<class W, class V, class T> StatusCode deepCopy(W& writeHandle,
                                                          const V* dummyContainerType,
                                                          const T*& oldContainer) const;

  enum TAUEFCALOMON{
    NoROIDescr=0,
    NoCellCont=1,
    EmptyCellCont=2,
    NoClustCont=3,
    NoClustKey=4,
    EmptyClustCont=5,
    NoJetAttach=6,
    NoHLTtauAttach=7,
    NoHLTtauDetAttach=8,
    NoHLTtauXdetAttach=9
  };

  enum TAUEFTRKMON{
    NoTrkCont=0,
    NoVtxCont=1
  };

  /** internal tool store */
  const ToolHandleArray<ITauToolBase> m_commonTools{this, "ComTools", {}, "List of ITauToolBase tools"};
  const ToolHandleArray<ITauToolBase> m_vertexFinderTools{this, "VFTools", {}, "Vertex Finder tools"};
  const ToolHandleArray<ITauToolBase> m_trackFinderTools{this, "TFTools", {}, "Track Finder tools"};
  const ToolHandleArray<ITauToolBase> m_vertexVarsTools{this, "VVTools", {}, "Vertex Variables tools"};
  const ToolHandleArray<ITauToolBase> m_idTools{this, "IDTools", {}, "Vertex Variables tools"};



  // Monitoring tool
  const ToolHandle< GenericMonitoringTool > m_monTool { this, "MonTool", "", "Monitoring tool" };

  //Gaudi::Property< std::string > m_outputName {this,"OutputCollection","TrigTauRecMerged","Name of output collection"};
  SG::ReadHandleKey< TrigRoiDescriptorCollection > m_roIInputKey { this,"RoIInputKey","InputRoI","Input RoI name"};
  SG::ReadHandleKey< xAOD::CaloClusterContainer > m_clustersKey  { this, "clustersKey", "CaloClusters", "caloclusters in view" };
  SG::ReadHandleKey< xAOD::TrackParticleContainer > m_tracksKey  { this, "Key_trackPartInputContainer", "InDetTrackParticles", "input track particle container key"};
  SG::ReadHandleKey< xAOD::VertexContainer> m_vertexKey          { this, "Key_vertexInputContainer", "HLT_IDVertex_Tau", "input vertex container key"};
  SG::ReadHandleKey< xAOD::TauJetContainer> m_trigTauJetKey      { this, "Key_trigTauJetInputContainer", "HLT_taujet", "input taujet container" };
  SG::ReadHandleKey< xAOD::TauTrackContainer> m_trigTauTrackInKey      { this, "Key_trigTauTrackInputContainer", "HLT_tautrack_input", "input tautrack container" };

  SG::WriteHandleKey< xAOD::JetContainer > m_trigtauSeedOutKey   { this,"Key_trigJetSeedOutputKey","HLT_jet_seed","Key for output jets which are seed for tau jets"};
  SG::WriteHandleKey< xAOD::TauJetContainer > m_trigtauRecOutKey {this,"Key_trigTauJetOutputContainer","HLT_taujet","Output taujet container"};
  SG::WriteHandleKey< xAOD::TauTrackContainer > m_trigtauTrkOutKey {this,"Key_trigTauTrackOutputContainer","HLT_tautrack","Output tautrack container"};

};

  // Function to perform deep copy on container
  template<class W, class V, class T>
    StatusCode TrigTauRecMerged::deepCopy(W& writeHandle,
                                          const V* ,
                                          const T*& oldContainer) const {
   if(!writeHandle.isValid()){
      ATH_MSG_FATAL("Provided with an invalid write handle ");
      return StatusCode::FAILURE;
   }
   if(oldContainer != nullptr){
     for( const V* v : *oldContainer ){
       V* newV = new V();
       // Put objects into new container
       writeHandle->push_back(newV);
       // Copy across aux store
       *newV = *v;
     }
   }
   return StatusCode::SUCCESS;
  }

#endif
