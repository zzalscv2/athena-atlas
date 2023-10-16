/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "HGTD_RDOAnalysis.h"
#include "StoreGate/ReadHandle.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "ReadoutGeometryBase/SiLocalPosition.h"

#include "TruthUtils/AtlasPID.h"
#include "TruthUtils/HepMCHelpers.h"
#include "GaudiKernel/PhysicalConstants.h"

#include "TTree.h"
#include "TString.h"

#include <algorithm>
#include <math.h>
#include <functional>
#include <iostream>


HGTD_RDOAnalysis::HGTD_RDOAnalysis(const std::string& name, ISvcLocator *pSvcLocator)
  : AthAlgorithm(name, pSvcLocator) { }

StatusCode HGTD_RDOAnalysis::initialize() {
  ATH_MSG_DEBUG( "Initializing HGTD_RDOAnalysis" );

  // This will check that the properties were initialized
  // properly by job configuration.
  ATH_CHECK( m_inputKey.initialize() );
  ATH_CHECK( m_inputTruthKey.initialize() );
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
    m_tree->Branch("m_rdo_hit_toa", &m_rdo_hit_toa);
    m_tree->Branch("m_rdo_hit_sdo_deposit_time", &m_rdo_hit_sdo_deposit_time);
    m_tree->Branch("m_rdo_hit_truth", &m_rdo_hit_truth);
  } else {
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
  m_rdo_hit_module_layer.clear();
  m_rdo_hit_module_x.clear();
  m_rdo_hit_module_y.clear();
  m_rdo_hit_module_z.clear();
  m_rdo_hit_x.clear();
  m_rdo_hit_y.clear();
  m_rdo_hit_z.clear();
  m_rdo_hit_toa.clear();
  m_rdo_hit_sdo_deposit_time.clear();
  m_rdo_hit_truth.clear();

  // Raw HGTD Data
  SG::ReadHandle<HGTD_RDO_Container> p_RDO_cont (m_inputKey);
  //Adding SimMap and McEvent here for added truthMatching checks
  SG::ReadHandle<InDetSimDataCollection> simDataMap (m_inputTruthKey);
  
  
  SG::ReadHandle<McEventCollection> mcEventCollection (m_inputMcEventCollectionKey);
  bool doTruthMatching = true;
  const HepMC::GenEvent* hardScatterEvent(nullptr);

  if (mcEventCollection->size()==0){
    ATH_MSG_WARNING("Failed to retrieve a nonzero sized truth event collection, disabling truthMatching");
    doTruthMatching = false;
  }
  if(doTruthMatching) hardScatterEvent = mcEventCollection->at(0);

  if(p_RDO_cont.isValid()) {
    // loop over RDO container
    for ( HGTD_RDO_Container::const_iterator rdoCont_itr =  p_RDO_cont->begin(); rdoCont_itr != p_RDO_cont->end(); ++rdoCont_itr ) {

      const HGTD_RDO_Collection* p_RDO_coll(*rdoCont_itr);
      const Identifier rdoIDColl((*rdoCont_itr)->identify());
        
      const InDetDD::HGTD_DetectorElement *elementColl = m_HGTD_Manager->getDetectorElement(rdoIDColl);
      InDetDD::SiLocalPosition localPosColl = elementColl->rawLocalPositionOfCell(rdoIDColl);
      Amg::Vector3D globalPosColl = elementColl->globalPosition(localPosColl);

      m_rdo_hit_module_x.push_back(globalPosColl[Amg::x]);
      m_rdo_hit_module_y.push_back(globalPosColl[Amg::y]);
      m_rdo_hit_module_z.push_back(globalPosColl[Amg::z]);

      m_rdo_hit_module_layer.push_back(m_HGTD_ID->layer(rdoIDColl));

      for ( HGTD_RDO_Collection::const_iterator rdo_itr= p_RDO_coll->begin(); rdo_itr != p_RDO_coll->end(); ++rdo_itr ) {

        const Identifier rdoID((*rdo_itr)->identify());
        
        const InDetDD::HGTD_DetectorElement *rdo_element = m_HGTD_Manager->getDetectorElement(rdoID);
        InDetDD::SiLocalPosition localPos_hit = rdo_element->rawLocalPositionOfCell(rdoID);
        Amg::Vector3D globalPos_hit = rdo_element->globalPosition(localPos_hit);

        m_rdo_hit_x.push_back(globalPos_hit[Amg::x]);
		    m_rdo_hit_y.push_back(globalPos_hit[Amg::y]);
        m_rdo_hit_z.push_back(globalPos_hit[Amg::z]);

        // Get the Time of Arrival which include the tof and the digitisation smearing of 25 ps
        float rdo_toa = (*rdo_itr)->getTOA(); 
        m_rdo_hit_toa.push_back(rdo_toa);

        // float pixelRadius = sqrt(globalPos[Amg::x]*globalPos[Amg::x]+globalPos[Amg::y]*globalPos[Amg::y]);
        // For truth matching studies we need to get the truth nature of each deposit.
        // For each hit, there can be up to 7 deposit from different nature accessed through the SDO map.
        if(doTruthMatching){
          if(simDataMap.isValid()){
            InDetSimDataCollection::const_iterator iter = (*simDataMap).find((*rdo_itr)->identify());
            std::vector<SdoInfo> sdo_info;
            if ( iter != (*simDataMap).end() ) {
              const InDetSimData& sdo = iter->second;
              const std::vector< InDetSimData::Deposit >& deposits = sdo.getdeposits();
              for( std::vector< InDetSimData::Deposit >::const_iterator nextdeposit = deposits.begin() ; nextdeposit!=deposits.end(); ++nextdeposit) {
                SdoInfo sdoi;
                // The times of the deposit also include the smearing of digi of 25 ps
                sdoi.time = nextdeposit->second / Gaudi::Units::c_light; //need to correct from mm to ns.
                const HepMcParticleLink& particleLink = nextdeposit->first;
                if(particleLink.isValid()){
                  HepMC::ConstGenParticlePtr genPart(particleLink.cptr());
                  if(IsHSGoodParticle(genPart,hardScatterEvent)) sdoi.truth = 1; // signal
                  else sdoi.truth = 2; // pile-up
                }
                sdo_info.push_back(sdoi);
              }
            } else {
              // Secondaries are not saved in the SDO map, hence saving the TOA for them
              SdoInfo sdoi;
              sdoi.time = rdo_toa;
              sdoi.truth = 3;
              sdo_info.push_back(sdoi);
            }

            // Only the particle that creates the first deposit is considered to be the correct particle
            std::sort(sdo_info.begin(), sdo_info.end());
            if (sdo_info.size() > 0) {
              m_rdo_hit_sdo_deposit_time.push_back(sdo_info.at(0).time);
              m_rdo_hit_truth.push_back(sdo_info.at(0).truth);
            }
          }
          else {
            ATH_MSG_ERROR("No HGTD SDO map found!");
          }
        }
      }
    }
  }
  else {
    ATH_MSG_ERROR("No HGTD RDO container found!");
  }

  if (m_tree) m_tree->Fill();
  
  return StatusCode::SUCCESS;
}

bool HGTD_RDOAnalysis::IsHSGoodParticle(HepMC3::ConstGenParticlePtr particlePtr,const HepMC::GenEvent* hardScatterGenEvent, float min_pt_cut) {
  bool decision = false;
  if( MC::isGenStable(particlePtr) and
      isCharged(particlePtr) and
      particlePtr->momentum().perp() >= min_pt_cut and
      particlePtr->momentum().eta() < 4. and
      particlePtr->parent_event() == hardScatterGenEvent)
  decision = true;

  return decision;
}
