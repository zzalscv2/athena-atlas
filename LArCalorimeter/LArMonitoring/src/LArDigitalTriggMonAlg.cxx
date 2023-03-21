/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


// ********************************************************************
//
// NAME:     LArDigitalTriggMonAlg.cxx
// PACKAGE:  LArMonitoring
//
// AUTHOR:   Pavol Strizenec (pavol@mail.cern.ch)
//           Ljiljana Morvaj (ljiljana.morvaj@cern.ch)
//           Yesenia Hernadez (yesenia@cern.ch)
//           Based on LAtDigitMon tool by L. Hellary and LArOddCellsMonTool.cxx  by Benjamin Trocme
// 
// Monitor a few things in the LArDigit...
//
//	1) Check that the highest value of the LArDigit is contained in an interval. 
//         If it is not the case increment 3 histograms for each subdetector:
//	      a) Out of range histograms
//	      b) The average histograms: give the average value of the highest digit sample 
//	      c) Channel VS FEB histograms: gives wich slot on wich FEB has his highest digit sample ou of the range
//	2) Check if a digits samples are in saturation. If it's the case increment the saturation histograms. 
//
// Available cuts in the jo file:
//
//   a) SampleRangeLow-SampleRangeUp: range to check the digit sample.
//   b) ADCcut : To select Digits Samples with signal.
//   c) ADCsature: lowest value to check if a Digit sample is in saturation.
// ********************************************************************

#include "LArDigitalTriggMonAlg.h"

//Histograms
//LAr infos:
#include "Identifier/HWIdentifier.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArRawEvent/LArDigit.h"

#include "LArRawEvent/LArSCDigit.h"
#include "LArRawEvent/LArLATOMEHeaderContainer.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"
#include "LArTrigStreamMatching.h"
#include "StoreGate/ReadDecorHandle.h"

#include "LArCOOLConditions/LArPedestalSC.h" 

//STL:
#include <sstream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <algorithm>
#include <set>
#include <fstream>

/*---------------------------------------------------------*/
LArDigitalTriggMonAlg::LArDigitalTriggMonAlg(const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name, pSvcLocator),
    m_LArOnlineIDHelper(0),
    m_LArEM_IDHelper(0),
    m_SCID_helper(0)
{	

}

/*---------------------------------------------------------*/
LArDigitalTriggMonAlg::~LArDigitalTriggMonAlg()
{
}

/*---------------------------------------------------------*/
StatusCode 
LArDigitalTriggMonAlg::initialize()
{
  
  ATH_MSG_INFO( "Initialize LArDigitalTriggMonAlg" );
  
  /** Get LAr Online Id Helper*/
  if ( detStore()->retrieve( m_LArOnlineIDHelper, "LArOnlineID" ).isSuccess() ) {
    ATH_MSG_DEBUG("connected non-tool: LArOnlineID" );    
  } else {
    return StatusCode::FAILURE;    
  }
  
  // Get LArEM Id Helper, not used now...
  if ( detStore()->retrieve( m_LArEM_IDHelper, "LArEM_ID" ).isSuccess() ) {
    ATH_MSG_DEBUG("connected non-tool: LArEM_ID" );
  } else {
    ATH_MSG_FATAL( "unable to connect non-tool: LArEM_ID" );
    return StatusCode::FAILURE;
  }

  /** Get LAr Online SC Id Helper*/
  if ( detStore()->retrieve( m_SCID_helper, "CaloCell_SuperCell_ID" ).isSuccess() ) {
    ATH_MSG_DEBUG("connected non-tool: CaloCell_SuperCell_ID" );    
  } else {
    return StatusCode::FAILURE;    
  }

  std::ifstream inFile("/detwork/lar/data/bad_scs.txt");

  std::string bad_sc; 
  while(std::getline(inFile, bad_sc)){
      m_badSCs.insert(bad_sc);
      
  }
  
  
  ATH_MSG_DEBUG("ADC NAME: " << m_AdcName);
  ATH_MSG_DEBUG("ET NAME: " << m_EtName);
  ATH_CHECK(m_digitContainerKey.initialize());
  ATH_CHECK(m_keyPedestalSC.initialize());
  ATH_CHECK(m_caloSuperCellMgrKey.initialize());
  ATH_CHECK(m_rawSCContainerKey.initialize());  
  ATH_CHECK(m_rawSCEtRecoContainerKey.initialize());  
  ATH_CHECK(m_eventInfoKey.initialize());
  ATH_CHECK(m_cablingKey.initialize());
  ATH_CHECK(m_actualMuKey.initialize());

  return AthMonitorAlgorithm::initialize();
}

StatusCode LArDigitalTriggMonAlg::fillHistograms(const EventContext& ctx) const
{
  
  ATH_MSG_DEBUG("in fillHists()" );

  //monitored variables
  auto lumi_block = Monitored::Scalar<unsigned int>("LB", 0);

  auto MNsamples = Monitored::Scalar<int>("MNsamples",-1);
  auto MSCChannel = Monitored::Scalar<int>("MSCChannel",-1);
  auto MlatomeSourceId = Monitored::Scalar<int>("MlatomeSourceId",-1);
  auto Mmaxpos = Monitored::Scalar<int>("Mmaxpos",-1);
  auto Mpartition = Monitored::Scalar<int>("Mpartition",-1);
  auto Msampos = Monitored::Scalar<int>("Msampos",-1);
  auto MADC = Monitored::Scalar<int>("MADC",-1);
  auto MADC_0 = Monitored::Scalar<int>("MADC_0",-1);
  auto MSCeT = Monitored::Scalar<float>("MSCeT",0.0);
  auto MSCeT_Nonzero = Monitored::Scalar<float>("MSCeT_Nonzero",0.0);

  auto MSCphi = Monitored::Scalar<float>("MSCphi",0.0);
  auto MSCeta = Monitored::Scalar<float>("MSCeta",0.0);
  auto MSCsatur = Monitored::Scalar<float>("MSCsatur",0.0);
  auto BCID = Monitored::Scalar<int>("BCID",0);
  auto Menergy_onl = Monitored::Scalar<int>("Menergy_onl",0.0);
  auto Monl_energy_tauSelFail = Monitored::Scalar<int>("Monl_energy_tauSelFail",0.0);
  auto Menergy_ofl = Monitored::Scalar<int>("Menergy_ofl",0.0);
  auto MSCEt_diff = Monitored::Scalar<int>("MSCEt_diff",0.0);
  auto MSCtime = Monitored::Scalar<float>("MSCtime",0.0);
  auto MSCtimeNoZero = Monitored::Scalar<float>("MSCtimeNoZero",0.0);
  auto MlatomeSourceIdBIN = Monitored::Scalar<int>("MlatomeSourceIdBIN",1);

  auto MlatomeSourceIdBIN_subdet = Monitored::Scalar<int>("MlatomeSourceIdBIN_subdet",1);

  auto Pedestal = Monitored::Scalar<float>("Pedestal",0.0);
  auto PedestalRMS = Monitored::Scalar<float>("PedestalRMS",0.0);

  /**EventID is a part of EventInfo, search event informations:*/
  SG::ReadHandle<xAOD::EventInfo> thisEvent{m_eventInfoKey,ctx};

  const std::vector<unsigned> streamsThisEvent=LArMon::trigStreamMatching(m_streams,thisEvent->streamTags());
  
  SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey,ctx};
  const LArOnOffIdMapping* cabling=*cablingHdl;

  SG::ReadHandle<LArDigitContainer> hLArDigitContainer{m_digitContainerKey,ctx}; //"SC"
  SG::ReadHandle<LArRawSCContainer > hSCetContainer{m_rawSCContainerKey,ctx}; //"SC_ET"
  SG::ReadHandle<LArRawSCContainer > hSCetRecoContainer{m_rawSCEtRecoContainerKey,ctx}; //"SC_ET_RECO"

  ATH_MSG_INFO("hLArDigitContainer.isValid() " << hLArDigitContainer.isValid());
  if (hLArDigitContainer.isValid()) ATH_MSG_INFO("hLArDigitContainer.size() " << hLArDigitContainer->size());
  ATH_MSG_INFO("hSCetContainer.isValid() " << hSCetContainer.isValid());
  if (hSCetContainer.isValid()) ATH_MSG_INFO("hSCetContainer.size() " << hSCetContainer->size());
  ATH_MSG_INFO("hSCetRecoContainer.isValid() " << hSCetRecoContainer.isValid());
  if(hSCetRecoContainer.isValid()) ATH_MSG_INFO("hSCetRecoContainer.size() " << hSCetRecoContainer->size());

  BCID = thisEvent->bcid();// - 88)%((36+7)*4 + 36 + 31);
  lumi_block = thisEvent->lumiBlock();

  float mu = lbInteractionsPerCrossing(ctx);
  float event_mu = lbLuminosityPerBCID(ctx);

  ATH_MSG_INFO("ADC NAME: "<<m_AdcName);
  ATH_MSG_INFO("ET NAME: "<<m_EtName);
  ATH_MSG_INFO("mu (LB): "<<mu);
  ATH_MSG_INFO("mu (BCID): "<<event_mu);
  ATH_MSG_INFO("LB number: "<<thisEvent->lumiBlock());
  SG::ReadCondHandle<ILArPedestal>    pedestalHdl{m_keyPedestalSC, ctx};
  const LArPedestalSC* pedestals=dynamic_cast<const LArPedestalSC*>(pedestalHdl.cptr());

   SG::ReadCondHandle<CaloSuperCellDetDescrManager> caloSuperCellMgrHandle{m_caloSuperCellMgrKey,ctx};
  const CaloSuperCellDetDescrManager* ddman = *caloSuperCellMgrHandle;

  if ( (hLArDigitContainer.isValid()) ){
 
    /** Define iterators to loop over Digits containers*/
    LArDigitContainer::const_iterator itDig = hLArDigitContainer->begin(); 
    LArDigitContainer::const_iterator itDig_e= hLArDigitContainer->end(); 

    const LArDigit* pLArDigit;

    std::string layer;

    /** Loop over digits*/
    for ( ; itDig!=itDig_e;++itDig) {
      pLArDigit = *itDig;
      unsigned int trueNSamples = pLArDigit->nsamples();
      MNsamples = trueNSamples;

      fill(m_scMonGroupName, MNsamples);
      
      HWIdentifier id = pLArDigit->hardwareID(); //gives online ID
      // will be used in next iteration
      //int channelHWID = m_LArOnlineIDHelper->channel(id);

      //From https://acode-browser2.usatlas.bnl.gov/lxr/source/r22/athena/LArCalorimeter/LArMonitoring/src/LArDigitMonAlg.cxx
      int cgain = pLArDigit->gain();

      if(pedestals){
	Pedestal = pedestals->pedestal(id,cgain);
	PedestalRMS = pedestals->pedestalRMS(id,cgain);
      }
      else
	ATH_MSG_INFO( "Pedestal values not received");

      /**skip cells with no pedestals reference in db.*/
      //if(Pedestal <= (1.0+LArElecCalibMCSC::ERRORCODE)) continue;
   
      /**skip disconnected channels:*/
      if(!cabling->isOnlineConnected(id)) continue;   

      const Identifier offlineID=cabling->cnvToIdentifier(id);

      const CaloDetDescrElement* caloDetElement = ddman->get_element(offlineID);
      
      if(caloDetElement == 0 ){
	ATH_MSG_ERROR( "Cannot retrieve caloDetElement" );
        continue;
      } 
      
      float etaSC = caloDetElement->eta_raw();
      float phiSC = caloDetElement->phi_raw();
      
      int calosample=caloDetElement->getSampling();
      unsigned iLyrNS=m_caloSamplingToLyrNS.at(calosample);

      const unsigned whichSide=(etaSC>0) ? 0 : 1; //Value >0 means A-side 
      unsigned iLyr=iLyrNS*2+whichSide;
      auto layerName=m_layerNames[iLyr];
      
      /** Determine to which partition this channel belongs to*/
      int side = m_LArOnlineIDHelper->pos_neg(id);
      const int ThisPartition=WhatPartition(id,side);
      //    std::string spart = m_partitions[ThisPartition];
      Mpartition = ThisPartition;
      
      const LArSCDigit* scdigi = dynamic_cast<const LArSCDigit*>(pLArDigit);
      if(!scdigi){ ATH_MSG_DEBUG(" CAN'T CAST ");
      }else{
	MSCChannel = scdigi->Channel();
	fill(m_scMonGroupName, MSCChannel);
		
	MlatomeSourceId = scdigi->SourceId();
	MlatomeSourceIdBIN=getXbinFromSourceID(MlatomeSourceId);

	fill(m_scMonGroupName, MlatomeSourceId);

      }
      
      /** Retrieve samples*/
      const std::vector<short>* digito = &pLArDigit->samples();
      
      /**retrieve the max sample digit ie digitot.back().*/
      std::vector<short>::const_iterator maxSam = std::max_element(digito->begin(), digito->end());
      int thismaxPos=(maxSam-digito->begin());
      Mmaxpos=thismaxPos+1; //count samples [1,5]

      /** max sample per layer*/
      if(layerName.find('P')!= std::string::npos || layerName.find('0')!= std::string::npos){  
        layer = "0";
      }
      else if(layerName.find('1')!= std::string::npos){ 
	layer = "1";
      }
      else if(layerName.find('2')!= std::string::npos){ 
	layer = "2";
      }
      else{  
	layer = "3";
      }


      auto Mmaxpos_layer = Monitored::Scalar<float>("Mmaxpos_"+layer,Mmaxpos);
      auto eta_digi = Monitored::Scalar<float>("eta_digi_"+layerName,etaSC);
      auto phi_digi = Monitored::Scalar<float>("phi_digi_"+layerName,phiSC);
      auto eta_rms_all  = Monitored::Scalar<float>("eta_rms",etaSC);
      auto phi_rms_all  = Monitored::Scalar<float>("phi_rms",phiSC);
      auto eta_rms  = Monitored::Scalar<float>("eta_rms_"+layerName,etaSC);
      auto phi_rms  = Monitored::Scalar<float>("phi_rms_"+layerName,phiSC);
      auto eta_rmsFromDB_all  = Monitored::Scalar<float>("eta_rms_fromDB",etaSC);
      auto phi_rmsFromDB_all  = Monitored::Scalar<float>("phi_rms_fromDB",phiSC);
      auto eta_rmsFromDB  = Monitored::Scalar<float>("eta_rms_fromDB_"+layerName,etaSC);
      auto phi_rmsFromDB  = Monitored::Scalar<float>("phi_rms_fromDB_"+layerName,phiSC);
      auto badbit_eta_digi = Monitored::Scalar<float>("badQualBit_eta_"+layerName,etaSC);
      auto badbit_phi_digi = Monitored::Scalar<float>("badQualBit_phi_"+layerName,phiSC);


      auto badbit_eta = Monitored::Scalar<float>("badQualBit_eta",etaSC);
      auto badbit_phi = Monitored::Scalar<float>("badQualBit_phi",phiSC);
      auto saturation_eta = Monitored::Scalar<float>("Saturation_eta",etaSC);
      auto saturation_phi = Monitored::Scalar<float>("Saturation_phi",phiSC);


      bool isBadQualityBit=false;

      //samples
      for(unsigned i=0; i<trueNSamples;++i) {
	Msampos=i+1;
	MADC=pLArDigit->samples().at(i);

	if (m_AdcName.size() >= 3){ //SC_ADC_BAS, have to divide by 8
	  MADC=MADC/8;
	}

	if(i==0){
          MADC_0=pLArDigit->samples().at(i);
	  if (m_AdcName.size() >= 3) MADC_0=MADC_0/8;
	}

  	if( MADC < 100 ) ATH_MSG_DEBUG("Low ADC for Layer "<<layerName<<" "<<MADC<<", BCID: "<<BCID);
	auto ADC_xLayer = Monitored::Scalar<int>("ADC_"+layerName,MADC);
	fill(m_scMonGroupName, BCID, ADC_xLayer);

	fill(m_scMonGroupName, Msampos, MADC, MlatomeSourceIdBIN);
	  
	fill(m_scMonGroupName, MlatomeSourceIdBIN, Pedestal);

	fill(m_scMonGroupName, Msampos, Pedestal, MlatomeSourceIdBIN);

	if(MADC==-1){
	  isBadQualityBit=true;

	}
	else {
	  float Diff_ADC_Pedestal = -999; 
	  float ADC_max = pLArDigit->samples().at(Mmaxpos-1);

	  if (pLArDigit->samples().at(Mmaxpos-1) != Pedestal) 
	    Diff_ADC_Pedestal = (MADC - Pedestal) / std::abs(ADC_max  - Pedestal);                                                               
                          
	  if(std::abs(ADC_max  - Pedestal) > 10*PedestalRMS) { 
	    fill(m_scMonGroupName, Mmaxpos, Mpartition, MlatomeSourceIdBIN);  

	    fill(m_scMonGroupName, Mmaxpos_layer, Mmaxpos);

	    if(Diff_ADC_Pedestal < -0.99){ 
	      ATH_MSG_DEBUG( "ADC - Pedestal / std::abs(ADC_max - Pedestal) = "<< Diff_ADC_Pedestal);
	      ATH_MSG_DEBUG(" RMS = "<< PedestalRMS);
	      ATH_MSG_DEBUG(" ADC = "<< MADC );
	      ATH_MSG_DEBUG(" pedestal = "<< Pedestal );
	      ATH_MSG_DEBUG(" ADC_max = " << ADC_max );
	      ATH_MSG_DEBUG(" ADC - pedestal = "<< MADC - Pedestal);
	      ATH_MSG_DEBUG(" ADC_max - pedestal = "<< ADC_max - Pedestal );
	    }
            auto ADC_Pedesdal_Norm = Monitored::Scalar<float>("Diff_ADC_Pedesdal_Norm",Diff_ADC_Pedestal);
	    fill(m_scMonGroupName, Msampos,ADC_Pedesdal_Norm);
	    auto Diff_ADC_Pedesdal_Norm_xLayer = Monitored::Scalar<float>("Diff_ADC_Pedesdal_Norm_"+layerName,Diff_ADC_Pedestal);
	    fill(m_scMonGroupName, Msampos, Diff_ADC_Pedesdal_Norm_xLayer);
	  }

          auto ADC_Pedesdal = Monitored::Scalar<float>("Diff_ADC_Pedesdal", MADC - Pedestal);
          fill(m_scMonGroupName, ADC_Pedesdal);
	}

      }


      /** bad quality bit coverage plot*/
      if(isBadQualityBit) {
	fill(m_scMonGroupName, badbit_eta, badbit_phi);
	fill(m_scMonGroupName, badbit_eta_digi, badbit_phi_digi);
      }

      /** max sample vs latome per layer*/

      /** max sample position: 1D --> Mean around 3 and RMS < 1*/
      // FIXME Need to add a cut: ADC_max - pedestal > 5*RMS(DB) 
      //}

      /** max sample vs latome per layer if RMS > 5 and RMS > 3*RMS(DB)*/

      if(MADC - Pedestal > 10*PedestalRMS) { 
	auto Mmaxpos_layer_cutfromDB = Monitored::Scalar<float>("Mmaxpos_"+layer+"_cutRMSfromDB",Mmaxpos);
	
	fill(m_scMonGroupName, eta_rmsFromDB_all, phi_rmsFromDB_all);
      }

    }/** End of loop on LArDigit*/
    
  } // End if(LArDigitContainer is valid)

  if ( hSCetContainer.isValid() ){

    LArRawSCContainer::const_iterator itSC = hSCetContainer->begin();
    LArRawSCContainer::const_iterator itSCReco = hSCetRecoContainer->begin();
    const LArRawSC* rawSC = 0;
    const LArRawSC* rawSCReco = 0;



    std::map<std::string, float> AvEnergyPerLayer= { {"EMBPA",0}, {"EMBPC",0}, {"EMB1A",0}, {"EMB1C",0}, {"EMB2A",0}, {"EMB2C",0}, {"EMB3A",0}, {"EMB3C",0},
                                                     {"HEC0A",0}, {"HEC0C",0}, {"HEC1A",0}, {"HEC1C",0}, {"HEC2A",0}, {"HEC2C",0}, {"HEC3A",0}, {"HEC3C",0},
                                                     {"EMECPA",0}, {"EMECPC",0}, {"EMEC1A",0}, {"EMEC1C",0}, {"EMEC2A",0}, {"EMEC2C",0}, {"EMEC3A",0}, {"EMEC3C",0},
                                                     {"FCAL1A",0}, {"FCAL1C",0}, {"FCAL2A",0}, {"FCAL2C",0}, {"FCAL3A",0}, {"FCAL3C",0}};
    std::map<std::string, float> SCsPerLayer= { {"EMBPA",0}, {"EMBPC",0}, {"EMB1A",0}, {"EMB1C",0}, {"EMB2A",0}, {"EMB2C",0}, {"EMB3A",0}, {"EMB3C",0},
						{"HEC0A",0}, {"HEC0C",0}, {"HEC1A",0}, {"HEC1C",0}, {"HEC2A",0}, {"HEC2C",0}, {"HEC3A",0}, {"HEC3C",0},
						{"EMECPA",0}, {"EMECPC",0}, {"EMEC1A",0}, {"EMEC1C",0}, {"EMEC2A",0}, {"EMEC2C",0}, {"EMEC3A",0}, {"EMEC3C",0},
						{"FCAL1A",0}, {"FCAL1C",0}, {"FCAL2A",0}, {"FCAL2C",0}, {"FCAL3A",0}, {"FCAL3C",0}};
        
    
    /** Loop over SCs*/
    while(itSC != hSCetContainer->end() ){
      rawSC = *itSC;
      rawSCReco = *itSCReco;
      MSCChannel = rawSC->chan();
      int bcid_ind = 0;
      if (rawSC->energies().size()>0){
	for ( auto & SCe : rawSC->bcids() ) 
	  {
	    if ( SCe == BCID ) break;
	    bcid_ind++;
	  }
      }

      if ( rawSC->bcids().at(bcid_ind) != BCID ) ATH_MSG_WARNING("BCID not found in SC bcids list!! "<<BCID<<" "<<rawSC->bcids().at(bcid_ind));

      Menergy_onl = rawSC->energies().at(bcid_ind);
      Menergy_ofl = rawSC->energies().at(0); // algorithm already selects the correct energy
      MSCEt_diff = Menergy_onl - Menergy_ofl;	
      if (rawSCReco->passTauSelection().at(0) == true){ //only compare Et if tau selection is passed
	fill(m_scMonGroupName, MSCEt_diff);
	fill(m_scMonGroupName, Menergy_onl,Menergy_ofl);
      }else{
	ATH_MSG_DEBUG("Filling tau selection histo with "<<Monl_energy_tauSelFail);
	fill(m_scMonGroupName, Monl_energy_tauSelFail);      
      }
      ATH_MSG_DEBUG("Energy onl - Energy ofl: "<<Menergy_onl<<",  "<<Menergy_ofl<<std::endl); 
      
      if (rawSC->energies().size()>0){
	int energy  = rawSC->energies().at(bcid_ind);
	
	MSCeT = energy;
        if (energy != 0 ){
            MSCeT_Nonzero = energy;
            fill(m_scMonGroupName, MSCeT_Nonzero);
        }

      }
      
      if (rawSC->satur().size()>0){
	bool saturated  = rawSC->satur().at(bcid_ind);
	
	MSCsatur = saturated;
      }
     
      /* 
      if (rawSC->satur().size()>0){
	MSCsatur = rawSC->satur().at(0);      
      }
      */
            
      if (MSCChannel<10){
	ATH_MSG_DEBUG("--- from  etcontainer MSCeT  = " << MSCeT);
	ATH_MSG_DEBUG("    |______ --------- MSCChannel = "<< MSCChannel);
	ATH_MSG_DEBUG("    |______ --------- MSCsatur = "<< MSCsatur);
	ATH_MSG_DEBUG("    |______ --------- MlatomeSourceIdBIN = "<< MlatomeSourceIdBIN);
	ATH_MSG_DEBUG("    |______ --------- rawSC->SourceId() = "<< rawSC->SourceId());
      }

      HWIdentifier id = rawSC->hardwareID(); //gives online ID

      //SC time from Offline computation.  require to pass tau selection exclude energies less than 5 GeV, exclude SCs from bad SC list
      int Etau = rawSCReco->tauEnergies().at(0);  

      //check if in list of bad SCs
      bool sc_is_bad = false;
      std::set<std::string>::iterator badSc_it = m_badSCs.find(std::to_string(id.get_identifier32().get_compact()));
      if(badSc_it != m_badSCs.end()){
          sc_is_bad = true;
      }
 
      if (rawSCReco->passTauSelection().at(0) == true){

          if(sc_is_bad){
              ATH_MSG_DEBUG("SC "<<id.get_identifier32().get_compact()<<" is in list of bad SCs! exclude it from time histo");
          }
          else{
              ATH_MSG_DEBUG("SC "<<id.get_identifier32().get_compact()<<" is not in list of bad SCs! adding to time histo");
		  if (Menergy_ofl > 400.0){
		      MSCtime = (float)Etau / (float)Menergy_ofl; 
		      if (Etau != 0){
			  MSCtimeNoZero = (float)Etau / (float)Menergy_ofl; 
			  fill(m_scMonGroupName, MSCtimeNoZero);
		      }
		      ATH_MSG_DEBUG("Offline time: "<<MSCtime<<std::endl); 
		      fill(m_scMonGroupName, MSCtime);
                      fill(m_scMonGroupName, lumi_block, MSCtime);
		  }
          } 
      }
      ////////////////// make coverage plots



      //      if(!cabling->isOnlineConnected(id)) continue;

      const Identifier offlineID=cabling->cnvToIdentifier(id); //converts online to offline ID
      Identifier32 Ofl32 =offlineID.get_identifier32();
      Identifier32 Onl32 =id.get_identifier32();
	
      // Get Physical Coordinates

      float etaSC = 0; float phiSC = 0.;
      const CaloDetDescrElement* caloDetElement = ddman->get_element(offlineID);
      if(caloDetElement == 0 ){
	
	ATH_MSG_ERROR( "Cannot retrieve (eta,phi) coordinates for raw channels" );
	ATH_MSG_ERROR( "  ==============> etaSC, phiSC: " << etaSC << " ," << phiSC << std::hex << ";  offlineID = "<< offlineID << "; Ofl32 compact= "<< Ofl32.get_compact()<< "; online ID =" << id << "; Onl32 = " << Onl32.get_compact()  << "; rawSC->SourceId() = " << rawSC->SourceId());
	continue; 
      }
      etaSC = caloDetElement->eta_raw();
      phiSC = caloDetElement->phi_raw();
      
      int calosample=caloDetElement->getSampling();
      unsigned iLyrNS=m_caloSamplingToLyrNS.at(calosample);
      
      const unsigned side=(etaSC>0) ? 0 : 1; //Value >0 means A-side                                                                                              
      unsigned iLyr=iLyrNS*2+side;


      
      MlatomeSourceIdBIN=getXbinFromSourceID(rawSC->SourceId());
      

      fill(m_scMonGroupName, MSCChannel, MSCeT, MSCsatur, MlatomeSourceIdBIN);


      MSCeta = etaSC; MSCphi = phiSC;
      
      auto MSCphi_Satur_all = Monitored::Scalar<float>("superCellPhi_Satur_all",phiSC);
      auto MSCeta_Satur_all = Monitored::Scalar<float>("superCellEta_Satur_all",etaSC);
      auto MSCphi_Satur = Monitored::Scalar<float>("superCellPhi_Satur",phiSC);
      auto MSCeta_Satur = Monitored::Scalar<float>("superCellEta_Satur",etaSC);
      auto MSCetaEcomp = Monitored::Scalar<float>("MSCetaEcomp",etaSC);
      auto MSCphiEcomp= Monitored::Scalar<float>("MSCphiEcomp",phiSC);

      auto OFCb_overflow_eta = Monitored::Scalar<float>("OFCb_overflow_eta",etaSC);
      auto OFCb_overflow_phi = Monitored::Scalar<float>("OFCb_overflow_phi",phiSC);
      auto OFCa_overflow_eta = Monitored::Scalar<float>("OFCa_overflow_eta",etaSC);
      auto OFCa_overflow_phi = Monitored::Scalar<float>("OFCa_overflow_phi",phiSC);
      
      auto OFCb_overflow_eta_all = Monitored::Scalar<float>("OFCb_overflow_eta_all",etaSC);
      auto OFCb_overflow_phi_all = Monitored::Scalar<float>("OFCb_overflow_phi_all",phiSC);
      auto OFCa_overflow_eta_all = Monitored::Scalar<float>("OFCa_overflow_eta_all",etaSC);
      auto OFCa_overflow_phi_all = Monitored::Scalar<float>("OFCa_overflow_phi_all",phiSC);


      /** OFCa and OFCb overflows*/
      if ( rawSCReco->ofcbOverflow() == true ){
	fill(m_scMonGroupName, OFCb_overflow_eta_all, OFCb_overflow_phi_all);
	if(! sc_is_bad) fill(m_scMonGroupName, OFCb_overflow_eta, OFCb_overflow_phi);
      }
      if ( rawSCReco->ofcaOverflow() == true ){
	fill(m_scMonGroupName, OFCa_overflow_eta_all, OFCa_overflow_phi_all);
	if(! sc_is_bad) fill(m_scMonGroupName, OFCa_overflow_eta, OFCa_overflow_phi);
      }

      if (MSCsatur != 0){
	fill(m_scMonGroupName, MSCeta_Satur_all, MSCphi_Satur_all);
	if(! sc_is_bad) fill(m_scMonGroupName, MSCeta_Satur, MSCphi_Satur);
      }
      if (rawSCReco->passTauSelection().at(0) == true){ //only compare Et if tau selection is passed
	if (Menergy_onl != Menergy_ofl){ //fill eta phi plot if the energies are different
	  fill(m_scMonGroupName, MSCetaEcomp, MSCphiEcomp);
	}
      }


      auto layerName=m_layerNames[iLyr];
      SCsPerLayer.at(layerName) += 1;
      if(! sc_is_bad){
	  if (MSCeT > -2000){
		ATH_MSG_DEBUG("Adding to total energy "<<layerName<<" : (total_SCeT  += energy/8.0) = ("<<MSCeT/8.0<<")");
	  	AvEnergyPerLayer.at(layerName) += MSCeT*12.5; // E --> E_t
	  }
      }
 
      auto tau_above_cut_eta = Monitored::Scalar<float>("tau_above_cut_eta_"+layerName,etaSC);
      auto tau_above_cut_phi = Monitored::Scalar<float>("tau_above_cut_phi_"+layerName,phiSC);
      /** abs(tau) > 2*/
      if ( std::abs(MSCtime) > 3 ){
	if(! sc_is_bad) fill(m_scMonGroupName, tau_above_cut_eta, tau_above_cut_phi);
      }



      auto LMSCphi = Monitored::Scalar<float>("superCellPhi_"+layerName,phiSC);
      auto LMSCeta = Monitored::Scalar<float>("superCellEta_"+layerName,etaSC);
      auto LMSCet = Monitored::Scalar<float>("superCellEt_"+layerName,MSCeT);

      //per-layer timing histos
      auto MSCtime_layer = Monitored::Scalar<float>("MSCtime_"+layerName,MSCtime);
      auto MSCtimeNoZero_layer = Monitored::Scalar<float>("MSCtimeNoZero_"+layerName,MSCtimeNoZero);
      if (rawSCReco->passTauSelection().at(0) == true){

          if(sc_is_bad){
              ATH_MSG_DEBUG("SC "<<id.get_identifier32().get_compact()<<" is in list of bad SCs! exclude it from time histo");
          }
          else{
              ATH_MSG_DEBUG("SC "<<id.get_identifier32().get_compact()<<" is not in list of bad SCs! adding to time histo");
		  if (Menergy_ofl > 400.0){
		      MSCtime = (float)Etau / (float)Menergy_ofl; 
		      if (Etau != 0){
			  MSCtimeNoZero = (float)Etau / (float)Menergy_ofl; 
			  fill(m_scMonGroupName, MSCtimeNoZero_layer);
		      }
		      ATH_MSG_DEBUG("Offline time: "<<MSCtime<<std::endl); 
		      fill(m_scMonGroupName, MSCtime_layer);
                      fill(m_scMonGroupName, lumi_block, MSCtime_layer);
		  }
          } 
      }


      fill(m_scMonGroupName, MSCeta, MSCphi);
      fill(m_scMonGroupName, LMSCeta, LMSCphi);
      fill(m_scMonGroupName, LMSCeta, LMSCphi, LMSCet);

      if (MSCeT>1000) { //make some conditions for filling
	auto LMSCphi_EtCut = Monitored::Scalar<float>("superCellPhi_EtCut_"+layerName,phiSC);
	auto LMSCeta_EtCut = Monitored::Scalar<float>("superCellEta_EtCut_"+layerName,etaSC);
	auto LMSCet_EtCut = Monitored::Scalar<float>("superCellEt_EtCut_"+layerName,MSCeT);

	if (! sc_is_bad ) fill(m_scMonGroupName, LMSCeta_EtCut, LMSCphi_EtCut);
      }

      if (MSCeT>10000) { //make some conditions for filling                                                                                                                
	auto LMSCphi_EtCut10 = Monitored::Scalar<float>("superCellPhi_EtCut10_"+layerName,phiSC);
	auto LMSCeta_EtCut10 = Monitored::Scalar<float>("superCellEta_EtCut10_"+layerName,etaSC);
	auto LMSCet_EtCut10 = Monitored::Scalar<float>("superCellEt_EtCut10_"+layerName,MSCeT);

	if (! sc_is_bad ) fill(m_scMonGroupName, LMSCeta_EtCut10, LMSCphi_EtCut10);
      }
      ++itSC;
      ++itSCReco;
    }//end loop over SCs
    for(auto layerName: m_layerNames){
      

      ATH_MSG_DEBUG("Filling BCID vs Av Energy"<<layerName<<" hist: (BCID,mu,AvEnergy,NSCs in Layer) = ("<<BCID<<", "<<event_mu<<", "<<AvEnergyPerLayer.at(layerName)<<", "<<SCsPerLayer.at(layerName)<<")");
      if (event_mu > 0){
          AvEnergyPerLayer.at(layerName) = AvEnergyPerLayer.at(layerName)/event_mu;
      }

      else  ATH_MSG_DEBUG("MU IS ZERO FOR THIS BCID!");

      auto LMAvEnergyOverMu = Monitored::Scalar<float>("AvEnergy_"+layerName,AvEnergyPerLayer.at(layerName)/SCsPerLayer.at(layerName));

      fill(m_scMonGroupName,BCID,LMAvEnergyOverMu); 
      AvEnergyPerLayer.at(layerName) = 0;
      SCsPerLayer.at(layerName) = 0;
       
    }
     
  }  // End if(LArSCContainer is valid)
  
  
  return StatusCode::SUCCESS;
}


/*---------------------------------------------------------*/
/** Say which partition is a channel*/

int LArDigitalTriggMonAlg::WhatPartition(HWIdentifier id, int side) const
{
  
  if (m_LArOnlineIDHelper->isEmBarrelOnline(id)) {
    if(side==0) return 0;
    else return 1;
  } else if (m_LArOnlineIDHelper->isEMECchannel(id)) {
    if(side==0) return 2;
    else return 3;
  } else if (m_LArOnlineIDHelper->isHECchannel(id)) {
    if(side==0) return 4;
    else return 5;
  } else {
    if(side==0) return 6;
    else return 7;
  }
}

  
  
int LArDigitalTriggMonAlg::getXbinFromSourceID(int sourceID) const
{
  //  int NLatomeBins = 117;
  int binx=m_NLatomeBins;
  std::string detName="";
  int detStartingBin=m_NLatomeBins;

  std::stringstream ss;
  ss<< std::hex << sourceID; // int decimal_value
  std::string sourceIDhex ( ss.str() );

  std::string detID=sourceIDhex.substr(0,2);
  //make sure we got the right string
  if (detID=="0x"){
    detID=sourceIDhex.substr(0,4);
  }
  for( auto& pair:m_LatomeDetBinMapping){
    if (pair.first.find(detID)!=std::string::npos ){
      detName=pair.second.first;
      detStartingBin=pair.second.second;
    }
  }

  std::stringstream ss2;
  std::string phiBin = sourceIDhex.substr(sourceIDhex.size()-1);
  ss2 << phiBin;
  int value;
  ss2 >> std::hex >> value; //value is in hex

  binx = detStartingBin + value;
  if (binx>m_NLatomeBins){
    ATH_MSG_WARNING("something wrong with binning, filling overflowbin");
    binx=m_NLatomeBins;
  }

  return binx;
}

