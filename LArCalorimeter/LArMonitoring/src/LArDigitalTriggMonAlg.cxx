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
    m_bcMask(1),
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
  ATH_MSG_INFO("Building tool map");
  m_toolmapLayerNames_digi = Monitored::buildToolMap<int>( m_tools, "LArDigitalTriggerMon_digi", m_layerNames);
  m_toolmapLayerNames_sc = Monitored::buildToolMap<int>( m_tools, "LArDigitalTriggerMon_sc", m_layerNames);
 
  ATH_MSG_INFO("Done building tool map");

  /** Get bad-channel mask (only if jO IgnoreBadChannels is true)*/
  ATH_CHECK(m_bcContKey.initialize());
  ATH_CHECK(m_bcMask.buildBitMask(m_problemsToMask,msg()));
 
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
  ATH_CHECK(m_LATOMEHeaderContainerKey.initialize());
  return AthMonitorAlgorithm::initialize();
}


struct Digi_MonValues {
  float digi_eta;
  float digi_phi;
  int digi_adc;
  float digi_diff_adc_ped;
  int digi_bcid;
  unsigned int digi_lb;
  bool digi_passDigiNom;
  bool digi_badNotMasked;

};

struct SC_MonValues {
  float sc_eta;
  float sc_phi;
  float sc_et_onl;
  float sc_et_onl_muscaled;
  float sc_time;
  int sc_bcid;
  unsigned int sc_lb;
  bool sc_passSCNom;
  bool sc_passSCNom1;
  bool sc_passSCNom10;
  bool sc_passSCNom10tauGt3;
  bool sc_saturNotMasked;
  bool sc_OFCbOFNotMasked;
};


StatusCode LArDigitalTriggMonAlg::fillHistograms(const EventContext& ctx) const
{
  
  ATH_MSG_DEBUG("in fillHists()" );
  
  // General Monitored variables
  auto lumi_block = Monitored::Scalar<unsigned int>("lumi_block", 0);
  auto BCID = Monitored::Scalar<int>("BCID",0);
  auto Pedestal = Monitored::Scalar<float>("Pedestal",0.0);
  auto PedestalRMS = Monitored::Scalar<float>("PedestalRMS",0.0);

  // From digi loop
  auto Digi_Nsamples = Monitored::Scalar<int>("Digi_Nsamples",-1);  // MNsamples
  auto Digi_SCChannel = Monitored::Scalar<int>("Digi_SCChannel",-1); // MSCChannel
  auto Digi_latomeSourceId = Monitored::Scalar<int>("Digi_latomeSourceId",-1); // MlatomeSourceId
  auto Digi_latomeSourceIdBIN = Monitored::Scalar<int>("Digi_latomeSourceIdBIN",1); // MlatomeSourceIdBIN
  auto Digi_phi = Monitored::Scalar<float>("Digi_phi",0.0); // MSCphi
  auto Digi_eta = Monitored::Scalar<float>("Digi_eta",0.0); // MSCeta
  auto Digi_maxpos = Monitored::Scalar<int>("Digi_maxpos",-1); // Mmaxpos
  auto Digi_partition = Monitored::Scalar<int>("Digi_partition",-1); // Mpartition
  auto Digi_sampos = Monitored::Scalar<int>("Digi_sampos",-1); // Msampos
  auto Digi_ADC = Monitored::Scalar<int>("Digi_ADC",-1); // MADC
  auto Digi_ADC_0 = Monitored::Scalar<int>("Digi_ADC_0",-1); // MADC_0
  auto Digi_Diff_ADC_Ped = Monitored::Scalar<float>("Digi_Diff_ADC_Ped", -999); // Diff_ADC_Pedestal
  auto Digi_Diff_ADC_Ped_Norm = Monitored::Scalar<float>("Digi_Diff_ADC_Ped_Norm",-999); // Diff_ADC_Pedestal_Norm
  // cuts
  auto notBadQual = Monitored::Scalar<bool>("notBadQual",false);
  auto ADCped10RMS = Monitored::Scalar<bool>("ADCped10RMS",false);
  auto passDigiNom = Monitored::Scalar<bool>("passDigiNom",false);
  auto badNotMasked = Monitored::Scalar<bool>("badNotMasked",false);

  // cuts which are used in both loops
  auto notMasked = Monitored::Scalar<bool>("notMasked",false);

  // From SC loop
  auto SC_SCChannel = Monitored::Scalar<int>("SC_SCChannel",-1); // MSCChannel
  auto SC_latomeSourceId = Monitored::Scalar<int>("SC_latomeSourceId",-1); // MlatomeSourceId
  auto SC_partition = Monitored::Scalar<int>("SC_partition",-1); // Mpartition
  auto SC_phi = Monitored::Scalar<float>("SC_phi",0.0); // MSCphi
  auto SC_eta = Monitored::Scalar<float>("SC_eta",0.0); // MSCeta
  auto SC_energy_onl = Monitored::Scalar<int>("SC_energy_onl",0.0); // Menergy_onl
  auto SC_ET_onl = Monitored::Scalar<float>("SC_ET_onl",0.0); // Menergy_onl
  auto SC_ET_onl_muscaled = Monitored::Scalar<float>("SC_ET_onl_muscaled",0.0); // Menergy_onl
  auto SC_energy_ofl = Monitored::Scalar<int>("SC_energy_ofl",0.0); // Menergy_ofl
  auto SC_ET_ofl = Monitored::Scalar<float>("SC_ET_ofl",0.0); // Menergy_onl
  auto SC_ET_diff = Monitored::Scalar<int>("SC_ET_diff",0.0); // MSCEt_diff
  auto SC_time = Monitored::Scalar<float>("SC_time",0.0); // MSCtime
  auto SC_latomeSourceIdBIN = Monitored::Scalar<int>("SC_latomeSourceIdBIN",1); // MlatomeSourceIdBIN
  auto SC_AvEnergyOverMu = Monitored::Scalar<float>("SC_AvEnergyOverMu",0); // LMAvEnergyOverMu
  // cuts
  auto passTauSel = Monitored::Scalar<bool>("passTauSel",false);
  auto nonZeroET = Monitored::Scalar<bool>("nonZeroET",false); // eTgt0GeV
  auto onlofflEmismatch = Monitored::Scalar<bool>("onlofflEmismatch",false);
  auto notSatur = Monitored::Scalar<bool>("notSatur",false);
  auto notOFCbOF = Monitored::Scalar<bool>("notOFCbOF",false);
  auto tauGt3 = Monitored::Scalar<bool>("tauGt3",false);
  auto nonZeroEtau = Monitored::Scalar<bool>("nonZeroEtau",false);
  auto eTgt1GeV = Monitored::Scalar<bool>("eTgt1GeV",false);
  auto eTgt10GeV = Monitored::Scalar<bool>("eTgt10GeV",false);


  auto passSCNom = Monitored::Scalar<bool>("passSCNom",false);  // pass tau, not satur, not OFCb OF, not masked  nonZeroET
  auto passSCNom1 = Monitored::Scalar<bool>("passSCNom1",false);  // pass tau, not satur, not OFCb OF, not masked eTgt1GeV
  auto passSCNom10 = Monitored::Scalar<bool>("passSCNom10",false);  // pass tau, not satur, not OFCb OF, not masked eTgt10GeV
  auto passSCNom10tauGt3 = Monitored::Scalar<bool>("passSCNom10tauGt3",false);  // pass tau, not satur, not OFCb OF, not masked eTgt10GeV  tauGt3
  auto saturNotMasked = Monitored::Scalar<bool>("saturNotMasked",false);  // notSatur is false, notMasked is false
  auto OFCbOFNotMasked = Monitored::Scalar<bool>("OFCbOFNotMasked",false);  // notOFCbOF is false, notMasked is false
  

  // From LATOME header loop
  auto event_size = Monitored::Scalar<float>("event_size",-1);

  /**EventID is a part of EventInfo, search event informations:*/
  SG::ReadHandle<xAOD::EventInfo> thisEvent{m_eventInfoKey,ctx};

  const std::vector<unsigned> streamsThisEvent=LArMon::trigStreamMatching(m_streams,thisEvent->streamTags());
  
  SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey,ctx};
  const LArOnOffIdMapping* cabling=*cablingHdl;

  SG::ReadHandle<LArDigitContainer> hLArDigitContainer{m_digitContainerKey,ctx}; //"SC"
  if (!hLArDigitContainer.isValid()) {
    ATH_MSG_WARNING("The requested digit container key could not be retrieved. Was there a problem retrieving information from the run logger?");
  }else{
    ATH_MSG_INFO("hLArDigitContainer.size() " << hLArDigitContainer->size());
  }
  SG::ReadHandle<LArRawSCContainer > hSCetContainer{m_rawSCContainerKey,ctx}; //"SC_ET"
  if (!hSCetContainer.isValid()) {
    ATH_MSG_WARNING("The requested SC ET container key could not be retrieved. Was there a problem retrieving information from the run logger?");
  }else{
    ATH_MSG_INFO("hSCetContainer.size() " << hSCetContainer->size());
  }
  SG::ReadHandle<LArRawSCContainer > hSCetRecoContainer{m_rawSCEtRecoContainerKey,ctx}; //"SC_ET_RECO"
  if (!hSCetRecoContainer.isValid()) {
    ATH_MSG_WARNING("The requested SC ET reco container key could not be retrieved. Was there a problem retrieving information from the run logger?");
  }else{
    ATH_MSG_INFO("hSCetRecoContainer.size() " << hSCetRecoContainer->size());
  }
  
  
  SG::ReadHandle<LArLATOMEHeaderContainer> hLArLATOMEHeaderContainer{m_LATOMEHeaderContainerKey,ctx}; //"SC_LATOME_HEADER"
  if (!hLArLATOMEHeaderContainer.isValid()) {
    ATH_MSG_WARNING("The requested LATOME header container key could not be retrieved. Was there a problem retrieving information from the run logger?");
  }else{
    ATH_MSG_INFO("hLArLATOMEHeaderContainer.size() " << hLArLATOMEHeaderContainer->size());
  }

  if ( hLArDigitContainer->size() == 0 && hSCetContainer->size() == 0 && hSCetRecoContainer->size() == 0 && hLArLATOMEHeaderContainer->size() == 0){
    //Make this only warning, come CI tests use the runs without DT info
    ATH_MSG_WARNING("All of the requested containers are empty. Was there a problem retrieving information from the run logger?");
    return StatusCode::SUCCESS;
  }

  BCID = thisEvent->bcid();// - 88)%((36+7)*4 + 36 + 31);
  lumi_block = thisEvent->lumiBlock();

  float mu = lbInteractionsPerCrossing(ctx);
  float event_mu = lbLuminosityPerBCID(ctx);

  ATH_MSG_INFO("ADC NAME: "<<m_AdcName);
  ATH_MSG_INFO("ET NAME: "<<m_EtName);
  ATH_MSG_INFO("mu (LB): "<<mu);
  ATH_MSG_INFO("mu (BCID): "<<event_mu);
  ATH_MSG_INFO("Event number: "<<thisEvent->eventNumber());
  ATH_MSG_INFO("LB number: "<<thisEvent->lumiBlock());
  ATH_MSG_INFO("BCID: "<<thisEvent->bcid());
  SG::ReadCondHandle<ILArPedestal>    pedestalHdl{m_keyPedestalSC, ctx};
  const LArPedestalSC* pedestals=dynamic_cast<const LArPedestalSC*>(pedestalHdl.cptr());

   SG::ReadCondHandle<CaloSuperCellDetDescrManager> caloSuperCellMgrHandle{m_caloSuperCellMgrKey,ctx};
  const CaloSuperCellDetDescrManager* ddman = *caloSuperCellMgrHandle;



  //retrieve BadChannel info:
  const LArBadChannelCont* bcCont=nullptr;
  SG::ReadCondHandle<LArBadChannelCont> bcContHdl{m_bcContKey,ctx};
  bcCont=(*bcContHdl);

  
  if ( (hLArDigitContainer.isValid()) ){
    const LArDigit* pLArDigit;
    std::string layer;

    std::vector<std::vector<Digi_MonValues>> digiMonValueVec;
    digiMonValueVec.reserve(m_layerNames.size());
    for (size_t ilayer = 0; ilayer < m_layerNames.size(); ++ilayer) {
      digiMonValueVec.emplace_back();
      digiMonValueVec[ilayer].reserve(1600); // (m_layerNcells[ilayer]) * nsamples;
    }
    

    // Loop over digits
    for ( auto itDig = hLArDigitContainer->begin(); itDig != hLArDigitContainer->end(); ++itDig ) {
      pLArDigit = *itDig;
      HWIdentifier id = pLArDigit->hardwareID(); //gives online ID
      const Identifier offlineID=cabling->cnvToIdentifier(id);
      const CaloDetDescrElement* caloDetElement = ddman->get_element(offlineID);
      unsigned int trueNSamples = pLArDigit->nsamples();
      Digi_Nsamples = trueNSamples; // Fill the monitored variable
      int cgain = pLArDigit->gain();

      //skip disconnected channels:
      if(!cabling->isOnlineConnected(id)) continue;   
      if(caloDetElement == 0 ){
	ATH_MSG_ERROR( "Cannot retrieve caloDetElement" );
        continue;
      } 
      Digi_eta = caloDetElement->eta_raw();
      Digi_phi = caloDetElement->phi_raw();
      int calosample=caloDetElement->getSampling();
      unsigned iLyrNS=m_caloSamplingToLyrNS.at(calosample);
      const unsigned whichSide=(Digi_eta>0) ? 0 : 1; //Value >0 means A-side 
      unsigned iLyr=iLyrNS*2+whichSide;
      auto& lvaluemap_digi = digiMonValueVec[iLyr];      
      auto layerName=m_layerNames[iLyr];
      // Determine to which partition this channel belongs to
      int side = m_LArOnlineIDHelper->pos_neg(id);
      const int ThisPartition=WhatPartition(id,side);
      Digi_partition = ThisPartition; // Fill the monitored variable


      fill(m_scMonGroupName, Digi_Nsamples);
      
      // Check if this is a maskedOSUM SC 
      notMasked = true;
      if ( m_bcMask.cellShouldBeMasked(bcCont,id)) {
	notMasked = false;
      }

      if(pedestals){
	Pedestal = pedestals->pedestal(id,cgain);
	PedestalRMS = pedestals->pedestalRMS(id,cgain);
      }
      else
	ATH_MSG_INFO( "Pedestal values not received");
      
      const LArSCDigit* scdigi = dynamic_cast<const LArSCDigit*>(pLArDigit);
      if(!scdigi){ ATH_MSG_DEBUG(" CAN'T CAST ");
      }else{
	Digi_SCChannel = scdigi->Channel();
	Digi_latomeSourceId = scdigi->SourceId();
	Digi_latomeSourceIdBIN=getXbinFromSourceID(Digi_latomeSourceId);
      }
      // Retrieve samples
      const std::vector<short>* digito = &pLArDigit->samples();
      
      //retrieve the max sample digit ie digitot.back().
      std::vector<short>::const_iterator maxSam = std::max_element(digito->begin(), digito->end());
      int thismaxPos = std::distance(digito->begin(), maxSam);
      Digi_maxpos=thismaxPos+1; //count samples [1,5]
      float ADC_max = pLArDigit->samples().at(Digi_maxpos-1);

      // max sample per layer
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

      // Start Loop over samples
      for(unsigned i=0; i<trueNSamples;++i) {
	badNotMasked = false;
	notBadQual = false;
	ADCped10RMS = false;
	passDigiNom = false;
	
	Digi_sampos=i+1;
	Digi_ADC = pLArDigit->samples().at(i);
	if (m_AdcName.size() >= 3){ //SC_ADC_BAS, have to divide by 8
	  Digi_ADC=Digi_ADC/8;
	}
	if(i==0){
          Digi_ADC_0=pLArDigit->samples().at(i);
	  if (m_AdcName.size() >= 3) Digi_ADC_0=Digi_ADC_0/8;
	}
	Digi_Diff_ADC_Ped = Digi_ADC - Pedestal;
	if ( ADC_max != Pedestal ){
	  Digi_Diff_ADC_Ped_Norm = (Digi_ADC - Pedestal) / std::abs(ADC_max  - Pedestal);
	}

	// Some selections
	if(Digi_ADC!=-1){
	  notBadQual = true;
	}else{
	  if ( notMasked ){
	    badNotMasked = true;
	  }
	}
	if(ADC_max - Pedestal > 10*PedestalRMS) { 
	  ADCped10RMS = true;
	}
	if ( notMasked && notBadQual && ADCped10RMS ){
	  passDigiNom = true;
	}
	lvaluemap_digi.push_back({Digi_eta, Digi_phi, Digi_ADC, Digi_Diff_ADC_Ped, BCID, lumi_block, passDigiNom, badNotMasked });

	fill(m_scMonGroupName, Digi_eta, Digi_phi, Digi_sampos, Digi_ADC, Digi_latomeSourceIdBIN, Pedestal, Digi_maxpos, Digi_partition, Digi_Diff_ADC_Ped_Norm, Digi_Diff_ADC_Ped, passDigiNom, badNotMasked);
      } // End loop over samples

    } // End of loop on LArDigit

    // fill, for every layer/threshold
    for (size_t ilayer = 0; ilayer < digiMonValueVec.size(); ++ilayer) {
      const auto& tool = digiMonValueVec[ilayer];
      auto digi_part_eta = Monitored::Collection("Digi_part_eta",tool,[](const auto& v){return v.digi_eta;});
      auto digi_part_phi = Monitored::Collection("Digi_part_phi",tool,[](const auto& v){return v.digi_phi;});
      auto digi_part_adc = Monitored::Collection("Digi_part_adc",tool,[](const auto& v){return v.digi_adc;});
      auto digi_part_diff_adc_ped = Monitored::Collection("Digi_part_diff_adc_ped",tool,[](const auto& v){return v.digi_diff_adc_ped;});
      auto digi_part_bcid = Monitored::Collection("Digi_part_BCID",tool,[](const auto& v){return v.digi_bcid;});
      auto digi_part_lb = Monitored::Collection("Digi_part_LB",tool,[](const auto& v){return v.digi_bcid;});
      auto digi_part_passDigiNom = Monitored::Collection("Digi_part_passDigiNom",tool,[](const auto& v){return v.digi_passDigiNom;});
      auto digi_part_badNotMasked = Monitored::Collection("Digi_part_badNotMasked",tool,[](const auto& v){return v.digi_badNotMasked;});

      fill(m_tools[m_toolmapLayerNames_digi.at(m_layerNames[ilayer])], 
	   digi_part_eta, digi_part_phi, digi_part_adc, digi_part_diff_adc_ped, digi_part_bcid, digi_part_lb, digi_part_passDigiNom, digi_part_badNotMasked);
    }
  


  } // End if(LArDigitContainer is valid)
 


  if ( hSCetContainer.isValid() && hSCetRecoContainer.isValid() ){
    LArRawSCContainer::const_iterator itSC = hSCetContainer->begin();
    LArRawSCContainer::const_iterator itSC_e= hSCetContainer->end(); 
    LArRawSCContainer::const_iterator itSCReco = hSCetRecoContainer->begin();
    const LArRawSC* rawSC = 0;
    const LArRawSC* rawSCReco = 0;

    std::vector<std::vector<SC_MonValues>> scMonValueVec;
    scMonValueVec.reserve(m_layerNames.size());
    for (size_t ilayer = 0; ilayer < m_layerNames.size(); ++ilayer) {
      scMonValueVec.emplace_back();
      scMonValueVec[ilayer].reserve(1600); // (m_layerNcells[ilayer]) * nsamples;
    }

    // Loop over SCs
    for ( ; itSC!=itSC_e;++itSC,++itSCReco) {
      rawSC = *itSC;
      if ( itSCReco < hSCetRecoContainer->end() ){ 
	rawSCReco = *itSCReco;	
      }else{
	ATH_MSG_WARNING("Looping SC ET container, but we have reached the end of the SC ET Reco iterator. Check the sizes of these containers. Is SC ET Reco size zero? Is there a problem with the digit container name sent by the run logger?");
	rawSCReco = 0;
      }
      Digi_SCChannel = rawSC->chan();
      HWIdentifier id = rawSC->hardwareID(); // gives online ID
      const Identifier offlineID=cabling->cnvToIdentifier(id); //converts online to offline ID
      Identifier32 Ofl32 =offlineID.get_identifier32();
      Identifier32 Onl32 =id.get_identifier32();
      // Get Physical Coordinates
      const CaloDetDescrElement* caloDetElement = ddman->get_element(offlineID);
      
      //skip disconnected channels:
      if(!cabling->isOnlineConnected(id)) continue;   
      
      if(caloDetElement == 0 ){
	ATH_MSG_ERROR( "Cannot retrieve (eta,phi) coordinates for raw channels" );
	ATH_MSG_ERROR( "  ==============> " << std::hex << ";  offlineID = "<< offlineID << "; Ofl32 compact= "<< Ofl32.get_compact()<< "; online ID =" << id << "; Onl32 = " << Onl32.get_compact()  << "; rawSC->SourceId() = " << rawSC->SourceId());
	continue; 
      }
      SC_eta = caloDetElement->eta_raw();
      SC_phi = caloDetElement->phi_raw();
      int calosample=caloDetElement->getSampling();
      unsigned iLyrNS=m_caloSamplingToLyrNS.at(calosample);
      const unsigned side=(SC_eta>0) ? 0 : 1; //Value >0 means A-side
      unsigned iLyr=iLyrNS*2+side;
      auto& lvaluemap_sc = scMonValueVec[iLyr];      
      SC_latomeSourceIdBIN=getXbinFromSourceID(rawSC->SourceId());
      auto layerName=m_layerNames[iLyr];
      std::string layer;
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
      
      // initialise cuts
      notMasked = false;
      passTauSel = false;
      nonZeroET = false;
      notSatur = false;
      nonZeroEtau = false;
      eTgt1GeV = false;
      eTgt10GeV = false;
      notOFCbOF = false;
      tauGt3 = false;
      onlofflEmismatch=false;
      passSCNom = false;
      passSCNom1 = false;
      passSCNom10 = false;
      passSCNom10tauGt3 = false;
      saturNotMasked = false;
      OFCbOFNotMasked = false;
      OFCbOFNotMasked = false;
      
      
      // Check if this is a maskedOSUM SC 
      if ( ! m_bcMask.cellShouldBeMasked(bcCont,id)) {
	notMasked = true;
      }
      if ( rawSCReco != 0 && rawSCReco->passTauSelection().at(0) == true){ //only compare Et if tau selection is passed
	passTauSel = true;
      }
      int bcid_ind = 0;
      if (rawSC->energies().size()>0){
	for ( auto & SCe : rawSC->bcids() ) 
	  {
	    if ( SCe == BCID ) break;
	    bcid_ind++;
	  }
      }
      if ( rawSC->bcids().at(bcid_ind) != BCID ) ATH_MSG_WARNING("BCID not found in SC bcids list!! "<<BCID<<" "<<rawSC->bcids().at(bcid_ind));
      
      SC_energy_onl = rawSC->energies().at(bcid_ind); 
      if ( rawSCReco != 0 ){ 
	SC_energy_ofl = rawSCReco->energies().at(0); // algorithm already selects the correct energy 
      } 
      SC_ET_diff = SC_energy_onl - SC_energy_ofl;	
      SC_ET_onl = ( SC_energy_onl* 12.5 ) / 1000;  // Converted to GeV
      SC_ET_ofl = ( SC_energy_ofl* 12.5 ) / 1000;  // Converted to GeV
      SC_ET_onl_muscaled = event_mu > 0. ? SC_ET_onl / event_mu : SC_ET_onl;
      int Etau = 0; 
      if ( rawSCReco != 0 ){ Etau = rawSCReco->tauEnergies().at(0); }
      SC_time = (SC_energy_ofl != 0) ? (float)Etau / (float)SC_energy_ofl : Etau; 
      
      ATH_MSG_DEBUG("Energy onl - Energy ofl: "<<SC_energy_onl<<",  "<<SC_energy_ofl<<std::endl); 
      if (SC_ET_onl != 0 ){
	nonZeroET = true;
      }
      if (SC_ET_onl > 1){
	eTgt1GeV = true;
      }
      if (SC_ET_onl > 10){
	eTgt10GeV = true;
      }
      if ( rawSC->satur().size()>0 ){
	if ( rawSC->satur().at(bcid_ind) ){
	  if ( notMasked ){
	    saturNotMasked = true;  
	  }
	}else{
	  notSatur = true;
	}
      }
      if (Etau != 0){
	nonZeroEtau = true;
      }
      if ( rawSCReco != 0 && rawSCReco->ofcbOverflow() == false ){
	notOFCbOF = true;
      }else{
	if ( notMasked ){
	  OFCbOFNotMasked = true;
	}
      }
      if ( std::abs(SC_time) > 3 ){
	tauGt3 = true;
      }
      if ( notMasked && passTauSel && notSatur && notOFCbOF ){
	if ( nonZeroET ){
	  passSCNom = true;
	}
	if ( eTgt1GeV ){
	  passSCNom1 = true;
	}
	if (eTgt10GeV ){
	  passSCNom10 = true;
	  if ( tauGt3 ){
	    passSCNom10tauGt3 = true;
	  }
	}
	if (SC_energy_onl != SC_energy_ofl){
	  onlofflEmismatch=true;
	}
	
      } // end nominal selections
      

      fill(m_scMonGroupName, SC_eta, SC_phi, SC_latomeSourceIdBIN, SC_ET_ofl, SC_ET_onl, SC_ET_diff, SC_time, lumi_block, passSCNom, passSCNom1, passSCNom10, passSCNom10tauGt3, onlofflEmismatch, saturNotMasked, OFCbOFNotMasked);

      lvaluemap_sc.push_back({SC_eta, SC_phi, SC_ET_onl, SC_ET_onl_muscaled, SC_time, BCID, lumi_block, passSCNom, passSCNom1, passSCNom10, passSCNom10tauGt3, saturNotMasked, OFCbOFNotMasked});


    } //end loop over SCs


    // fill, for every layer/threshold
    for (size_t ilayer = 0; ilayer < scMonValueVec.size(); ++ilayer) {
      const auto& tool = scMonValueVec[ilayer];
      auto sc_part_eta = Monitored::Collection("SC_part_eta",tool,[](const auto& v){return v.sc_eta;});
      auto sc_part_phi = Monitored::Collection("SC_part_phi",tool,[](const auto& v){return v.sc_phi;});
      auto sc_part_et_onl = Monitored::Collection("SC_part_ET_onl",tool,[](const auto& v){return v.sc_et_onl;});
      auto sc_part_et_onl_muscaled = Monitored::Collection("SC_part_ET_onl_muscaled",tool,[](const auto& v){return v.sc_et_onl_muscaled;});
      auto sc_part_time = Monitored::Collection("SC_part_time",tool,[](const auto& v){return v.sc_time;});
      auto sc_part_bcid = Monitored::Collection("SC_part_BCID",tool,[](const auto& v){return v.sc_bcid;});
      auto sc_part_lb = Monitored::Collection("SC_part_LB",tool,[](const auto& v){return v.sc_bcid;});
      auto sc_part_passSCNom = Monitored::Collection("SC_part_passSCNom",tool,[](const auto& v){return v.sc_passSCNom;});
      auto sc_part_passSCNom1 = Monitored::Collection("SC_part_passSCNom1",tool,[](const auto& v){return v.sc_passSCNom1;});
      auto sc_part_passSCNom10 = Monitored::Collection("SC_part_passSCNom10",tool,[](const auto& v){return v.sc_passSCNom10;});
      auto sc_part_passSCNom10tauGt3 = Monitored::Collection("SC_part_passSCNom10tauGt3",tool,[](const auto& v){return v.sc_passSCNom10tauGt3;});
      auto sc_part_saturNotMasked = Monitored::Collection("SC_part_saturNotMasked",tool,[](const auto& v){return v.sc_saturNotMasked;});
      auto sc_part_OFCbOFNotMasked = Monitored::Collection("SC_part_OFCbOFNotMasked",tool,[](const auto& v){return v.sc_OFCbOFNotMasked;});


      fill(m_tools[m_toolmapLayerNames_sc.at(m_layerNames[ilayer])], 
	   sc_part_eta, sc_part_phi, sc_part_et_onl, sc_part_et_onl_muscaled, sc_part_time, sc_part_bcid, sc_part_lb, sc_part_passSCNom, sc_part_passSCNom1, sc_part_passSCNom10, sc_part_passSCNom10tauGt3, sc_part_saturNotMasked, sc_part_OFCbOFNotMasked);
    }

     
  }  // End if(LArSCContainer is valid)

 

  //LATOME event size
  if ( (hLArLATOMEHeaderContainer.isValid()) ){
    const LArLATOMEHeader* pLArLATOMEHeader;
    for ( auto itLatH = hLArLATOMEHeaderContainer->begin(); itLatH != hLArLATOMEHeaderContainer->end(); ++itLatH ){
      pLArLATOMEHeader = *itLatH;
      event_size += pLArLATOMEHeader->ROBFragSize() + 48; //48 is the offset between rod_ndata and ROB fragment size
    }
    event_size /= (1024*1024/4);
    fill(m_scMonGroupName,lumi_block,event_size);
  }
  

//end LATOME event size

  
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

