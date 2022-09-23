/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS
//-----------------------------------------------------------------------------

#include <algorithm>
#include <math.h>
#include <sstream>

#include "CaloUtils/CaloVertexedCell.h"

#include "xAODTau/TauJet.h"
#include "xAODJet/Jet.h"
#include "xAODTau/TauTrackContainer.h"
#include "tauRecTools/KineUtils.h"
#include "xAODCaloEvent/CaloVertexedTopoCluster.h"
#include "TrkParametersIdentificationHelpers/TrackParametersIdHelper.h"
#include "RecoToolInterfaces/IParticleCaloExtensionTool.h"
#include "DiTauRec/MuHadElectronVetoVariables.h"

using CLHEP::GeV;

const double MuHadElectronVetoVariables::DEFAULT = -1111.;

//-------------------------------------------------------------------------
// Constructor
//-------------------------------------------------------------------------
MuHadElectronVetoVariables::MuHadElectronVetoVariables(const std::string &name) :
TauRecToolBase(name),
m_doCellCorrection(false), //FF: don't do cell correction by default
m_caloExtensionTool("Trk::ParticleCaloExtensionTool/ParticleCaloExtensionTool")
{
    declareProperty("CellCorrection", m_doCellCorrection);
    declareProperty("ParticleCaloExtensionTool",   m_caloExtensionTool );
}

MuHadElectronVetoVariables::~MuHadElectronVetoVariables() {} 

//-------------------------------------------------------------------------
// Initializer
//-------------------------------------------------------------------------
StatusCode MuHadElectronVetoVariables::initialize()
{
    if (m_caloExtensionTool.retrieve().isFailure()) {
      ATH_MSG_ERROR("Cannot find tool named <" << m_caloExtensionTool << ">");
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}

//-------------------------------------------------------------------------
// Execution
//-------------------------------------------------------------------------
StatusCode MuHadElectronVetoVariables::execute(xAOD::TauJet& pTau)
{

    if (pTau.nTracks() < 1) {
        return StatusCode::SUCCESS;
    }

    ATH_MSG_VERBOSE(name() << " in execute() ...");

    float energy_3phi[101] = {0.0};
    float eta[101] = {0.0};
    int max1 = 0;
    int max2 = 0;
    int max = 0;
    int n = 0;
    float Emax1 = 0.;
    float Emax2 = 0.;

    const xAOD::TauTrack *tauTrk = pTau.track(0) ;

    float signum_eta = ( tauTrk->eta() >= 0. ?  1. : -1. ) ;  

    float sumETCellsLAr = 0.;

    float etareg = 0.;
    float sumETCellsHad1 = 0. ;

    //use tau vertex to correct cell position
    bool applyCellCorrection = m_doCellCorrection && pTau.vertexLink() ;

    //---------------------------------------------------------------------
    // Calculate eta, phi impact point of leading track at calorimeter layers EM 0,1,2,3
    //---------------------------------------------------------------------
    Trk::TrackParametersIdHelper parsIdHelper;
        
    const int numOfsampEM = 4;
    double eta_extrapol[4];
    double phi_extrapol[4];

    for (int i = 0; i < numOfsampEM; ++i) {
      eta_extrapol[i] = DEFAULT ;
      phi_extrapol[i] = DEFAULT ;
    }

    // get the extrapolation into the calo
    const Trk::CaloExtension* caloExtension = nullptr ;
    if( !m_caloExtensionTool->caloExtension( *tauTrk->track(), caloExtension) || caloExtension->caloLayerIntersections().empty() ){
      ATH_MSG_WARNING("extrapolation of leading track to calo surfaces failed  " );
      return StatusCode::SUCCESS;
    }

    // loop over calo layers
    for( const auto cur : caloExtension->caloLayerIntersections() ){
      
      // only use entry layer
      if( !parsIdHelper.isEntryToVolume(cur->cIdentifier()) ) continue;
      
      CaloSampling::CaloSample sample = parsIdHelper.caloSample(cur->cIdentifier());
      int index = -1;
      if( sample == CaloSampling::PreSamplerE || sample == CaloSampling::PreSamplerB ) index = 0;
      else if( sample == CaloSampling::EME1 || sample == CaloSampling::EMB1 )          index = 1;
      else if( sample == CaloSampling::EME2 || sample == CaloSampling::EMB2 )          index = 2;
      else if( sample == CaloSampling::EME3 || sample == CaloSampling::EMB3 )          index = 3;
      if( index < 0 ) continue;
      eta_extrapol[index] = cur->position().eta();
      phi_extrapol[index] = cur->position().phi();
    }

    for (int i = 0; i < numOfsampEM; ++i) {
      if ( eta_extrapol[i] <= DEFAULT || phi_extrapol[i] <= DEFAULT )
	{
	  ATH_MSG_DEBUG("extrapolation of leading track to calo surfaces failed for sampling : " << i );
	  return StatusCode::SUCCESS;
	}
    }

    static const SG::AuxElement::ConstAccessor< std::vector< double > > accMuonCluster( "overlapMuonCluster" );
    std::vector< double > muCluster_v4 = accMuonCluster( pTau ) ;
    TLorentzVector muCluster ;
    muCluster.SetPtEtaPhiE( muCluster_v4[0], muCluster_v4[1], muCluster_v4[2], muCluster_v4[3] ) ;

    int foundMuon = 0 ;

    const xAOD::Jet* pJetSeed = (*pTau.jetLink());
    // a sufficiently bad jet will be anyway rejected in downstream algos
    if (!pJetSeed) 
    {
      ATH_MSG_WARNING(  "Careful : a tau without its link to seedjet ! " ) ;  
      return StatusCode::SUCCESS;
    }

    xAOD::JetConstituentVector jcvec = pJetSeed->getConstituents() ;

    for( const xAOD::JetConstituent* cItr : jcvec )
    {
      
      const xAOD::CaloCluster* cluster = dynamic_cast<const xAOD::CaloCluster*>( (*cItr)->rawConstituent() ); 

      std::unique_ptr< xAOD::CaloVertexedTopoCluster > vtxCls( applyCellCorrection ? 
        new xAOD::CaloVertexedTopoCluster( *cluster, (*pTau.vertexLink())->position() ) :
                    new xAOD::CaloVertexedTopoCluster( *cluster ) 
                               )  ;

      TLorentzVector clusP4 = vtxCls->p4() ;

      if (    muCluster.Pt() > 0
           && muCluster.DeltaR( clusP4 ) < 0.05 
           && std::abs( muCluster.Pt() - clusP4.Pt() )/clusP4.Pt() < 0.2
         )
      {
        ATH_MSG_DEBUG( " overlapping preSelected muon cluster found in MRtauClusterSubStructVariables" )  ;
        foundMuon ++ ;
        continue ;
      }

      double cellPhi = clusP4.Phi() ;
      double cellEta = clusP4.Eta() ;
	
      float fet = cluster->rawE() == 0. ?  0. : cluster->et()/cluster->rawE() ;

      float detPhiTrk[4], detEtaTrk[4], clEtaTrk[4], distEtaTrk[4] ;

      for ( int smpl = 0 ; smpl < 20 ; smpl ++ )
      {
        int i = FromSamplingToIndex( smpl ) ;
        detPhiTrk[i] = Tau1P3PKineUtils::deltaPhi( cellPhi, phi_extrapol[i] );
        detEtaTrk[i] = std::abs( cellEta - eta_extrapol[i] );
        clEtaTrk[i] = eta_extrapol[i];
        distEtaTrk[i] = cellEta - eta_extrapol[i];
      }

      if ( detEtaTrk[0] < eta0cut && detPhiTrk[0] < phi0cut ) sumETCellsLAr += 
                         fet*( cluster->eSample(CaloSampling::PreSamplerB ) + cluster->eSample(CaloSampling::PreSamplerE) ) ;
      if ( detEtaTrk[1] < eta1cut && detPhiTrk[1] < phi1cut ) sumETCellsLAr += 
                         fet*( cluster->eSample(CaloSampling::EMB1 ) + cluster->eSample(CaloSampling::EME1) ) ;
      if ( detEtaTrk[2] < eta2cut && detPhiTrk[2] < phi2cut ) sumETCellsLAr += 
                         fet*( cluster->eSample(CaloSampling::EMB2 ) + cluster->eSample(CaloSampling::EME2) ) ;
      if ( detEtaTrk[3] < eta3cut && detPhiTrk[3] < phi3cut ) sumETCellsLAr += 
                         fet*( cluster->eSample(CaloSampling::EMB3 ) + cluster->eSample(CaloSampling::EME3) ) ;
     

      if ( detEtaTrk[3] < etahadcut && detPhiTrk[3] < phihadcut) 
      {
        sumETCellsHad1 += fet*( 
                      cluster->eSample(CaloSampling::TileBar0) + cluster->eSample(CaloSampling::TileExt0) 
                    + cluster->eSample(CaloSampling::HEC0) + cluster->eSample(CaloSampling::TileGap1) ) ;
        if (  std::abs(cellEta) > 0.8 && std::abs(cellEta) <= 1.2  )
        {
          sumETCellsHad1 += fet*( 
            cluster->eSample(CaloSampling::TileBar1) + cluster->eSample(CaloSampling::TileGap2) + cluster->eSample(CaloSampling::TileExt1 )
          + cluster->eSample(CaloSampling::TileBar2) + cluster->eSample(CaloSampling::TileGap3) + cluster->eSample(CaloSampling::TileExt2 ) ) ;
        }
      }

      if (    std::abs( tauTrk->eta()) <= 1.7 
           && detEtaTrk[1] < etamaxcut 
           && detPhiTrk[1] <= phimaxcut 
           && ( std::abs(cellEta) < 1.37 || std::abs(cellEta) > 1.52 ) 
           && std::abs(cellEta) < 1.85 )
      {
        if (std::abs(clEtaTrk[1]) <= etacase1 && std::abs(cellEta) <= etacase1) {
          n = 50 + static_cast<int>(distEtaTrk[1] / etagran1);
        }
        if (std::abs(clEtaTrk[1]) <= etacase1 && std::abs(cellEta) > etacase1) {
          n = 50 + static_cast<int>(signum_eta * ((etacase1 - std::abs(clEtaTrk[1])) / etagran1 + (-etacase1 + std::abs(cellEta)) / etagran2));
        }
        energy_3phi[n] += cluster->eSample(CaloSampling::EMB1)/ GeV;
        eta[n] = signum_eta * (clEtaTrk[1] - cellEta);
      } else {
        energy_3phi[n] = 0;
        eta[n] = 0;
      }

      if ( std::abs(cellEta) <= etacase1 ) etareg = 0.00315*13.;
      else etareg = 0.00415*13.;

    }// end jet constituent loop

    for (int m1 = 0; m1 < 101; m1++) {
        if ((energy_3phi[m1] > Emax1)) {
            Emax1 = energy_3phi[m1];
            max1 = m1;
        }
    }

    for (int m2 = 1; m2 < 100; m2++) {
        if (m2 == max1) continue;

        if ((energy_3phi[m2] > Emax2) && (energy_3phi[m2] > energy_3phi[m2 - 1]) && (energy_3phi[m2] > energy_3phi[m2 + 1])) {
            Emax2 = energy_3phi[m2];
            max2 = m2;
        }
    }

    if (std::abs(eta[max1]) >= etareg) {
        max = max1;
    } else {
        max = max2;
    }

    float TRTratio = 0.0;
    uint8_t TRTHTHits;
    uint8_t TRTHTOutliers;
    uint8_t TRTHits;
    uint8_t TRTOutliers;

    const xAOD::TrackParticle* leadTrack = tauTrk->track();
      
    if ( !leadTrack->summaryValue( TRTHits, xAOD::SummaryType::numberOfTRTHits ) )
    {
      ATH_MSG_DEBUG("retrieval of track summary value failed. Not filling electron veto variables for this one prong candidate");
      return StatusCode::SUCCESS;
    }

    if ( !leadTrack->summaryValue( TRTHTHits, xAOD::SummaryType::numberOfTRTHighThresholdHits ) )
    {
      ATH_MSG_DEBUG("retrieval of track summary value failed. Not filling electron veto variables for this one prong candidate");
      return StatusCode::SUCCESS;
    }

    if ( !leadTrack->summaryValue( TRTOutliers, xAOD::SummaryType::numberOfTRTOutliers ) )
    {
      ATH_MSG_DEBUG("retrieval of track summary value failed. Not filling electron veto variables for this one prong candidate");
      return StatusCode::SUCCESS;
    }

    if ( !leadTrack->summaryValue( TRTHTOutliers, xAOD::SummaryType::numberOfTRTHighThresholdOutliers ) )
    {
      ATH_MSG_DEBUG("retrieval of track summary value failed. Not filling electron veto variables for this one prong candidate");
      return StatusCode::SUCCESS;
    }
      
    if (TRTHits + TRTOutliers != 0) 
      TRTratio = float( TRTHTHits + TRTHTOutliers) / float( TRTHits + TRTOutliers );
 
    pTau.setDetail(xAOD::TauJetParameters::TRT_NHT_OVER_NLT , TRTratio );
    pTau.setDetail(xAOD::TauJetParameters::secMaxStripEt , energy_3phi[max] );
    pTau.setDetail(xAOD::TauJetParameters::hadLeakEt , static_cast<float>( sumETCellsHad1 / ( tauTrk->pt() ) ) );
    pTau.setDetail(xAOD::TauJetParameters::sumEMCellEtOverLeadTrkPt , static_cast<float>( ( sumETCellsLAr / ( tauTrk->pt() ) ) ) );

    return StatusCode::SUCCESS;
}

int MuHadElectronVetoVariables::FromSamplingToIndex( int sampling )
{
  int idx = -1 ;

  if ( sampling < 4) idx = sampling ; 
  else if ( sampling <= 7 ) idx = sampling - 4 ;
  else if (  sampling == 8 || ( sampling >= 12 && sampling <= 20 ) ) idx = 3;
  else if ( sampling >= 9 && sampling <= 11 ) idx = 2 ;

  return idx ;
}

#endif
