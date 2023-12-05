/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "ZDCHitAnalysis.h"

#include "ZDC_SimEvent/ZDC_SimFiberHit_Collection.h"
#include "ZDC_SimEvent/ZDC_SimFiberHit.h"
#include "CaloSimEvent/CaloCalibrationHit.h"
#include "CaloSimEvent/CaloCalibrationHitContainer.h"

#include "TH1.h"
#include "TString.h"

#include <algorithm>
#include <math.h>
#include <functional>
#include <iostream>
#include <stdio.h>

ZDCHitAnalysis::ZDCHitAnalysis(const std::string& name, ISvcLocator* pSvcLocator)
  : AthAlgorithm(name, pSvcLocator)
  , m_zdc_fiber_side(0)
  , m_zdc_fiber_mod(0)
  , m_zdc_fiber_channel(0)
  , m_zdc_fiber_photons(0)
  , m_zdc_calib_side(0)
  , m_zdc_calib_mod(0)
  , m_zdc_calib_channel(0)
  , m_zdc_calib_Total(0)
  , m_zdc_calib_EM(0)
  , m_zdc_calib_NonEM(0)
  , m_tree(0)
  , m_ntupleFileName("/ZDCHitAnalysis/")
  , m_path("/ZDCHitAnalysis/")
  , m_thistSvc("THistSvc", name)
  , m_ZdcID(nullptr)
{
  declareProperty("NtupleFileName", m_ntupleFileName);
  declareProperty("HistPath", m_path);
}

StatusCode ZDCHitAnalysis::initialize() {
  ATH_MSG_DEBUG( "Initializing ZDCHitAnalysis" );

  if (detStore()->retrieve( m_ZdcID ).isFailure() ) {
    msg(MSG::ERROR) << "execute: Could not retrieve ZdcID object from the detector store" << endmsg;
    return StatusCode::FAILURE;
}

  // Grab the Ntuple and histogramming service for the tree
  CHECK(m_thistSvc.retrieve());

  for(int side : {0,1}){
    for(int module = 0; module < 5; module++){
      std::string name = Form("%s%d", (side==1) ? "a" : "c", module);
      m_h_zdc_photons[side][module] = new TH1I( ("m_edep_module_" + name).c_str(), ("edep_module_" + name).c_str(), 100, 0, 20000);
      CHECK(m_thistSvc->regHist(m_path + m_h_zdc_photons[side][module]->GetName(), m_h_zdc_photons[side][module]));
    }
  }

  for(int side : {0,1}){
    for(int module = 0; module < 5; module++){
      std::string name = Form("%s%d", (side==1) ? "a" : "c", module);
      m_h_zdc_calibTot[side][module] = new TH1D( ("m_calibTot_module_" + name).c_str(), ("calibTot_module_" + name).c_str(), 100, 1e-2, 1e7);
      m_h_zdc_calibEM[side][module] = new TH1D( ("m_calibEM_module_" + name).c_str(), ("calibEM_module_" + name).c_str(), 100, 1e-2, 1e7);
      m_h_zdc_calibNonEM[side][module] = new TH1D( ("m_calibNonEM_module_" + name).c_str(), ("calibNonEM_module_" + name).c_str(), 100, 1e-2, 1e7);
      
      CHECK(m_thistSvc->regHist(m_path + m_h_zdc_calibTot[side][module]->GetName(), m_h_zdc_calibTot[side][module]));
      CHECK(m_thistSvc->regHist(m_path + m_h_zdc_calibEM[side][module]->GetName(), m_h_zdc_calibEM[side][module]));
      CHECK(m_thistSvc->regHist(m_path + m_h_zdc_calibNonEM[side][module]->GetName(), m_h_zdc_calibNonEM[side][module]));
    }
  }
 
  /** now add branches and leaves to the tree */
  m_tree = new TTree("ZDC","ZDC");
  std::string fullNtupleName =  "/" + m_ntupleFileName + "/";
  CHECK(m_thistSvc->regTree(fullNtupleName,m_tree));
  
  if (m_tree) {
    m_tree->Branch("fiber_side", &m_zdc_fiber_side);
    m_tree->Branch("fiber_mod", &m_zdc_fiber_mod);
    m_tree->Branch("fiber_channel", &m_zdc_fiber_channel);
    m_tree->Branch("fiber_nphotons", &m_zdc_fiber_photons);

    m_tree->Branch("calib_side", &m_zdc_calib_side);
    m_tree->Branch("calib_mod", &m_zdc_calib_mod);
    m_tree->Branch("calib_channel", &m_zdc_calib_channel);
    m_tree->Branch("calib_total", &m_zdc_calib_Total);
    m_tree->Branch("calib_em", &m_zdc_calib_EM);
    m_tree->Branch("calib_nonem", &m_zdc_calib_NonEM);

  }
  else {
    ATH_MSG_ERROR("No tree found!");
  }

  return StatusCode::SUCCESS;
}


StatusCode ZDCHitAnalysis::execute() {
  ATH_MSG_DEBUG( "In ZDCHitAnalysis::execute()" );
  
  m_zdc_fiber_side->clear();
  m_zdc_fiber_mod->clear();
  m_zdc_fiber_channel->clear();
  m_zdc_fiber_photons->clear();

  double photons_fiber = -1;
  int side_fiber = -1;
  int mod_fiber = -1;
  int channel_fiber = -1;

  ZDC_SimFiberHit_ConstIterator fiberhi;
  const ZDC_SimFiberHit_Collection* fiberiter;
  CHECK(evtStore()->retrieve(fiberiter,"ZDC_SimFiberHit_Collection"));
  for (fiberhi=(*fiberiter).begin(); fiberhi != (*fiberiter).end(); ++fiberhi) {
    ZDC_SimFiberHit ghit(*fiberhi);
    Identifier id = ghit.getID();
    photons_fiber = ghit.getNPhotons();
    side_fiber = (m_ZdcID->side(id)==-1) ? 0 : 1;
    mod_fiber = m_ZdcID->module(id);
    channel_fiber = m_ZdcID->channel(id);

    m_h_zdc_photons[side_fiber][mod_fiber]->Fill(photons_fiber);

    m_zdc_fiber_side->push_back(side_fiber);
    m_zdc_fiber_mod->push_back(mod_fiber);
    m_zdc_fiber_channel->push_back(channel_fiber);
    m_zdc_fiber_photons->push_back(photons_fiber);
  }


  m_zdc_calib_side->clear();
  m_zdc_calib_mod->clear();
  m_zdc_calib_channel->clear();
  m_zdc_calib_Total->clear();
  m_zdc_calib_EM->clear();
  m_zdc_calib_NonEM->clear();

  int side_calib = -1;
  int mod_calib = -1;
  int channel_calib = -1;
  float calib_eTot = -999.;
  float calib_eEM = -999.;
  float calib_eNonEM = -999.;
  
  const CaloCalibrationHitContainer* calibiter;
  CHECK(evtStore()->retrieve(calibiter,"ZDC_CalibrationHit"));
  for (auto hit : *calibiter) {
    Identifier id = hit->cellID();
    side_calib = (m_ZdcID->side(id)==-1) ? 0 : 1;
    mod_calib = m_ZdcID->module(id);
    channel_calib = m_ZdcID->channel(id);
    calib_eTot = hit->energyTotal();
    calib_eEM = hit->energyEM();
    calib_eNonEM = hit->energyNonEM();

    m_h_zdc_calibTot[side_calib][mod_calib]->Fill(calib_eTot);
    m_h_zdc_calibEM[side_calib][mod_calib]->Fill(calib_eEM);
    m_h_zdc_calibNonEM[side_calib][mod_calib]->Fill(calib_eNonEM);

    m_zdc_calib_side->push_back(side_calib);
    m_zdc_calib_mod->push_back(mod_calib);
    m_zdc_calib_channel->push_back(channel_calib);
    m_zdc_calib_Total->push_back(calib_eTot);
    m_zdc_calib_EM->push_back(calib_eEM);
    m_zdc_calib_NonEM->push_back(calib_eNonEM);
  }

  if (m_tree) m_tree->Fill();
  
  return StatusCode::SUCCESS;
}
