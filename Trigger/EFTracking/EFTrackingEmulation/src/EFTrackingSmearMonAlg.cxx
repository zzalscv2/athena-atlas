/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "EFTrackingSmearMonAlg.h"


EFTrackingSmearMonAlg::EFTrackingSmearMonAlg( const std::string& name, ISvcLocator* pSvcLocator ) 
: AthHistogramAlgorithm( name, pSvcLocator ){}



StatusCode EFTrackingSmearMonAlg::initialize() {
  ATH_MSG_INFO ("Initializing " << name() << "...");
  ATH_CHECK( m_inputTrackParticleKey.initialize() );
  ATH_CHECK( m_inputTruthParticleKey.initialize() );
  
  return StatusCode::SUCCESS;
}

StatusCode EFTrackingSmearMonAlg::execute() {
  auto ctx = getContext() ;

  SG::ReadHandle<xAOD::TrackParticleContainer> inputTracks_handle( m_inputTrackParticleKey, ctx );
  const xAOD::TrackParticleContainer* inputTracks = inputTracks_handle.cptr();
  if (not inputTracks) {
     ATH_MSG_FATAL("Unable to retrieve input ID tacks");
     return StatusCode::FAILURE;
  }


  ATH_MSG_DEBUG ("Found "<<inputTracks->size()<< " input tracks");
  // trakc particles
  for ( const auto* trk : *inputTracks ) 
    {            
      // get Cov matrix of input track
      xAOD::ParametersCovMatrix_t trkcov = trk->definingParametersCovMatrix();  
      auto trkcovvec = trk->definingParametersCovMatrixVec();        
      ATH_MSG_DEBUG ("Track: "
                      <<" curv=" << 1./trk->pt()
                      <<" phi="  << trk->phi()
                      <<" eta="  << trk->eta()
                      <<" d0="   << trk->d0()
                      <<" z0="   << trk->z0()
                      <<" pT="   << trk->pt()
                      <<" cov_d0=" << trkcov(Trk::d0,Trk::d0)
                      <<" cov_z0=" << trkcov(Trk::z0,Trk::z0)
                      <<" sigma_d0="  << std::sqrt(std::abs(trkcov(Trk::d0,Trk::d0)))
                      <<" sigma_z0="  << std::sqrt(std::abs(trkcov(Trk::z0,Trk::z0))) );      
    }
    

    //truth
    SG::ReadHandle<xAOD::TruthParticleContainer> inputTruth_handle( m_inputTruthParticleKey, ctx );
    const xAOD::TruthParticleContainer* inputTruth = inputTruth_handle.cptr();
    if (not inputTruth) {
        ATH_MSG_FATAL("Unable to retrieve input truth particle");
        return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG ("Found "<<inputTruth->size()<< " input truth particles");
    for ( const auto* part : *inputTruth ) 
    {          
      ATH_MSG_DEBUG ("===> Truth : "       
                      <<" curv=" << 1./part->pt()
                      <<" phi="  << part->phi()
                      <<" eta="  << part->eta()
                      <<" d0="   << part->auxdata<float>("d0")
                      <<" z0="   << part->auxdata<float>("z0")
                      <<" pT="   << part->pt()
                      <<" PDGID=" << part->pdgId()
                      <<" status=" << part->status()                                        
                      ); 
      if (part->parent()) ATH_MSG_DEBUG (" parent pdgId=" << part->parent()->pdgId()); 
    }

  return StatusCode::SUCCESS;
}
