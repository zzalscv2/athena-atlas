/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigTRTHTHCounter.h"
#include "GaudiKernel/IssueSeverity.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"

#include "InDetIdentifier/TRT_ID.h"
#include "Identifier/IdentifierHash.h"
#include "InDetPrepRawData/TRT_DriftCircleContainer.h" 
#include "TrigInDetToolInterfaces/ITrigTRT_DriftCircleProvider.h"

#include "TrigCaloEvent/TrigEMCluster.h"
#include "xAODTrigRinger/TrigRNNOutput.h"
#include "TMath.h"
#include "GeoPrimitives/GeoPrimitives.h"

#include<cmath>
#include<fstream>

const double PI = TMath::Pi();
const double TWOPI = 2.0*PI;

//Function to return deltaPhi between -PI and PI
double hth_delta_phi(const float& phi1, const float& phi2){
  float PHI=fabs(phi1-phi2);
  return (PHI<=PI)? PHI : TWOPI-PHI;
}

//Function to check whether eta of TRT hit is acceptable
bool hth_eta_match(const float& caleta, const float& trteta, const float& etaWindow){
  bool r=false;
  if (fabs(caleta)<etaWindow || caleta*trteta>0.) r = true;
  return r;
}

//Function to calculate distance for road algorithm
float dist2COR(float R, float phi1, float phi2){
  float PHI=fabs(phi1-phi2);
  return fabs(R*sin(PHI));
}

//TRT hit struct used for convenience
struct TRT_hit {
  float phi;
  float R;
  bool isHT;
};

//Constructor of above struct
TRT_hit make_hit(float phi, float R, bool isHT){
  TRT_hit my_hit={phi,R,isHT};
  return my_hit;
}


//---------------------------------------------------------------------------------

TrigTRTHTHCounter::TrigTRTHTHCounter(const std::string& name, ISvcLocator* pSvcLocator)
  : HLT::FexAlgo(name, pSvcLocator),
    m_detStore("DetectorStore", name),
    m_storeGate("StoreGateSvc", name),
    m_trtHelper(0),
    m_rawDataTool("TrigTRT_DriftCircleProviderTool"),
    m_phiHalfWidth(0.1),
    m_etaHalfWidth(0.1),
    m_doFullScan(false),
    m_maxCaloEta(1.7),
    m_roadWidth(4.),
    m_nBinCoarse(14),
    m_nBinFine(14),
    m_wedgeMinEta(0),
    m_roadMaxEta(1.06),
    m_wedgeNBin(5)
{
  
  declareProperty("TRT_DC_ContainerName", m_trtDCContainerName = "Trig_OfflineDriftCircles" );
  declareProperty("TrtDataProviderTool",  m_rawDataTool, "TrigTRT_DriftCircleProviderTool");
  declareProperty("EtaHalfWidth",         m_etaHalfWidth); //Used to define subsection of RoI (to retrieve fewer TRT hits)
  declareProperty("PhiHalfWidth",         m_phiHalfWidth); //Used to define subsection of RoI (to retrieve fewer TRT hits)
  declareProperty("doFullScan",           m_doFullScan); //Whether to use all RoI (not implmented)
  declareProperty("RoadWidth",            m_roadWidth); //Width of road in mm
  declareProperty("nBinCoarse",           m_nBinCoarse); //Number of coarse bins used while phi centering
  declareProperty("nBinFine",             m_nBinFine); //Number of fine bins used while phi centering
  declareProperty("WedgeMinEta",          m_wedgeMinEta); //Min eta for wedge algorithm
  declareProperty("RoadMaxEta",           m_roadMaxEta); //Max eta for road algorithm (barrel only)
  declareProperty("WedgeNBin",            m_wedgeNBin); //Number of fine bins to consider in the wedge algorithm

}

//---------------------------------------------------------------------------------

TrigTRTHTHCounter::~TrigTRTHTHCounter() {
}

//---------------------------------------------------------------------------------

HLT::ErrorCode TrigTRTHTHCounter::hltInitialize() {
  ATH_MSG_DEBUG ( "Initialising this TrigTRTHTHCounter: " << name());
  
  // Get storegate svc
  if(m_detStore.retrieve().isFailure()) {
    ATH_MSG_FATAL ( "Failed to connect to " << m_detStore.typeAndName());
    return StatusCode::FAILURE;
  } else
    ATH_MSG_INFO ( "Retrieved service " << m_detStore.typeAndName());
  
  if(m_storeGate.retrieve().isFailure()) {
    ATH_MSG_FATAL ( "Failed to connect to " << m_storeGate.typeAndName());
    return StatusCode::FAILURE;
  } else
    ATH_MSG_INFO ( "Retrieved service " << m_storeGate.typeAndName());

  // Get a TRT identifier helper
  if( m_detStore->retrieve(m_trtHelper, "TRT_ID").isFailure()) {
    ATH_MSG_ERROR ( "Failed to retrieve " << m_trtHelper); // fatal?
    return StatusCode::FAILURE;
  } else
    ATH_MSG_INFO ( "Retrieved service " << m_trtHelper);
  

  // Get TrigTRT_DriftCircleProviderTool
  if( m_rawDataTool.retrieve().isFailure() ){
    ATH_MSG_FATAL ( "Failed to retrieve " << m_rawDataTool);
    return StatusCode::FAILURE;
  } else
    ATH_MSG_INFO ( "Retrieved service " << m_rawDataTool);

  ATH_MSG_INFO ( " TrigTRTHTHCounter initialized successfully");

  return HLT::OK;  
}

//---------------------------------------------------------------------------------------------------------------------------------------------
HLT::ErrorCode TrigTRTHTHCounter::hltBeginRun() {
  ATH_MSG_DEBUG ( "beginning run in this " << name());

  ATH_MSG_INFO ( "Pixel_TrgClusterization::hltBeginRun() ");
  if (!m_doFullScan){
    ATH_MSG_INFO ( "PhiHalfWidth: " << m_phiHalfWidth << " EtaHalfWidth: "<< m_etaHalfWidth);
  } else {
    ATH_MSG_INFO ( "FullScan mode");
  }

  return HLT::OK;
}
//---------------------------------------------------------------------------------------------------------------------------------------------

HLT::ErrorCode TrigTRTHTHCounter::hltExecute(const HLT::TriggerElement* inputTE,
					     HLT::TriggerElement* outputTE) {

  ATH_MSG_DEBUG ( "Executing TrigTRTHTHCounter " << name());

  m_listOfTrtIds.clear();

  float hitInit[5]={0,-999,0,-999,-999};
  m_trththits.clear();
  for (int i=0; i<5; i++) {
    m_trththits.push_back(hitInit[i]);
  }

  //Vectors to hold the count of total and HT TRT hits in the coarse bins 
  std::vector<int> count_httrt_c(m_nBinCoarse);
  std::vector<int> count_tottrt_c(m_nBinCoarse);
  
  //Vectors to hold the count of total and HT TRT hits in the fine bins
  std::vector<int> count_httrt(3*m_nBinFine);
  std::vector<int> count_tottrt(3*m_nBinFine);

  //Vector to hold TRT hits that are within RoI
  std::vector<TRT_hit> hit;

  const TrigRoiDescriptor* roi = 0;
  HLT::ErrorCode stat = getFeature( inputTE, roi, "initialRoI");
  if (stat!=HLT::OK || roi==0){
    return HLT::NAV_ERROR;
  }

  float coarseWedgeHalfWidth = m_phiHalfWidth/m_nBinCoarse;
  float fineWedgeHalfWidth = coarseWedgeHalfWidth/m_nBinFine;

  // Adding xAOD information 
  ATH_MSG_VERBOSE ( "Attempting to get xAOD::RNNOutput");

  xAOD::TrigRNNOutput *rnnOutput = new xAOD::TrigRNNOutput();
  ATH_MSG_VERBOSE ( "Successfully got xAOD::RNNOutput ");
  rnnOutput->makePrivateStore();

  ATH_MSG_VERBOSE ( "Got makePrivateStore " << name());

  //Code will only proceed if the RoI eta is not too large; used to limit rate from endcap
  if ( fabs(roi->eta())<=m_maxCaloEta ){

    float phi=roi->phi();
    float phimin=roi->phi()-m_phiHalfWidth;
    float phimax=roi->phi()+m_phiHalfWidth;
    if (phimin<-PI) phimin+=2.*PI; if (phimin>PI) phimin-=2.*PI;
    if (phimax<-PI) phimax+=2.*PI; if (phimax>PI) phimax-=2.*PI;

    //Define a smaller RoI for which TRT hits will be retrieved 
    TrigRoiDescriptor tmproi(roi->eta(), roi->eta()-m_etaHalfWidth, roi->eta()+m_etaHalfWidth,
			     phi, phimin, phimax);

    StatusCode sc_fc = m_rawDataTool->fillCollections(tmproi);

    if (sc_fc.isFailure()){
      ATH_MSG_WARNING ( "Failed to prepare TRTDriftCircle collection");
    }

    const InDet::TRT_DriftCircleContainer* driftCircleContainer = nullptr;
    StatusCode sc_sg = evtStore()->retrieve( driftCircleContainer, m_trtDCContainerName) ;
    if( sc_sg.isFailure() ){
      ATH_MSG_ERROR ( " Failed to retrieve trt data from SG. "); 
      return HLT::TOOL_FAILURE;
    }
    else {
      ATH_MSG_VERBOSE ( " Successfully retrieved trt data from SG. "); 
    }    

    InDet::TRT_DriftCircleContainer::const_iterator trtdriftContainerItr  = driftCircleContainer->begin();
    InDet::TRT_DriftCircleContainer::const_iterator trtdriftContainerItrE = driftCircleContainer->end();

    for (; trtdriftContainerItr != trtdriftContainerItrE; ++trtdriftContainerItr) {
    
      InDet::TRT_DriftCircleCollection::const_iterator trtItr = (*trtdriftContainerItr)->begin();
      InDet::TRT_DriftCircleCollection::const_iterator trtEnd = (*trtdriftContainerItr)->end();
      
      for(; trtItr!=trtEnd; trtItr++){
	
        // find out which detector element the hit belongs to
        const InDetDD::TRT_BaseElement *det = (*trtItr)->detectorElement();
        Identifier ID = (*trtItr)->identify();
        const Amg::Vector3D& strawcenter = det->strawCenter(m_trtHelper->straw(ID));

        bool hth = false;
	float hphi = strawcenter.phi();
	float heta = strawcenter.eta();
	float R = strawcenter.perp(); 

        if ((*trtItr)->highLevel()) hth = true;
	
	//if the hit is close in eta to the RoI, it needs to be stored
        if(hth_eta_match(roi->eta(), heta, m_etaHalfWidth)) hit.push_back(make_hit(hphi,R,hth));

	//First, define coarse wedges in phi, and count the TRT hits in these wedges
        int countbin=0;	
        if(hth_delta_phi(hphi, roi->phi()) < 0.1){
	  float startValue = roi->phi() - m_phiHalfWidth + coarseWedgeHalfWidth;
	  float endValue = roi->phi() + m_phiHalfWidth;
	  float increment = 2*coarseWedgeHalfWidth;
          for(float roibincenter = startValue; roibincenter < endValue; roibincenter += increment){
            if (hth_delta_phi(hphi,roibincenter)<=coarseWedgeHalfWidth && hth_eta_match(roi->eta(), heta, m_etaHalfWidth)) {
	      if(hth) count_httrt_c.at(countbin) += 1.;
	      count_tottrt_c.at(countbin) += 1.;
	      break; //the hit has been assigned to one of the coarse wedges, so no need to continue the for loop							
	    }
	    countbin++;
	  }
	}
        ATH_MSG_VERBOSE ( "timeOverThreshold=" << (*trtItr)->timeOverThreshold()
			<< "  highLevel=" << (*trtItr)->highLevel()
			<< " rawDriftTime=" << (*trtItr)->rawDriftTime()
			<< " barrel_ec=" << m_trtHelper->barrel_ec(ID)
			<< " phi_module=" << m_trtHelper->phi_module(ID)
			<< " layer_or_wheel=" << m_trtHelper->layer_or_wheel(ID)
			<< " straw_layer=" << m_trtHelper->straw_layer(ID)
			<< " straw=" << m_trtHelper->straw(ID)
			<< " scR=" << det->strawCenter(m_trtHelper->straw(ID)).perp()
			<< " scPhi=" << hphi
			<< " scEta=" << heta);		    
      } // end of loop over TRT drift circle collection
    } //end of loop over TRT drift circle container
  } //end of check to see if RoI eta is not too large

  //Now figure out which of the coarse bins in phi has the max number of HT TRT hits
  int maxHits = 0; //used to keep track of the max number of HT TRT hits in a coarse bin
  int dist = 0; //used to keep track of which coarse bin has the max number of HT TRT hits

  for (size_t iw = 0; iw < count_httrt_c.size(); iw++){
    if(maxHits <= count_httrt_c[iw]){ 
      maxHits = count_httrt_c[iw]; 
      dist = iw;  
    }
  }

  //Value of dist can range between 0 and (m_nBinCoarse-1)
  float center_pos_phi=roi->phi()+(2*dist+1-m_nBinCoarse)*coarseWedgeHalfWidth;

  //Now, define fine wedges in phi, centered around the best coarse wedge, and count the TRT hits in these fine wedges
  for(size_t v=0;v<hit.size();v++){
    int countbin=0;	
    if(hth_delta_phi(hit[v].phi, center_pos_phi) < 0.01){
      float startValue = center_pos_phi - 3*coarseWedgeHalfWidth + fineWedgeHalfWidth;
      float endValue = center_pos_phi + 3*coarseWedgeHalfWidth;
      float increment = 2*fineWedgeHalfWidth;
      for(float roibincenter = startValue; roibincenter < endValue; roibincenter += increment){	
        if (hth_delta_phi(hit[v].phi,roibincenter)<=fineWedgeHalfWidth) {
          if(hit[v].isHT) count_httrt.at(countbin) += 1.;
          count_tottrt.at(countbin) += 1.;
          break; //the hit has been assigned to one of the fine wedges, so no need to continue the for loop							
	}
	countbin++;
      }
    }    
  }
  
  //Now figure out which of the fine bins in phi has the max number of HT TRT hits
  maxHits = 0; //used to keep track of the max number of HT TRT hits in a fine bin
  dist = 0; //used to keep track of which fine bin has the max number of HT TRT hits

  for (size_t iw = 0; iw < count_httrt.size(); iw++){
    if(maxHits <= count_httrt[iw]){ 
      maxHits = count_httrt[iw]; 
      dist = iw; 
    }
  }

  //Value of dist can range between 0 and (3*m_nBinFine-1)
  center_pos_phi+=(2*dist+1-3*m_nBinFine)*fineWedgeHalfWidth;

  //Count the number of total and HT TRT hits for the road algorithm
  int trthit=0, trthit_ht=0;
  for(size_t v=0;v<hit.size();v++){
    if (dist2COR(hit[v].R,hit[v].phi,center_pos_phi)<=m_roadWidth){
      if(hit[v].isHT) trthit_ht+=1; 
      trthit+=1;						
    }
  }

  if (trthit!=0&&(fabs(roi->eta())<=m_roadMaxEta)){
    m_trththits[0] = trthit_ht;
    m_trththits[1] = (float)trthit_ht/trthit;
  }

  //Count the number of total and HT TRT hits for the wedge algorithm
  trthit = 0;
  trthit_ht = 0;

  for (int k=0;k<(2*m_wedgeNBin+1);k++){
    int binNumber = dist+k-m_wedgeNBin;
    //Even though we are supposed to use 2*m_wedgeNBin+1 fine bins, 
    //need to make sure that binNumber is sensible
    if(binNumber >= 0 && binNumber < (int)count_httrt.size()){
      trthit += count_tottrt.at(binNumber);
      trthit_ht += count_httrt.at(binNumber);
    }
  }

  if (trthit!=0&&(fabs(roi->eta())>=m_wedgeMinEta)){
    m_trththits[2] = trthit_ht;
    m_trththits[3] = (float)trthit_ht/trthit;
  }

  m_trththits[4]=roi->eta();

  ATH_MSG_VERBOSE ( "trthits with road algorithm : " << m_trththits[0]);
  ATH_MSG_VERBOSE ( "fHT with road algorithm : " << m_trththits[1]);
  ATH_MSG_VERBOSE ( "trthits with wedge algorithm : " << m_trththits[2]);
  ATH_MSG_VERBOSE ( "fHT with wedge algorithm : " << m_trththits[3]);
  ATH_MSG_VERBOSE ( "ROI eta : " << m_trththits[4]);
  
  //Writing to xAOD
  rnnOutput->setDecision(m_trththits);

  std::string key="";
  std::string label="TrigTRTHTCounts";

  //Write and attach for xAOD
  HLT::ErrorCode hitStatus = recordAndAttachFeature<xAOD::TrigRNNOutput>(outputTE, rnnOutput, key, label) ;

  if (hitStatus != HLT::OK){
    ATH_MSG_ERROR ( "Writing to xAODs failed");
    return HLT::NAV_ERROR;
  }
  return HLT::OK;
}

//---------------------------------------------------------------------------------

HLT::ErrorCode TrigTRTHTHCounter::hltFinalize() {
  ATH_MSG_DEBUG ( " finalizing TrigTRTHTHCounter : "<< name()); 
  return HLT::OK;  
}


