/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EFTRACKING_SMEARINGMONALG_H
#define EFTRACKING_SMEARINGMONALG_H 

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "xAODTracking/TrackParticleContainer.h" 
#include "xAODTracking/TrackParticleAuxContainer.h" 
#include "xAODTracking/TrackParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"

class EFTrackingSmearMonAlg: public ::AthHistogramAlgorithm { 
 public: 
  EFTrackingSmearMonAlg( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~EFTrackingSmearMonAlg(){}; 

  virtual StatusCode  initialize() override;
  virtual StatusCode  execute() override;
  

 private: 


  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_inputTrackParticleKey { this, "InputTrackParticleContainer", "InDetTrackParticles_tosmear",
                                                                          "key for retrieval of input TrackParticles" };

  
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_inputTruthParticleKey{this,"InputTruthParticleContainer","TruthParticles_tosmear",
                                                                         "key for retrieval of input Truth particle"};
  

}; 

#endif //> !EFTRACKING_SMEARINGMONALG_H
