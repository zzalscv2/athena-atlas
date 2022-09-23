/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************************
 * NAME:     MuHadClusterSubStructVariables.cxx
 * PACKAGE:  as a merged then correction of TauCellVariables & tauSubStructureVariables 
 *           from the offline/Reconstruction/tauRecTools
 *           AUTHORS:  Lianyou SHAN
 **********************************************************************/

#ifndef XAOD_ANALYSIS

#include <algorithm> 
#include <math.h>
#include <sstream>

#include "xAODJet/Jet.h"
#include "xAODTau/TauJet.h"

#include "xAODCaloEvent/CaloVertexedTopoCluster.h"
#include "DiTauRec/MuHadClusterSubStructVariables.h"

#include "tauRecTools/KineUtils.h"

const double MuHadClusterSubStructVariables::DEFAULT = -1111.;

//**********************************
// Constructor
//**********************************

MuHadClusterSubStructVariables::MuHadClusterSubStructVariables( const std::string& name ) :
		TauRecToolBase(name),
		m_maxPileUpCorrection(4 * 1000. ),
		m_pileUpAlpha(1.0),
		m_doVertexCorrection(false) //FF: don't do cell correction by default
		{
	declareProperty("maxPileUpCorrection", m_maxPileUpCorrection);
	declareProperty("pileUpAlpha", m_pileUpAlpha);
	declareProperty("VertexCorrection", m_doVertexCorrection);
        declareProperty("CellCone", m_cellCone);
        declareProperty("OnlyCoreTrack", m_onlyCore = false );
        declareProperty("TrkClassifyDone", m_classifierDone = false ) ;
	declareProperty("ConfigPath", m_configPath);
}

//***********************************
// Initialize method
//***********************************

StatusCode MuHadClusterSubStructVariables::initialize() {

    if ( m_classifierDone ) m_isoTrackType = xAOD::TauJetParameters::classifiedIsolation ;
    else m_isoTrackType = xAOD::TauJetParameters::modifiedIsolationTrack ;

    ATH_MSG_INFO(  " using isoTrackType : " << m_isoTrackType ) ;

	return StatusCode::SUCCESS;
}

StatusCode MuHadClusterSubStructVariables::execute(xAOD::TauJet& pTau) {
	// Getting our hands on the TauJet object
	//----------------------------------------

	// Getting the jet seed
	// By asking taujet instead of TauEventData->seed, we take advantage of the machinery already
	// in place to retrieve a jet seed for track only candidates.
	//------------------------------------------------------------------------------------------------
    const xAOD::Jet* taujetseed = (*pTau.jetLink());

    if (taujetseed == nullptr ) 
    {
      ATH_MSG_ERROR( "Taujet->jet() pointer is NULL: return failure ");
      return StatusCode::FAILURE;
    }

    double totalEnergy(0.);
    double calo_iso(0.);

    unsigned int num_clusters(0);
    std::vector< xAOD::CaloVertexedTopoCluster > constituents;

    TLorentzVector leadClusVec;
    TLorentzVector subLeadClusVec;
    TLorentzVector approxSubstructure4Vec( 0., 0., 0., 0. );
    double clusELead = DEFAULT ;
    double clusESubLead = DEFAULT ;

    static const SG::AuxElement::ConstAccessor< std::vector< double > > accMuonCluster( "overlapMuonCluster" );
    std::vector< double > muCluster_v4 = accMuonCluster( pTau ) ;
    TLorentzVector muCluster ;
    muCluster.SetPtEtaPhiE( muCluster_v4[0], muCluster_v4[1], muCluster_v4[2], muCluster_v4[3] ) ;

	// loop over all clusters of the jet seed
    xAOD::JetConstituentVector jcvec = taujetseed->getConstituents();
    int foundMuon = 0 ;
    for( const xAOD::JetConstituent* nav_it : jcvec ) 
    {
      ++num_clusters;

      const xAOD::CaloCluster*  incluster = dynamic_cast<const xAOD::CaloCluster*>( (*nav_it)->rawConstituent() );
      if (!incluster) continue;  // following conventional treatment in tau group since nothing else seems better here

      TLorentzVector clusP4 = incluster->p4() ;
 
      double dR = Tau1P3PKineUtils::deltaR( pTau.eta(), pTau.phi(), clusP4.Eta(), clusP4.Phi() );
      if ( dR > m_cellCone ) continue ;

      if (    muCluster.Pt() > 0   
           && muCluster.DeltaR( clusP4 ) < 0.05 
           && std::abs( muCluster.Pt() - clusP4.Pt() )/clusP4.Pt() < 0.2  
         )
      {
        ATH_MSG_DEBUG( " overlapping preSelected muon cluster found in MuHadClusterSubStructVariables" )  ;
        foundMuon ++ ;
        continue ;
      }

      if ( pTau.vertexLink() )
        constituents.emplace_back ( *incluster, (*pTau.vertexLink())->position()  ) ;
      else
        constituents.emplace_back ( *incluster );

      // calc total energy
      totalEnergy += incluster->e();
	
      // apply Vertex correction on a temporary
      TLorentzVector tempclusvec = ( constituents.back() ).p4() ;

      dR = Tau1P3PKineUtils::deltaR(pTau.eta(),pTau.phi(), tempclusvec.Eta(), tempclusvec.Phi());
      if ( 0.2 <= dR && dR < 0.4) 
      {
        calo_iso += tempclusvec.Et();
      }
      else if ( dR < 0.2)
      {
        double clusEnergyBE = ( incluster->energyBE(0) + incluster->energyBE(1) + incluster->energyBE(2) );
        if (clusEnergyBE > clusELead) 
        {
	  //change current leading cluster to subleading
          clusESubLead = clusELead;
	  subLeadClusVec = leadClusVec;

	  //set energy and 4-vector of leading cluster
	  clusELead = clusEnergyBE;
	  leadClusVec.SetPtEtaPhiM(clusELead/cosh(tempclusvec.Eta()), tempclusvec.Eta(), tempclusvec.Phi(), 0);
        }
        else if (clusEnergyBE > clusESubLead) 
        {
	  //set energy and 4-vector of subleading cluster only
          clusESubLead = clusEnergyBE;
	  subLeadClusVec.SetPtEtaPhiM(clusESubLead/cosh(tempclusvec.Eta()), tempclusvec.Eta(), tempclusvec.Phi(), 0);
        }
      }
    }

    if (clusELead > 0.) {
      approxSubstructure4Vec += leadClusVec;
    }
    if (clusESubLead > 0.) {
      approxSubstructure4Vec += subLeadClusVec;
    }
	
    // now sort cluster by energy
    auto cmp_e = []( const xAOD::CaloVertexedTopoCluster & lhs, const xAOD::CaloVertexedTopoCluster & rhs ) {
        return lhs.e() > rhs.e() ;
    };
    sort( constituents.begin(), constituents.end(), cmp_e );

    // determine energy sum of leading 2 and leading 3 clusters
    double sum2LeadClusterE(0.);
    double sum3LeadClusterE(0.);

    int m_numConstit = constituents.size() ;
    double m_totMass = DEFAULT ;

    double m_aveRadius = DEFAULT ;
    double sumEt = 0. ;
    double sumEt_Core = 0. ;
    double pt_emScale_EM = 0. ;
    double pt_emScale_HAD = 0. ;
 
    double PSSEnergy(0.);
    double EMEnergy(0.);
    double HADEnergy(0.);

    double sum_px = 0;
    double sum_py = 0;
    double sum_pz = 0;
    double sum_e = 0;
    double sum_of_E2 = 0;
    double sum_radii = 0;
    TLorentzVector centroid = calculateTauCentroid( m_numConstit, constituents );

    //  1'st loopping over clusters 
    int ic = 0 ;
    for ( const xAOD::CaloVertexedTopoCluster & cVl : constituents) 
    {
      double clEnergy = cVl.e();
      if ( ic < 3 )
      {
        sum3LeadClusterE += clEnergy;
        if ( ic < 2 )  sum2LeadClusterE += clEnergy;
      }

      sum_of_E2 += clEnergy*clEnergy;

      double px = cVl.p4().Px();
      double py = cVl.p4().Py();
      double pz = cVl.p4().Pz();

      double dr = std::sqrt( std::pow(cVl.eta() - centroid.Eta(),2) + std::pow(cVl.phi() - centroid.Phi(),2));
      sum_radii += dr;
      sum_e += clEnergy;
      sum_px += px;
      sum_py += py;
      sum_pz += pz;

      double EtOverE = cVl.pt()/clEnergy ;
      //Calculate the fractions of energy in different calorimeter layers
      const xAOD::CaloCluster *cl = &(cVl.clust()) ;
      double PreSampler = cl->eSample(CaloSampling::PreSamplerB) + cl->eSample(CaloSampling::PreSamplerE);
      double EMLayer1   = cl->eSample(CaloSampling::EMB1) + cl->eSample(CaloSampling::EME1);
      double EMLayer2   = cl->eSample(CaloSampling::EMB2) + cl->eSample(CaloSampling::EME2);

      double Energy = cl->rawE();
      double PSSF = Energy != 0. ? (PreSampler + EMLayer1) / Energy : 0.;
      double EM2F = Energy != 0. ? EMLayer2 / Energy : 0.;
      double EMF = PSSF + EM2F;

      PSSEnergy += PSSF * clEnergy;
      double emEnergy  = EMF * clEnergy;
      EMEnergy  += emEnergy;
      double hadEnergy = (Energy != 0. ) ? (1. - EMF) * clEnergy : 0. ;
      HADEnergy += hadEnergy ;

      pt_emScale_EM += (cVl.pt()/clEnergy)*EMF*Energy ;
      pt_emScale_HAD += (cVl.pt()/clEnergy)*( 1. - EMF )*Energy  ;

      sumEt += EtOverE*( emEnergy + hadEnergy ) ;
      if ( dr < 0.1 ) sumEt_Core += EtOverE*( emEnergy + hadEnergy ) ;

      ic ++ ;
    }

    if ( m_numConstit >= 2 )
    {
      double mass2 = sum_e * sum_e - (sum_px * sum_px + sum_py * sum_py + sum_pz * sum_pz);
      m_totMass = mass2 > 0 ? std::sqrt(mass2) : -std::sqrt(-mass2);
    } 
    m_aveRadius = sum_radii / static_cast<float>( m_numConstit ) ;

    // one more loop for Effective stuff
    double m_effMass = DEFAULT ;
    double m_aveEffRadius = DEFAULT ;

    int m_effNumConstit_int = static_cast<int>( ceil( sum_of_E2 > 0 ? (sum_e * sum_e) /(sum_of_E2) : DEFAULT ) ) ;
    if ( ! ( m_effNumConstit_int  > m_numConstit || m_effNumConstit_int < 0 ) )
    {
      sum_px = 0;
      sum_py = 0;
      sum_pz = 0;
      sum_e = 0;
      sum_of_E2 = 0;
      sum_radii = 0;
      centroid = calculateTauCentroid(  m_effNumConstit_int, constituents);

      int icount = m_effNumConstit_int;
      for (const xAOD::CaloVertexedTopoCluster & cVl : constituents) 
      {
        if (icount <= 0) break;
        --icount;

        double energy = cVl.e();
        double px = cVl.p4().Px();
        double py = cVl.p4().Py();
        double pz = cVl.p4().Pz();
        double dr = Tau1P3PKineUtils::deltaR( cVl.eta(), cVl.phi(), centroid.Eta(), centroid.Phi() ) ;
        sum_radii += dr;

        sum_e += energy;
        sum_px += px;
        sum_py += py;
        sum_pz += pz;
      }

      if ( m_effNumConstit_int >= 2 )
      {
        double mass2 = sum_e * sum_e - (sum_px * sum_px + sum_py * sum_py + sum_pz * sum_pz);
        m_effMass = mass2 > 0 ? std::sqrt(mass2) : -std::sqrt(-mass2);
      } 
      if ( m_effNumConstit_int > 0 ) m_aveEffRadius = sum_radii / static_cast<float>( m_effNumConstit_int ) ;
    }

    //record variables
    if ( foundMuon > 0 )
    {
      pTau.setDetail(xAOD::TauJetParameters::topoInvMass ,  static_cast<float>( m_totMass ) ) ;
      pTau.setDetail(xAOD::TauJetParameters::effTopoInvMass ,  static_cast<float>( m_effMass ) ) ;
      pTau.setDetail(xAOD::TauJetParameters::topoMeanDeltaR ,  static_cast<float>( m_aveRadius ) ) ;
      pTau.setDetail(xAOD::TauJetParameters::effTopoMeanDeltaR ,  static_cast<float>( m_aveEffRadius ) ) ;

      if (  totalEnergy != 0 ) {
        pTau.setDetail(xAOD::TauJetParameters::lead2ClusterEOverAllClusterE, static_cast<float>(sum2LeadClusterE / totalEnergy)  );
        pTau.setDetail(xAOD::TauJetParameters::lead3ClusterEOverAllClusterE, static_cast<float>(sum3LeadClusterE / totalEnergy)  );
      } else 
      {
        pTau.setDetail(xAOD::TauJetParameters::lead2ClusterEOverAllClusterE, static_cast<float>( DEFAULT )  );
        pTau.setDetail(xAOD::TauJetParameters::lead3ClusterEOverAllClusterE, static_cast<float>( DEFAULT )  );
      }

      pTau.setDetail(xAOD::TauJetParameters::caloIso, static_cast<float>(calo_iso)  );

      //  there is at least (usually) one muon found within tau cone 
      if ( sumEt > 0.0 && sumEt_Core > 0.0 )
        pTau.setDetail( xAOD::TauJetParameters::centFrac , static_cast<float>( sumEt_Core/sumEt ) ) ;

      if ( pt_emScale_EM > 0. ) 
        pTau.setDetail( xAOD::TauJetParameters::etEMAtEMScale , static_cast<float>( pt_emScale_EM ) ) ;
      if ( pt_emScale_HAD > 0. ) 
        pTau.setDetail(xAOD::TauJetParameters::etHadAtEMScale , static_cast<float>( pt_emScale_HAD ) ) ;

      double fPSSFraction                  = (totalEnergy != 0) ? PSSEnergy / totalEnergy : DEFAULT;
      pTau.setDetail(xAOD::TauJetParameters::PSSFraction,         static_cast<float>(fPSSFraction));

    }

    //  PileUp correction along PtSum 
    double  jvf(0.0);
    double  sumPtTrk(0.0);

    static const SG::AuxElement::ConstAccessor< std::vector< double > > accMuonTrack( "overlapMuonTrack" );
    std::vector< double > muTrack_v4 = accMuonTrack( pTau ) ;
    TLorentzVector muTrack ;
    muTrack.SetPtEtaPhiE( muTrack_v4[0], muTrack_v4[1], muTrack_v4[2], muTrack_v4[3] ) ;
    float mupt = muTrack.Pt() ;

	// for tau trigger: JVF and sumPtTrack are not available
    bool inTrigger = tauEventData()->inTrigger();

    if (!inTrigger)
    {
      std::vector<float> sumPtTrkvec;
      std::vector<float> jvfvec;

      taujetseed->getAttribute(xAOD::JetAttribute::SumPtTrkPt500, sumPtTrkvec);
      taujetseed->getAttribute(xAOD::JetAttribute::JVF, jvfvec);

      if (!jvfvec.empty() && !sumPtTrkvec.empty()) {
	// ToDo need to check if first vertex is the vertex we want to use here!
 	jvf = jvfvec[0];
	sumPtTrk = sumPtTrkvec[0];
      } else 
      {
        if ( sumPtTrkvec.empty() ) sumPtTrk = mupt ;
      }

      // get muon pt from decoration and has it substract :
      sumPtTrk = sumPtTrk - mupt ;
    }

    float pt_pileup = (1.0 - jvf) * sumPtTrk;

    const float max_pileup_correction = m_maxPileUpCorrection;
    const float alpha = m_pileUpAlpha;
    float pileup_correction = alpha * pt_pileup;  
    //  jvfvec.empty() or sumPtTrkvec.empty() is expected very rare, where the correction will vanish

    if (pileup_correction > max_pileup_correction) {
       pileup_correction = max_pileup_correction;
    }
    const float calo_iso_corrected = calo_iso - pileup_correction;

    pTau.setDetail(xAOD::TauJetParameters::caloIsoCorrected, static_cast<float>(calo_iso_corrected)  );


//  more variables with trakcs involved 
    double dRmin = -1 * DEFAULT;
    double dRmax = DEFAULT ;

    double ptSum = 0.;
    double innerPtSum = 0.;
    double sumWeightedDR = 0.;
    double innerSumWeightedDR = 0.;
    double sumWeightedDR2 = 0.;

    double trkSysMomentum(0.);
    double leadTrkPt = DEFAULT ;
    TLorentzVector sumOfTrackVector( 0., 0., 0., 0. ) ;

    std::vector<const xAOD::TauTrack*> tauTracks = pTau.tracks( xAOD::TauJetParameters::TauTrackFlag::classifiedCharged );
    unsigned int ncct = tauTracks.size() ;

    if ( ncct > 0 ) leadTrkPt = tauTracks[0]->pt() ;

    for( const xAOD::TauTrack* trk : pTau.tracks( m_isoTrackType ) ) tauTracks.push_back(trk);

    for (const xAOD::TauTrack* tau_trk : tauTracks)
    {
      if ( tau_trk == 0 ) continue ;

      bool coreTrack = tau_trk->flag( xAOD::TauJetParameters::TauTrackFlag::classifiedCharged ) ;
      bool modifIso = tau_trk->flag( m_isoTrackType ) ;

      if ( ! ( coreTrack || modifIso ) ) continue ;

      TLorentzVector trkTLV = tau_trk->p4() ;
      sumOfTrackVector += trkTLV ;

      double dR1 = Tau1P3PKineUtils::deltaR( pTau.etaIntermediateAxis(), pTau.phi(), tau_trk->eta(), tau_trk->phi() ) ;
      double trkPt = tau_trk->pt();

      ptSum += trkPt ;
      sumWeightedDR += dR1 * trkPt;
      sumWeightedDR2 += dR1 * dR1 * trkPt;

      if ( m_onlyCore  && ! coreTrack )  continue ;

      if ( coreTrack ) 
      { 
        innerPtSum += trkPt ;
        innerSumWeightedDR += dR1 * trkPt;
      }

      trkSysMomentum += ( tau_trk->pt() )*cosh( tau_trk->eta() );
      approxSubstructure4Vec += trkTLV ;

      double dR2 = Tau1P3PKineUtils::deltaR( tau_trk->eta(), tau_trk->phi(), pTau.eta(), pTau.phi() );

      if ( dRmin > dR2 && ncct > 0 ) dRmin = dR2;
      if ( dRmax < dR2 && ncct > 0 ) dRmax = dR2;

    }

    pTau.setDetail(xAOD::TauJetParameters::ptRatioEflowApprox,
                                 static_cast<float>( approxSubstructure4Vec.Pt()/ pTau.ptDetectorAxis()) );

    pTau.setDetail(xAOD::TauJetParameters::mEflowApprox, static_cast<float>( approxSubstructure4Vec.M() ) );

    double fChPIEMEOverCaloEME   = (EMEnergy != 0) ? (trkSysMomentum - HADEnergy) / EMEnergy : DEFAULT;
    double fEMPOverTrkSysP = (trkSysMomentum != 0) ? EMEnergy / trkSysMomentum : DEFAULT;

    pTau.setDetail(xAOD::TauJetParameters::ChPiEMEOverCaloEME,  static_cast<float>(fChPIEMEOverCaloEME));
    pTau.setDetail(xAOD::TauJetParameters::EMPOverTrkSysP,              static_cast<float>(fEMPOverTrkSysP));
    if ( leadTrkPt > 0. )
      pTau.setDetail( xAOD::TauJetParameters::etOverPtLeadTrk,
                                         static_cast<float>( ( pt_emScale_EM + pt_emScale_HAD ) / leadTrkPt ) );
    else
      pTau.setDetail( xAOD::TauJetParameters::etOverPtLeadTrk, static_cast<float>( DEFAULT ) ) ;

    pTau.setDetail(xAOD::TauJetParameters::dRmax, static_cast<float>(dRmax)  );

    pTau.setDetail( xAOD::TauJetParameters::massTrkSys, static_cast<float>( sumOfTrackVector.M() ) );

    if ( ptSum > 0.0001) 
    {
      // seedCalo_trkAvgDist
      pTau.setDetail( xAOD::TauJetParameters::trkAvgDist, static_cast<float>( sumWeightedDR / ptSum ) );

      double trkRmsDist2 = sumWeightedDR2 / ptSum - sumWeightedDR * sumWeightedDR / ptSum / ptSum;
      if (trkRmsDist2 > 0) {
        pTau.setDetail( xAOD::TauJetParameters::trkRmsDist, static_cast<float>( std::sqrt(trkRmsDist2) ) );
      } 
    }

    if ( ptSum > 0.0 ) 
    {
      // SumPtTrkFrac
      pTau.setDetail( xAOD::TauJetParameters::SumPtTrkFrac, static_cast<float>( 1. - innerPtSum/ptSum ) );

      if ( innerPtSum > 0.0) pTau.setDetail( xAOD::TauJetParameters::innerTrkAvgDist, 
                                              static_cast<float>( innerSumWeightedDR / innerPtSum ) );
      else pTau.setDetail( xAOD::TauJetParameters::innerTrkAvgDist,
                                              static_cast<float>( 0.0 ) );
    } else 
      pTau.setDetail( xAOD::TauJetParameters::SumPtTrkFrac, static_cast<float>( 0.0 ) );

    return StatusCode::SUCCESS;
}

TLorentzVector MuHadClusterSubStructVariables::calculateTauCentroid( int nConst,   
                                 const std::vector< xAOD::CaloVertexedTopoCluster >& constituents  )
{
    double px = 0;
    double py = 0;
    double pz = 0;
    double current_px, current_py, current_pz, modulus;

    for (const xAOD::CaloVertexedTopoCluster & c : constituents) {
      if (nConst <= 0) break;
      --nConst;
      current_px = c.p4().Px();
      current_py = c.p4().Py();
      current_pz = c.p4().Pz();
      modulus = std::sqrt(current_px * current_px + current_py * current_py + current_pz * current_pz);
      px += current_px / modulus;
      py += current_py / modulus;
      pz += current_pz / modulus;
    }
    TLorentzVector centroid(px, py, pz, 1);
    return centroid;
}

#endif
