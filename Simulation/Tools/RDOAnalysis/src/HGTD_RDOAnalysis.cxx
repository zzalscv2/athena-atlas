/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "HGTD_RDOAnalysis.h"
#include "StoreGate/ReadHandle.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "ReadoutGeometryBase/SiLocalPosition.h"

#include "TruthUtils/AtlasPID.h"
#include "TruthUtils/HepMCHelpers.h"

#include "TTree.h"
#include "TString.h"

#include <algorithm>
#include <math.h>
#include <functional>
#include <iostream>


HGTD_RDOAnalysis::HGTD_RDOAnalysis(const std::string& name, ISvcLocator *pSvcLocator)
  : AthAlgorithm(name, pSvcLocator)
{
}

StatusCode HGTD_RDOAnalysis::initialize() {
  ATH_MSG_DEBUG( "Initializing HGTD_RDOAnalysis" );

  // This will check that the properties were initialized
  // properly by job configuration.
  ATH_CHECK( m_inputKey_HGTD.initialize() );
  ATH_CHECK( m_inputTruthKey_HGTD.initialize() );
  ATH_CHECK( m_inputMcEventCollectionKey.initialize() );

  // Grab HGTDID helper
  ATH_CHECK(detStore()->retrieve(m_HGTD_Manager, m_HGTD_Name.value()));
  ATH_CHECK(detStore()->retrieve(m_HGTD_ID, m_HGTDID_Name.value()));

  // Grab Ntuple and histogramming service for tree
  ATH_CHECK(m_thistSvc.retrieve());

  m_tree = new TTree(m_ntupleName.value().c_str(), "HGTD_RDOAnalysis");
  ATH_CHECK(m_thistSvc->regTree(m_ntuplePath.value() + m_ntupleName.value(), m_tree));
  if (m_tree) {
    m_tree->Branch("m_rdo_hit_module_layer", &m_rdo_hit_module_layer);
    m_tree->Branch("m_rdo_hit_module_x", &m_rdo_hit_module_x);
    m_tree->Branch("m_rdo_hit_module_y", &m_rdo_hit_module_y);
    m_tree->Branch("m_rdo_hit_module_z", &m_rdo_hit_module_z);
    m_tree->Branch("m_rdo_hit_x", &m_rdo_hit_x);
    m_tree->Branch("m_rdo_hit_y", &m_rdo_hit_y);
    m_tree->Branch("m_rdo_hit_z", &m_rdo_hit_z);
    m_tree->Branch("m_rdo_hit_raw_time", &m_rdo_hit_raw_time);
    m_tree->Branch("m_rdo_hit_sdoraw_time", &m_rdo_hit_sdoraw_time);
    m_tree->Branch("m_rdo_hit_bcid", &m_rdo_hit_bcid);
    m_tree->Branch("m_rdo_hit_truth", &m_rdo_hit_truth);
  }
  else {
    ATH_MSG_ERROR("No tree found!");
  }
  /*
  // HISTOGRAMS

  /// global histograms
  m_h_rdoID = new TH1F("h_rdoID", "rdoID", 100, 0, 10e17);
  m_h_rdoID->StatOverflows();
  ATH_CHECK(m_thistSvc->regHist(m_histPath + m_h_rdoID->GetName(), m_h_rdoID));
  */

  return StatusCode::SUCCESS;
}

StatusCode HGTD_RDOAnalysis::execute() {
  ATH_MSG_DEBUG(" In HGTD_RDOAnalysis::execute()" );
  m_rdo_hit_module_layer->clear();
  m_rdo_hit_module_x->clear();
  m_rdo_hit_module_y->clear();
  m_rdo_hit_module_z->clear();
  m_rdo_hit_x->clear();
  m_rdo_hit_y->clear();
  m_rdo_hit_z->clear();
  m_rdo_hit_raw_time->clear();
  m_rdo_hit_sdoraw_time->clear();
  m_rdo_hit_bcid->clear();
  m_rdo_hit_truth->clear();

  // Raw HGTD Data
  SG::ReadHandle<HGTD_RDO_Container> p_HGTD_RDO_cont (m_inputKey_HGTD);
  //Adding SimMap and McEvent here for added truthMatching checks
  SG::ReadHandle<InDetSimDataCollection> simDataMapHGTD (m_inputTruthKey_HGTD);
  
  SG::ReadHandle<McEventCollection> mcEventCollection (m_inputMcEventCollectionKey);
  bool doTruthMatching = true;
  const HepMC::GenEvent* hardScatterEvent(nullptr);

  if (mcEventCollection->size()==0){
    ATH_MSG_WARNING("Failed to retrieve a nonzero sized truth event collection, disabling truthMatching");
    doTruthMatching = false;
  }
  if(doTruthMatching) hardScatterEvent = mcEventCollection->at(0);

  if(p_HGTD_RDO_cont.isValid()) {
    // loop over RDO container
    for ( HGTD_RDO_Container::const_iterator hgtd_rdoCont_itr =  p_HGTD_RDO_cont->begin(); hgtd_rdoCont_itr != p_HGTD_RDO_cont->end(); ++hgtd_rdoCont_itr ) {

      const HGTD_RDO_Collection* p_HGTD_RDO_coll(*hgtd_rdoCont_itr);
      const Identifier HGTD_rdoIDColl(p_HGTD_RDO_coll->identify());
      const InDetDD::HGTD_DetectorElement *element = m_HGTD_Manager->getDetectorElement(HGTD_rdoIDColl);
      InDetDD::SiLocalPosition localPos = element->rawLocalPositionOfCell(HGTD_rdoIDColl);
      Amg::Vector3D globalPos = element->globalPosition(localPos);

      m_rdo_hit_module_x->push_back(globalPos[Amg::x]);
      m_rdo_hit_module_y->push_back(globalPos[Amg::y]);
      m_rdo_hit_module_z->push_back(globalPos[Amg::z]);
      m_rdo_hit_module_layer->push_back(m_HGTD_ID->layer(HGTD_rdoIDColl));

      for ( HGTD_RDO_Collection::const_iterator hgtd_rdo_itr= p_HGTD_RDO_coll->begin(); hgtd_rdo_itr != p_HGTD_RDO_coll->end(); ++hgtd_rdo_itr ) {

        m_rdo_hit_bcid->push_back((*hgtd_rdo_itr)->getBCID());
        const Identifier HGTD_rdoID((*hgtd_rdo_itr)->identify());
        
        const InDetDD::HGTD_DetectorElement *rdo_element = m_HGTD_Manager->getDetectorElement(HGTD_rdoID);
        InDetDD::SiLocalPosition localPos_hit = rdo_element->rawLocalPositionOfCell(HGTD_rdoID);
        Amg::Vector3D globalPos_hit = rdo_element->globalPosition(localPos_hit);

        m_rdo_hit_x->push_back(globalPos_hit[Amg::x]);
		    m_rdo_hit_y->push_back(globalPos_hit[Amg::y]);
        m_rdo_hit_z->push_back(globalPos_hit[Amg::z]);

        // float pixelRadius = sqrt(globalPos[Amg::x]*globalPos[Amg::x]+globalPos[Amg::y]*globalPos[Amg::y]);

        if(doTruthMatching){
          if(simDataMapHGTD.isValid()){
            InDetSimDataCollection::const_iterator iter = (*simDataMapHGTD).find((*hgtd_rdo_itr)->identify());
            std::vector<float> sdo_times;
            std::vector<SdoInfo> sdo_info;
            if ( iter != (*simDataMapHGTD).end() ) {
              const InDetSimData& sdo = iter->second;
              const std::vector< InDetSimData::Deposit >& deposits = sdo.getdeposits();

              for( std::vector< InDetSimData::Deposit >::const_iterator nextdeposit = deposits.begin() ; nextdeposit!=deposits.end(); ++nextdeposit) {

                sdo_times.push_back(nextdeposit->second);

	              const HepMcParticleLink& particleLink = nextdeposit->first;
                if(particleLink.isValid()){
                  HepMC::ConstGenParticlePtr genPart(particleLink.cptr());
                  if(IsGoodParticle(genPart)){
                    if(genPart->parent_event() == hardScatterEvent){
                      SdoInfo sdoi;
                      sdoi.time = nextdeposit->second;
                      sdoi.truth = 0;
                      sdo_info.push_back(sdoi);
                    }
                    else {
                      SdoInfo sdoi;
                      sdoi.time = nextdeposit->second;
                      sdoi.truth = 1;
                      sdo_info.push_back(sdoi);
                    }
                  }
                }
              }
            }
            std::sort(sdo_times.begin(), sdo_times.end());
            std::sort(sdo_info.begin(),  sdo_info.end());
            if (sdo_times.size() > 0) 
              m_rdo_hit_sdoraw_time->push_back(sdo_times.at(0));
            if (sdo_info.size() > 0) {
              m_rdo_hit_raw_time->push_back(sdo_info.at(0).time);
              m_rdo_hit_truth->push_back(sdo_info.at(0).truth);
            }
          }
        }
      }
    }
  }

  if (m_tree) m_tree->Fill();
  
  return StatusCode::SUCCESS;
}

bool HGTD_RDOAnalysis::IsGoodParticle(HepMC3::ConstGenParticlePtr particlePtr, float min_pt_cut) {
  bool decision = false;
  if( MC::isGenStable(particlePtr) and
      isCharged(particlePtr) and
      particlePtr->momentum().perp() >= min_pt_cut and
      particlePtr->momentum().eta() < 4.)
  decision = true;

  return decision;
}