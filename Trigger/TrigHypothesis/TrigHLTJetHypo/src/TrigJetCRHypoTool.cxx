/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// ********************************************************************
//
// NAME:     TrigJetCRHypoTool.cxx
// PACKAGE:  Trigger/TrigHypothesis/TrigHLTJetHypo
//
//
// ********************************************************************

#include "TrigJetCRHypoTool.h"
#include "GaudiKernel/StatusCode.h"
#include "CLHEP/Units/PhysicalConstants.h"

#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/xAODJetAsIJetFactory.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/lineSplitter.h"
#include "./DebugInfoCollector.h"
#include "./xAODJetCollector.h"

#include "TrigCompositeUtils/HLTIdentifier.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "xAODTrigger/TrigCompositeContainer.h"

#include "CaloEvent/CaloCellContainer.h"
#include "CaloEvent/CaloCell.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "CaloEvent/CaloClusterContainer.h"

#include "CxxUtils/fpcompare.h"
#include "CxxUtils/phihelper.h"
#include "FourMomUtils/xAODP4Helpers.h"

using TrigCompositeUtils::DecisionID;
using TrigCompositeUtils::Decision;
using TrigCompositeUtils::DecisionContainer;
using TrigCompositeUtils::DecisionIDContainer;
using TrigCompositeUtils::decisionIDs;

using namespace TrigCompositeUtils;

TrigJetCRHypoTool::TrigJetCRHypoTool(const std::string& type,
				     const std::string& name,
				     const IInterface* parent) :
  AthAlgTool(type, name, parent),
  m_decisionId(HLT::Identifier::fromToolName(name)){

}


TrigJetCRHypoTool::~TrigJetCRHypoTool(){
}

StatusCode TrigJetCRHypoTool::initialize(){
  ATH_MSG_VERBOSE( "In TrigJetCRHypoTool, initialize()." ); 

  if (m_visitDebug){

    DebugInfoCollector collector(name());
    ATH_CHECK(m_helper->getDescription(collector));
    auto s = collector.toString();
  
    for(const auto& l : lineSplitter(s)){
      ATH_MSG_INFO(l);
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode TrigJetCRHypoTool::finalize(){
  return StatusCode::SUCCESS;
}

bool TrigJetCRHypoTool::decide_on_single_jet( JetInfo& input ) const {

  auto jet = input.jet;
  auto cellContainer = input.cells;
  auto allTracks= jet->getAttribute<std::vector<ElementLink<xAOD::IParticleContainer> >>("TracksForMinimalJetTag");
  //Checking jet logRatio requirements
  double jetEMF = jet->getAttribute<float>("EMFrac");
  ATH_MSG_DEBUG( " Jet EMF = " << jetEMF << " jet pT = " << jet->pt() << "jet eta = " << jet->eta() << " phi = " << jet->phi() );

  double jetRatio=-999;

  if (CxxUtils::fpcompare::greater(jetEMF,0.)){
    if(CxxUtils::fpcompare::greater_equal(jetEMF,1.)){
      ATH_MSG_DEBUG( "Fails logR cut, EMF>=1" );
      return false;
    }
    else jetRatio = log10(double(1./jetEMF - 1.));
  }
  if ( jetRatio < m_pufixLogRatio) {
       ATH_MSG_DEBUG( "Jet "<< " below the " << m_pufixLogRatio << " threshold for the log-ratio cut; logRatio = " << jetRatio << "; skipping this jet.");
       return false;
  }


  // Loop over all tracks above m_trackPtCut and reject the jet if the closest track is at dR(jet, track)< m_deltaR
  auto jetPhi= jet->phi();
  auto jetEta= jet->eta();
  double pufixLR = -1; 
  
  if(  jetRatio < m_jetlogRCut  ) {
  
      ATH_MSG_DEBUG( "Fails logR cut" << jetRatio << " > " << m_jetlogRCut );
      size_t nClusters = jet->numConstituents();
      double clusterPU_sumEEM = 0; double clusterPU_sumE = 0;
      for (size_t clust = 0; clust < nClusters; clust++) {
         const xAOD::CaloCluster * aCluster = dynamic_cast<const xAOD::CaloCluster*> (jet->rawConstituent(clust));
         double clusEEM = 0;
         clusEEM+=(aCluster)->eSample(CaloSampling::EMB1);
         clusEEM+=(aCluster)->eSample(CaloSampling::EMB2);
         clusEEM+=(aCluster)->eSample(CaloSampling::EMB3);
         clusEEM+=(aCluster)->eSample(CaloSampling::EME1);
         clusEEM+=(aCluster)->eSample(CaloSampling::EME2);
         clusEEM+=(aCluster)->eSample(CaloSampling::EME3);
         clusEEM+=(aCluster)->eSample(CaloSampling::FCAL1);
         double lambda = aCluster->getMomentValue(xAOD::CaloCluster::CENTER_LAMBDA);

         if (lambda > 500) continue;

         double d_eta = aCluster->rawEta() - jetEta;
         double d_phi = xAOD::P4Helpers::deltaPhi(aCluster->rawPhi(),jetPhi);
      
         double d_R2 = d_eta*d_eta + d_phi*d_phi;

         if (d_R2 < 0.15*0.15) continue;
         clusterPU_sumEEM+=clusEEM/1000.;
         clusterPU_sumE+=aCluster->rawE()/1000.;
    }
  
    double jetEEM_EMscale = 0; double jetE_EMscale = 0;  //Working on EM scale because calE() doesn't always return correct EEM and cluster moment EMF not accessable during testing

    std::vector<double> samplingEnergy = jet->getAttribute<std::vector<double> >("EnergyPerSampling");  

    for(size_t s=0; s<samplingEnergy.size(); s++) {

      double samplingE = 0.001*(samplingEnergy.at(s));
      if ( s < 8 || (s > 20 && s < 28) ) jetEEM_EMscale+=samplingE; // EM layers 0-7 and 21-27
      jetE_EMscale+=samplingE; 
    }

    double pufixEMF = (jetEEM_EMscale - clusterPU_sumEEM)/(jetE_EMscale - clusterPU_sumE);

    if (CxxUtils::fpcompare::greater(pufixEMF,0.)){
      if(CxxUtils::fpcompare::greater_equal(pufixEMF,1.0)) pufixLR = -999.;
        else pufixLR = log10(double(1./pufixEMF - 1.));
    } else {
      pufixLR = 999;
    }

    if ( pufixLR < m_jetlogRCut) {
      ATH_MSG_DEBUG( "Jet "<< " is still below the " << m_jetlogRCut << " threshold for the log-ratio cut; recalculated logRatio = " << pufixLR << "; skipping this jet.");
      return false;
    }
    
    jetRatio = pufixLR;
    
  }else {

    if ( jetRatio < m_jetlogRCut) {
        return false;
    } 
    ATH_MSG_DEBUG( "Jet "<< " above the " << m_jetlogRCut<< " threshold for the log-ratio cut; logRatio = " << jetRatio << "; skipping this jet.");
  }
 



  // Loop over all tracks above m_trackPtCut and reject the jet if the closest track is at dR(jet, track)< m_deltaR
  for ( unsigned int index(0); index < allTracks.size(); index++  ) {         
     const xAOD::IParticle* track = *( allTracks.at(index));

     if(track->pt() < m_trackPtCut ) continue;

     double phi  = track->p4().Phi();
     double eta  = track->p4().Eta() ;

     double dR = xAOD::P4Helpers::deltaR( eta, phi, jetEta, jetPhi );
     ATH_MSG_DEBUG(" track with " << "pt=" << track->pt() << ", eta=" << eta << ", phi=" << phi << " dR = " << dR);
     if (dR<m_deltaR)   return false;
  }

  ATH_MSG_DEBUG(" jet passed tracking" );

  if(m_doBIBrm==1){
    int countCaloCell=0;
    double countCell_layer[4] = {0,0,0,0};

    for(CaloCellContainer::const_iterator celliter = cellContainer->begin(); celliter != cellContainer->end(); ++celliter){
      //LoF cell selection in tile:
      if((*celliter)->caloDDE()->is_tile() && (*celliter)->energy() > m_minCellEt){
	double d_phi = xAOD::P4Helpers::deltaPhi( (*celliter)->phi(), jetPhi );
	double d_R = xAOD::P4Helpers::deltaR( (*celliter)->eta(), (*celliter)->phi(), jetEta, jetPhi );
	//select cells in a horizontal line, not being part of the jet and timing consistent with BIB
	ATH_MSG_DEBUG(" cell E " << (*celliter)->energy() << " dPhi " << d_phi << " dR = " << d_R << " time " << (*celliter)->time()  );
	if(std::abs(d_phi) < 0.2 && d_R > 0.4){
          //-early
          float t = (*celliter)->time();
          if(t < m_celltime){
            // check cell's time consitency with BIB
            float x = (*celliter)->x();
            float y = (*celliter)->y();
            float z = (*celliter)->z();
            float r = sqrt(x*x + y*y);

	    if((std::abs(t - (z-sqrt(z*z + r*r))/CLHEP::c_light) < m_dBIBtime) || (std::abs(t - (-z-sqrt(z*z + r*r))/CLHEP::c_light) < m_dBIBtime)){
	      ATH_MSG_DEBUG(" cell is tile; cell E = " << (*celliter)->energy() << " cell phi = " << (*celliter)->phi() << " cell eta = " << (*celliter)->eta() << " cell r = " << r );
	      // for selected cells, store in which layer they are
	      if(r<2200){ countCell_layer[0]++;}
	      else if(r>=2200 && r<2600){ countCell_layer[1]++;}
	      else if(r>=2600 && r<3100){ countCell_layer[2]++;}
	      else if(r>=3100){ countCell_layer[3]++;}
	    }
	  }
	}
      }
    }
  

  // get max number of selected cells in a layer
  for(int i=0; i<4; i++){ 
    if(countCaloCell<countCell_layer[i]) countCaloCell=countCell_layer[i];
  }

    ATH_MSG_DEBUG("Jet Pt " << jet->pt() << "; eta "<< jetEta << "; phi " << jetPhi <<"; logRatio " << jetRatio << "; LoF Cells " << countCaloCell );

    //apply cut on number of cells here
    if (countCaloCell>=m_countCaloCell){
      ATH_MSG_DEBUG( "Jet discarded, identified as BIB"  );    
      return false;
    }
    else {
      ATH_MSG_DEBUG( "Jet is not BIB"  );    
      return true;
    }

  }

  ATH_MSG_DEBUG( "Passed selection" );

  return  true;

}

StatusCode TrigJetCRHypoTool::decide( std::vector<JetInfo>& input )  const{
  for ( JetInfo& j: input ) {
    if ( passed ( m_decisionId.numeric(), j.previousDecisionIDs ) ) {
      if ( decide_on_single_jet( j ) ) {
	        addDecisionID( m_decisionId, j.decision );      
      }
    }
  }
  return StatusCode::SUCCESS;

}

