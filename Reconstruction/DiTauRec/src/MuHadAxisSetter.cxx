/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS
//tau
#include "tauRecTools/TauEventData.h"

#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauJetAuxContainer.h"
#include "xAODTau/TauJet.h"

#include "CaloUtils/CaloVertexedCluster.h"

#include "DiTauRec/MuHadAxisSetter.h"


/********************************************************************/
MuHadAxisSetter::MuHadAxisSetter(const std::string& name) :
TauRecToolBase(name),
m_clusterCone(0.2),
m_doAxisCorrection(true)
{
    declareProperty("ClusterCone", m_clusterCone);
    declareProperty("AxisCorrection", m_doAxisCorrection = true);
}

/********************************************************************/
StatusCode MuHadAxisSetter::initialize()
{
    return StatusCode::SUCCESS;
}

StatusCode MuHadAxisSetter::eventInitialize() 
{
    return StatusCode::SUCCESS;
      
}

/********************************************************************/
StatusCode MuHadAxisSetter::execute(xAOD::TauJet& pTau)
{
    const xAOD::Jet* pJetSeed = (*pTau.jetLink());
    if (!pJetSeed) {
        ATH_MSG_WARNING("tau does not have jet seed for LC calibration");
        return StatusCode::SUCCESS;
    }

    pTau.clearClusterLinks();

    static const SG::AuxElement::ConstAccessor< std::vector< double > >accMuonCluster( "overlapMuonCluster" ) ;
    std::vector< double > muCluster_v4 = accMuonCluster( pTau ) ;
    TLorentzVector muCluster ;
    muCluster.SetPtEtaPhiE( muCluster_v4[0], muCluster_v4[1], muCluster_v4[2], muCluster_v4[3] ) ;

    xAOD::JetConstituentVector jcvec = pJetSeed->getConstituents() ;

    ///////////////////////////////////////////////////////////////////////////
    //calculate barycenter
    TLorentzVector sumAllClusterVector;
    TLorentzVector tempClusterVector;
    for( const xAOD::JetConstituent* cItr : jcvec ) 
    {
      tempClusterVector.SetPtEtaPhiE( (*cItr)->pt(), (*cItr)->eta(), (*cItr)->phi(), (*cItr)->e() );

      if  (  muCluster.Pt() > 0. )
      {
        float dR = muCluster.DeltaR( tempClusterVector ) ;
        float dPt = std::abs( muCluster.Pt() - tempClusterVector.Pt() )/tempClusterVector.Pt() ;
        if ( dR < 0.05 && dPt < 0.2 ) continue ;
      }

      sumAllClusterVector += tempClusterVector;
    }
    TLorentzVector BaryCenter; 
    BaryCenter.SetPtEtaPhiM(1., sumAllClusterVector.Eta(), sumAllClusterVector.Phi(), 0.);
    
    ///////////////////////////////////////////////////////////////////////////
    // calculate detector axis
    TLorentzVector tauDetectorAxis;
    // count number of constituents in core cone. could be zero!
    int nConstituents = 0;
    for( const xAOD::JetConstituent* cItr : jcvec )
    {
	tempClusterVector.SetPtEtaPhiE( (*cItr)->pt(), (*cItr)->eta(), (*cItr)->phi(), (*cItr)->e() );

	ATH_MSG_DEBUG(" MR cluster in detector axis loop:" << (*cItr)->pt()<< " " << (*cItr)->eta() << " " << (*cItr)->phi()  << " " << (*cItr)->e() );
	ATH_MSG_DEBUG("delta R is " << BaryCenter.DeltaR(tempClusterVector) );

        if (BaryCenter.DeltaR(tempClusterVector) > m_clusterCone)
            continue;

        if (    muCluster.Pt() > 0.
             && muCluster.DeltaR( tempClusterVector ) < 0.05
             && std::abs( muCluster.Pt() - tempClusterVector.Pt() )/tempClusterVector.Pt() < 0.2 
           )
        {
          ATH_MSG_DEBUG( " overlapping muon cluster found in MRMuHadAxisSetter::execute::DetectorAxis " ) ;
          continue ;
        }

	ElementLink<xAOD::IParticleContainer> linkToCluster;
	linkToCluster.toContainedElement( *(static_cast<const xAOD::IParticleContainer*> ((*cItr)->rawConstituent()->container())), (*cItr)->rawConstituent() );
	pTau.addClusterLink(linkToCluster);

	nConstituents++;
	tauDetectorAxis += tempClusterVector;
    }
    
    if  (nConstituents == 0)
    {
      ATH_MSG_WARNING("this tau candidate does not have any constituent clusters! continue without updating this candidate!") ;
      return StatusCode::SUCCESS ;
    }

    ATH_MSG_DEBUG(" MR jet axis:" << (*pTau.jetLink())->pt()<< " " << (*pTau.jetLink())->eta() << " " << (*pTau.jetLink())->phi()  << " " << (*pTau.jetLink())->e() );
    // save values for detector axis.
    ATH_MSG_DEBUG(" MR detector axis:" << tauDetectorAxis.Pt()<< " " << tauDetectorAxis.Eta() << " " << tauDetectorAxis.Phi()  << " " << tauDetectorAxis.E() );

    // detectorAxis (set default) 
    pTau.setP4(tauDetectorAxis.Pt(), tauDetectorAxis.Eta(), tauDetectorAxis.Phi(), pTau.m());
    // save detectorAxis 
    pTau.setP4(xAOD::TauJetParameters::DetectorAxis, tauDetectorAxis.Pt(), tauDetectorAxis.Eta(), tauDetectorAxis.Phi(), tauDetectorAxis.M());

    ///////////////////////////////////////////////////////////////////////////
    // calculate tau intermediate axis (corrected for tau vertex)
    // not needed at trigger level
    if(m_doAxisCorrection)
    {
      TLorentzVector tauInterAxis;
	
      for( const xAOD::JetConstituent* cItr : jcvec )
      {
        tempClusterVector.SetPtEtaPhiE( (*cItr)->pt(), (*cItr)->eta(), (*cItr)->phi(), (*cItr)->e() );

	ATH_MSG_DEBUG(" MR cluster in detector axis loop:" << (*cItr)->pt()<< " " << (*cItr)->eta() << " " << (*cItr)->phi()  << " " << (*cItr)->e() );
	ATH_MSG_DEBUG("delta R is " << BaryCenter.DeltaR(tempClusterVector) );

        if (BaryCenter.DeltaR(tempClusterVector) > m_clusterCone) continue;
	  
        const xAOD::CaloCluster* cluster = dynamic_cast<const xAOD::CaloCluster*>( (*cItr)->rawConstituent() ); 
        if (!cluster) continue;  // acceptable in phys to skip a bad cluster, other choice is neither better.
	  
        if (    muCluster.Pt() > 0.
             && muCluster.DeltaR(  cluster->p4()  ) < 0.05
             && std::abs( muCluster.Pt() - cluster->et() )/cluster->et() < 0.2 
           )
        {
          ATH_MSG_DEBUG( " overlapping muon cluster found in MRMuHadAxisSetter::execute::AxisCorrection " ) ;
          continue ;
        }

        if (pTau.vertexLink())
        {
          tauInterAxis += xAOD::CaloVertexedCluster(*cluster, (*pTau.vertexLink())->position()).p4();
        }
        else
          tauInterAxis += xAOD::CaloVertexedCluster(*cluster).p4();
      }
	
	// save values for tau intermediate axis
	// energy will be overwritten by EnergyCalibrationLC (if correctEnergy is enabled)
	// direction will be overwritten by EnergyCalibrationLC (if correctAxis is enabled)
	
	// intermediate axis( set default) 
      pTau.setP4( tauInterAxis.Pt(), tauInterAxis.Eta(), tauInterAxis.Phi(), pTau.m() );
	
      ATH_MSG_DEBUG(" MR tau axis:" << tauInterAxis.Pt()<< " " << tauInterAxis.Eta() << " " << tauInterAxis.Phi()  << " " << tauInterAxis.E() );
	
	// save intermediateAxis 
      pTau.setP4(xAOD::TauJetParameters::IntermediateAxis, tauInterAxis.Pt(), tauInterAxis.Eta(), tauInterAxis.Phi(), tauInterAxis.M());
    }
    
    return StatusCode::SUCCESS;
}

#endif
