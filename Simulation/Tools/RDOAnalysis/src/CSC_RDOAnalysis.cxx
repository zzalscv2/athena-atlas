/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/


#include "CSC_RDOAnalysis.h"

#include "TTree.h"
#include "TString.h"

#include <algorithm>
#include <math.h>
#include <functional>
#include <iostream>

CSC_RDOAnalysis::CSC_RDOAnalysis(const std::string& name, ISvcLocator *pSvcLocator)
  : AthAlgorithm(name, pSvcLocator)
  , m_collID(0)
  , m_rodID(0)
  , m_subID(0)
  , m_collRpuID(0)
  , m_dataType(0)
  , m_smplPhase(0)
  , m_trigType(0)
  , m_firstBitSum(0)
  , m_eventType(0)
  , m_calAmp(0)
  , m_calLayer(0)
  , m_scaAdd(0)
  , m_numSmpl(0)
  , m_latency(0)
  , m_rate(0)
  , m_sparse(0)
  , m_neutron(0)
  , m_calEn(0)
  , m_spuCt(0)
  , m_cscRpuID(0)
  , m_cscID(0)
  , m_cscTime(0)
  , m_cscWidth(0)
  , m_cscTimeComp(0)
  , m_cscSmpl(0)
  , m_cscAdd(0)
  , m_cscHashID(0)
  , m_sdoID(0)
  , m_sdoWord(0)
  , m_barcode(0)
  , m_eventIndex(0)
  , m_energy(0)
  , m_ypos(0)
  , m_zpos(0)
  , m_charge(0)
  , m_barcode_vec(0)
  , m_eventIndex_vec(0)
  , m_energy_vec(0)
  , m_ypos_vec(0)
  , m_zpos_vec(0)
  , m_charge_vec(0)

  , h_collID(0)
  , h_rodID(0)
  , h_subID(0)
  , h_collRpuID(0)
  , h_dataType(0)
  , h_spuCt(0)
  , h_cscRpuID(0)
  , h_cscID(0)
  , h_cscTime(0)
  , h_cscWidth(0)
  , h_cscSmpl(0)
  , h_cscAdd(0)
  , h_cscHashID(0)
  , h_sdoID(0)
  , h_sdoWord(0)
  , h_barcode(0)
  , h_eventIndex(0)
  , h_energy(0)
  , h_ypos(0)
  , h_zpos(0)
  , h_charge(0)
    
  , m_tree(0)
  , m_ntupleFileName("/ntuples/file1")
  , m_ntupleDirName("/CSC_RDOAnalysis/")
  , m_ntupleTreeName("CSC_RDOAna")
  , m_path("/CSC_RDOAnalysis/")
  , m_thistSvc("THistSvc", name)
{
  declareProperty("NtupleFileName", m_ntupleFileName);
  declareProperty("NtupleDirectoryName", m_ntupleDirName);
  declareProperty("NtupleTreeName", m_ntupleTreeName);
  declareProperty("HistPath", m_path);
}

StatusCode CSC_RDOAnalysis::initialize() {
  ATH_MSG_DEBUG( "Initializing CSC_RDOAnalysis" );

  ATH_CHECK(m_thistSvc.retrieve());

  m_tree = new TTree(TString(m_ntupleTreeName), "CSC_RDOAna");
  std::string fullNtupleName = m_ntupleFileName + m_ntupleDirName + m_ntupleTreeName;
  ATH_CHECK(m_thistSvc->regTree(fullNtupleName, m_tree));
  if (m_tree) {
    m_tree->Branch("collID", &m_collID);
    m_tree->Branch("rodID", &m_rodID);
    m_tree->Branch("subID", &m_subID);
    m_tree->Branch("collRpuID", &m_collRpuID);
    m_tree->Branch("dataType", &m_dataType);
    m_tree->Branch("smplPhase", &m_smplPhase);
    m_tree->Branch("trigType", &m_trigType);
    m_tree->Branch("firstBitSum", &m_firstBitSum);
    m_tree->Branch("eventType", &m_eventType);
    m_tree->Branch("calAmp", &m_calAmp);
    m_tree->Branch("calLayer", &m_calLayer);
    m_tree->Branch("scaAdd", &m_scaAdd);
    m_tree->Branch("numSmpl", &m_numSmpl);
    m_tree->Branch("latency", &m_latency);
    m_tree->Branch("rate", &m_rate);
    m_tree->Branch("sparse", &m_sparse);
    m_tree->Branch("neutron", &m_neutron);
    m_tree->Branch("calEn", &m_calEn);
    m_tree->Branch("spuCt", &m_spuCt);
    m_tree->Branch("cscRpuID", &m_cscRpuID);
    m_tree->Branch("cscID", &m_cscID);
    m_tree->Branch("cscTime", &m_cscTime);
    m_tree->Branch("cscWidth", &m_cscWidth);
    m_tree->Branch("cscTimeComp", &m_cscTimeComp);
    m_tree->Branch("cscSmpl", &m_cscSmpl);
    m_tree->Branch("cscAdd", &m_cscAdd);
    m_tree->Branch("cscHashID", &m_cscHashID);
    m_tree->Branch("sdoID", &m_sdoID);
    m_tree->Branch("sdoWord", &m_sdoWord);
    m_tree->Branch("barcode", &m_barcode);
    m_tree->Branch("eventIndex", &m_eventIndex);
    m_tree->Branch("energy", &m_energy);
    m_tree->Branch("ypos", &m_ypos);
    m_tree->Branch("zpos", &m_zpos);
    m_tree->Branch("charge", &m_charge);
    m_tree->Branch("barcode_vec", &m_barcode_vec);
    m_tree->Branch("eventIndex_vec", &m_eventIndex_vec);
    m_tree->Branch("energy_vec", &m_energy_vec);
    m_tree->Branch("ypos_vec", &m_ypos_vec);
    m_tree->Branch("zpos_vec", &m_zpos_vec);
    m_tree->Branch("charge_vec", &m_charge_vec);
  }
  else {
    ATH_MSG_ERROR("No tree found!");
  }

  // HISTOGRAMS

  h_collID = new TH1F("h_collID", "Collection ID", 100, 0, 35);
  h_collID->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + h_collID->GetName(), h_collID));

  h_rodID = new TH1F("h_rodID", "Collection ROD ID", 100, 0, 150);
  h_rodID->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + h_rodID->GetName(), h_rodID));

  h_subID = new TH1F("h_subID", "Collection Sub-Detector ID", 100, 0, 110);
  h_subID->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + h_subID->GetName(), h_subID));

  h_collRpuID = new TH1F("h_collRpuID", "Collection RPU ID", 100, 0, 15);
  h_collRpuID->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + h_collRpuID->GetName(), h_collRpuID));

  h_dataType = new TH1F("h_dataType", "Collection data type", 100, 0, 5);
  h_dataType->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + h_dataType->GetName(), h_dataType));

  h_spuCt = new TH1F("h_spuCt", "Collection SPU count", 100, 0, 20);
  h_spuCt->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + h_spuCt->GetName(), h_spuCt));
  
  h_cscRpuID = new TH1F("h_cscRpuID", "SPU ID of strip", 100, 0, 10);
  h_cscRpuID->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + h_cscRpuID->GetName(), h_cscRpuID));

  h_cscID = new TH1F("h_cscID", "ID of strip collection", 100, 0, 35);
  h_cscID->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + h_cscID->GetName(), h_cscID));

  h_cscTime = new TH1F("h_cscTime", "Time of first strip", 100, 0, 10);
  h_cscTime->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + h_cscTime->GetName(), h_cscTime));

  h_cscWidth = new TH1F("h_cscWidth", "Width of strip cluster", 100, 0, 35);
  h_cscWidth->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + h_cscWidth->GetName(), h_cscWidth));

  h_cscSmpl = new TH1F("h_cscSmpl", "ADC samples", 100, 0, 4500);
  h_cscSmpl->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + h_cscSmpl->GetName(), h_cscSmpl));

  h_cscAdd = new TH1F("h_cscAdd", "ID of first strip", 100, 0, 1.5e5);
  h_cscAdd->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + h_cscAdd->GetName(), h_cscAdd));

  h_cscHashID = new TH1F("h_cscHashID", "hash ID of strip collection", 100, 0, 65000);
  h_cscHashID->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + h_cscHashID->GetName(), h_cscHashID));

  h_sdoID = new TH1F("h_sdoID", "sdoID", 100, 0, 1e19);
  h_sdoID->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + h_sdoID->GetName(), h_sdoID));

  h_sdoWord = new TH1F("h_sdoWord", "sdoWord", 100, 0, 10);
  h_sdoWord->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + h_sdoWord->GetName(), h_sdoWord));

  h_barcode= new TH1F("h_barcode", "Barcode (SDO)", 100, 0, 2.2e9);
  h_barcode->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + h_barcode->GetName(), h_barcode));

  h_eventIndex = new TH1F("h_eventIndex", "Event index (SDO)", 100, 0, 1000);
  h_eventIndex->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + h_eventIndex->GetName(), h_eventIndex));

  h_energy = new TH1F("h_energy", "Energy (SDO)", 100, 0, 0.3);
  h_energy->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + h_energy->GetName(), h_energy));

  h_ypos = new TH1F("h_ypos", "y-position (SDO)", 100, -1e5, 1e5);
  h_ypos->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + h_ypos->GetName(), h_ypos));

  h_zpos = new TH1F("h_zpos", "z-position (SDO)", 100, -1e5, 1e5);
  h_zpos->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + h_zpos->GetName(), h_zpos));

  h_charge = new TH1F("h_charge", "Charge (SDO)", 100, 0, 1e5);
  h_charge->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_path + h_charge->GetName(), h_charge));

  return StatusCode::SUCCESS;
}

StatusCode CSC_RDOAnalysis::execute() {
  ATH_MSG_DEBUG( "In CSC_RDOAnalysis::execute()" );

  m_collID->clear();
  m_rodID->clear();
  m_subID->clear();
  m_collRpuID->clear();
  m_dataType->clear();
  m_smplPhase->clear();
  m_trigType->clear();
  m_firstBitSum->clear();
  m_eventType->clear();
  m_calAmp->clear();
  m_calLayer->clear();
  m_scaAdd->clear();
  m_numSmpl->clear();
  m_latency->clear();
  m_rate->clear();
  m_sparse->clear();
  m_neutron->clear();
  m_calEn->clear();
  m_spuCt->clear();
  m_cscRpuID->clear();
  m_cscID->clear();
  m_cscTime->clear();
  m_cscWidth->clear();
  m_cscTimeComp->clear();
  m_cscSmpl->clear();
  m_cscAdd->clear();
  m_cscHashID->clear();
  m_sdoID->clear();
  m_sdoWord->clear();
  m_barcode->clear();
  m_eventIndex->clear();
  m_energy->clear();
  m_ypos->clear();
  m_zpos->clear();
  m_charge->clear();
  m_barcode_vec->clear();
  m_eventIndex_vec->clear();
  m_energy_vec->clear();
  m_ypos_vec->clear();
  m_zpos_vec->clear();
  m_charge_vec->clear();
  
  const CscRawDataContainer* p_CSCcont;
  if (evtStore()->retrieve(p_CSCcont, "CSCRDO") == StatusCode::SUCCESS) {
    CscRawDataContainer::const_iterator cscCont_itr(p_CSCcont->begin());
    const CscRawDataContainer::const_iterator cscCont_end(p_CSCcont->end());

    for ( ; cscCont_itr != cscCont_end; ++cscCont_itr ) {

      const uint16_t collID((*cscCont_itr)->identify());
      const uint16_t rodID((*cscCont_itr)->rodId());
      const uint16_t subID((*cscCont_itr)->subDetectorId());
      const std::vector<uint16_t>& collRpuID = (*cscCont_itr)->rpuID();
      const std::vector<uint8_t>& dataType = (*cscCont_itr)->dataType();
      const bool smplPhase((*cscCont_itr)->samplingPhase());
      const bool trigType((*cscCont_itr)->triggerType());
      const uint8_t firstBitSum((*cscCont_itr)->firstBitSummary());
      const uint32_t evtType((*cscCont_itr)->eventType());
      const uint8_t calAmp((*cscCont_itr)->calAmplitude());
      const uint8_t calLay((*cscCont_itr)->calLayer());
      const uint32_t scaAdd((*cscCont_itr)->scaAddress());
      const uint16_t numSmpl((*cscCont_itr)->numSamples());
      const uint16_t collLatency((*cscCont_itr)->latency());
      const uint8_t collRate((*cscCont_itr)->rate());
      const bool sparse((*cscCont_itr)->sparsified());
      const bool neut((*cscCont_itr)->neutron());
      const bool calEn((*cscCont_itr)->calEnabled());
      
      m_collID->push_back(collID);
      m_rodID->push_back(rodID);
      m_subID->push_back(subID);
      
      for (std::vector<uint16_t>::size_type i = 0; i != collRpuID.size(); ++i) {
	m_collRpuID->push_back(collRpuID.at(i));
	h_collRpuID->Fill(collRpuID.at(i));
      }
      for (std::vector<uint8_t>::size_type j = 0; j != dataType.size(); ++j) {
	m_dataType->push_back(dataType.at(j));
	h_dataType->Fill(dataType.at(j));
      }
      
      m_smplPhase->push_back((*cscCont_itr)->samplingPhase());
      m_trigType->push_back((*cscCont_itr)->triggerType());
      m_firstBitSum->push_back((*cscCont_itr)->firstBitSummary());
      m_eventType->push_back((*cscCont_itr)->eventType());
      m_calAmp->push_back((*cscCont_itr)->calAmplitude());
      m_calLayer->push_back((*cscCont_itr)->calLayer());
      m_scaAdd->push_back((*cscCont_itr)->scaAddress());
      m_numSmpl->push_back((*cscCont_itr)->numSamples());
      m_latency->push_back((*cscCont_itr)->latency());
      m_rate->push_back((*cscCont_itr)->rate());
      m_sparse->push_back((*cscCont_itr)->sparsified());
      m_neutron->push_back((*cscCont_itr)->neutron());
      m_calEn->push_back((*cscCont_itr)->calEnabled());
      
      for (unsigned int k = 0; k != 10; ++k) {
	m_spuCt->push_back((*cscCont_itr)->spuCount(k));
	h_spuCt->Fill((*cscCont_itr)->spuCount(k));
      }

      h_collID->Fill(collID);
      h_rodID->Fill(rodID);
      h_subID->Fill(subID);

      const CscRawDataCollection* p_CSCcoll(*cscCont_itr);
      CscRawDataCollection::const_iterator csc_itr(p_CSCcoll->begin());
      const CscRawDataCollection::const_iterator csc_end(p_CSCcoll->end());
      for ( ; csc_itr != csc_end; ++csc_itr ) {

	const uint16_t cscRpuID((*csc_itr)->rpuID());
      	const uint16_t cscID((*csc_itr)->identify());
      	const uint16_t cscTime((*csc_itr)->time());
      	const uint16_t cscWidth((*csc_itr)->width());
      	const bool cscTimeComp((*csc_itr)->isTimeComputed());
      	const std::vector<uint16_t> cscSmpl_vec((*csc_itr)->samples());
      	const uint32_t cscAdd((*csc_itr)->address());
      	const uint32_t cscHashID((*csc_itr)->hashId());
	
	m_cscRpuID->push_back(cscRpuID);
      	m_cscID->push_back(cscID);
      	m_cscTime->push_back(cscTime);
      	m_cscWidth->push_back(cscWidth);
      	m_cscTimeComp->push_back(cscTimeComp);
      	m_cscSmpl->push_back(cscSmpl_vec);
      	m_cscAdd->push_back(cscAdd);
      	m_cscHashID->push_back(cscHashID);

	h_cscRpuID->Fill(cscRpuID);
	h_cscID->Fill(cscID);
      	h_cscTime->Fill(cscTime);
      	h_cscWidth->Fill(cscWidth);
	for (std::vector<uint16_t>::size_type l = 0; l != cscSmpl_vec.size(); ++l) {
	  h_cscSmpl->Fill(cscSmpl_vec.at(l));
	}
	h_cscAdd->Fill(cscAdd);
	h_cscHashID->Fill(cscHashID);
      }
    }
  }

  // SimData
  const CscSimDataCollection* simDataMapCSC(nullptr);
  if (evtStore()->retrieve(simDataMapCSC, "CSC_SDO") == StatusCode::SUCCESS) {
    CscSimDataCollection::const_iterator sdo_itr(simDataMapCSC->begin());
    const CscSimDataCollection::const_iterator sdo_end(simDataMapCSC->end());

    std::vector<int> barcode_vec;
    std::vector<int> eventIndex_vec;
    std::vector<float> energy_vec;
    std::vector<float> ypos_vec;
    std::vector<float> zpos_vec;
    std::vector<float> charge_vec;
    for ( ; sdo_itr != sdo_end; ++sdo_itr ) {
      const Identifier sdoID((*sdo_itr).first);
      const CscSimData& sdo((*sdo_itr).second);

      const unsigned long long sdoID_int = sdoID.get_compact();
      const int sdoWord(sdo.word());

      m_sdoID->push_back(sdoID_int);
      m_sdoWord->push_back(sdoWord);

      // loop over deposits
      const std::vector<CscSimData::Deposit>& deposits = sdo.getdeposits();
      std::vector<CscSimData::Deposit>::const_iterator dep_itr(deposits.begin());
      const std::vector<CscSimData::Deposit>::const_iterator dep_end(deposits.end());
      for ( ; dep_itr != dep_end; ++dep_itr ) {
	const HepMcParticleLink& particleLink = (*dep_itr).first;
	const int bar(particleLink.barcode());
	const int eventIx(particleLink.eventIndex());
	const CscMcData& data = (*dep_itr).second;
	const float sdoEnergy(data.energy());
	const float sdoYpos(data.ypos());
	const float sdoZpos(data.zpos());
	const float sdoCharge(data.charge());
	
	m_barcode->push_back(bar);
	m_eventIndex->push_back(eventIx);
	m_energy->push_back(sdoEnergy);
	m_ypos->push_back(sdoYpos);
	m_zpos->push_back(sdoZpos);
	m_charge->push_back(sdoCharge);

	h_barcode->Fill(bar);
	h_eventIndex->Fill(eventIx);
	h_energy->Fill(sdoEnergy);
	h_ypos->Fill(sdoYpos);
	h_zpos->Fill(sdoZpos);
	h_charge->Fill(sdoCharge);
	
	barcode_vec.push_back(bar);
	eventIndex_vec.push_back(eventIx);
	energy_vec.push_back(sdoEnergy);
	ypos_vec.push_back(sdoYpos);
	zpos_vec.push_back(sdoZpos);
	charge_vec.push_back(sdoCharge);
      }
      m_barcode_vec->push_back(barcode_vec);
      m_eventIndex_vec->push_back(eventIndex_vec);
      m_energy_vec->push_back(energy_vec);
      m_ypos_vec->push_back(ypos_vec);
      m_zpos_vec->push_back(zpos_vec);
      m_charge_vec->push_back(charge_vec);
      barcode_vec.clear();
      eventIndex_vec.clear();
      energy_vec.clear();
      ypos_vec.clear();
      zpos_vec.clear();
      charge_vec.clear();
    }
  }

  if (m_tree) {
    m_tree->Fill();
  }

  return StatusCode::SUCCESS;
}

StatusCode CSC_RDOAnalysis::finalize() {
  return StatusCode::SUCCESS;
}
