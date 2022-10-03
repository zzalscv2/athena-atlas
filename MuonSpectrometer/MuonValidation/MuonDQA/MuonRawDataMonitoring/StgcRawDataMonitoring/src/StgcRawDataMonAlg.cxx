/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Package : sTgcRawDataMonAlg
// Author: Sebastian Fuenzalida Garrido
// Local supervisor: Edson Carquin Lopez
// Technical supervisor: Gerardo Vasquez
//
// DESCRIPTION:
// Subject: sTgc --> sTgc raw data monitoring
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include "StgcRawDataMonitoring/StgcRawDataMonAlg.h"

/////////////////////////////////////////////////////////////////////////////
// *********************************************************************
// Public Methods
// ********************************************************************* 
/////////////////////////////////////////////////////////////////////////////

sTgcRawDataMonAlg::sTgcRawDataMonAlg( const std::string& name, ISvcLocator* pSvcLocator ) : AthMonitorAlgorithm(name,pSvcLocator)	      
{
  //Declare the property 
}


StatusCode sTgcRawDataMonAlg::initialize()
{   
  ATH_CHECK(AthMonitorAlgorithm::initialize());
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(m_sTgcContainerKey.initialize());

  ATH_MSG_INFO("sTGCRawDataMonAlg Successfuly initialized");
  
  return StatusCode::SUCCESS;
} 

StatusCode sTgcRawDataMonAlg::fillHistograms(const EventContext& ctx) const
{  
  SG::ReadHandle<Muon::sTgcPrepDataContainer> sTgc_container(m_sTgcContainerKey, ctx);
  ATH_CHECK(sTgc_container.isValid());

  if (m_dosTgcESD && m_dosTgcOverview)  
    {
      for(const Muon::sTgcPrepDataCollection* coll : *sTgc_container)
	{	  
	  for (const Muon::sTgcPrepData* prd : *coll)
	    {
	      fillsTgcOverviewHistograms(prd, *coll);
	      fillsTgcSummaryHistograms(prd);
	    }
	}
    }
     
  return StatusCode::SUCCESS;
}

void sTgcRawDataMonAlg::fillsTgcOverviewHistograms(const Muon::sTgcPrepData *sTgc_object, const Muon::MuonPrepDataCollection<Muon::sTgcPrepData> &prd) const 
{   
  auto charge_all = Monitored::Collection("charge_all", prd, [] (const Muon::sTgcPrepData *aux) 
					  {
					    return aux -> charge();
					  });
  
  auto numberofstrips_percluster = Monitored::Collection("numberofstrips_percluster", prd, [] (const Muon::sTgcPrepData *aux) 
							 {
							   const std::vector<Identifier> &stripIds = aux -> rdoList(); 
							   return stripIds.size();
							 });
  
  fill("sTgcMonitor", charge_all, numberofstrips_percluster);
  
  std::vector<short int> strip_times_target = sTgc_object-> stripTimes();
  std::vector<int> strip_charges_target = sTgc_object-> stripCharges();
  std::vector<short unsigned int> strip_number_target = sTgc_object-> stripNumbers();

  auto time_all = Monitored::Collection("time_all", prd, [] (const Muon::sTgcPrepData *aux) 
					{
					  return aux -> time();
					});
      
  auto strip_times = Monitored::Collection("strip_times", strip_times_target);
  auto strip_charges = Monitored::Collection("strip_charges", strip_charges_target);
  auto strip_number = Monitored::Collection("strip_number", strip_number_target);

  fill("sTgcMonitor", time_all, strip_times, strip_charges, strip_number);

  auto x_mon = Monitored::Collection("x_mon", prd, [] (const Muon::sTgcPrepData *aux) 
				     {
				       Amg::Vector3D pos = aux -> globalPosition(); 
				       return pos.x();
				     });
  
  auto y_mon = Monitored::Collection("y_mon", prd, [] (const Muon::sTgcPrepData *aux) 
				     {
				       Amg::Vector3D pos = aux -> globalPosition(); 
				       return pos.y();
				     });
  
  auto z_mon = Monitored::Collection("z_mon", prd, [] (const Muon::sTgcPrepData *aux) 
				     {
				       Amg::Vector3D pos = aux -> globalPosition(); 
				       return pos.z();
				     });
  
  auto R_mon = Monitored::Collection("R_mon", prd, [] (const Muon::sTgcPrepData *aux) 
				     {
				       Amg::Vector3D pos = aux -> globalPosition(); 
				       return std::hypot(pos.x(), pos.y());
				     });

  fill("sTgcMonitor", x_mon, y_mon, z_mon, R_mon);
}

void sTgcRawDataMonAlg::fillsTgcSummaryHistograms(const Muon::sTgcPrepData *sTgc_object) const
{
  Identifier Id    = sTgc_object   -> identify();
  if(!Id.is_valid()) {
    ATH_MSG_DEBUG("Invalid identifier found in Muon::sTgcPrepData");
    return;
  }

  int stationPhi   = m_idHelperSvc -> stgcIdHelper().stationPhi(Id);
  int stationEta   = m_idHelperSvc -> stgcIdHelper().stationEta(Id);
  int iside        = (stationEta > 0) ? 1 : 0;
  int multiplet    = m_idHelperSvc -> stgcIdHelper().multilayer(Id);
  int gasgap       = m_idHelperSvc -> stgcIdHelper().gasGap(Id);  

  int channel_type = m_idHelperSvc -> stgcIdHelper().channelType(Id);

  std::string stationName = m_idHelperSvc -> stgcIdHelper().stationNameString(m_idHelperSvc -> stgcIdHelper().stationName(Id));
  int stationPhiComplete = get_sectorPhi_from_stationPhi_stName(stationPhi, stationName);
 
  std::vector<int> strip_charges_vec = sTgc_object -> stripCharges();  
  std::vector<short unsigned int> strip_numbers_perPhi_vec = sTgc_object -> stripNumbers();

  std::vector<int> charge, stationPhi_vec, stationEta_vec;
  charge.push_back(sTgc_object->charge());
  stationPhi_vec.push_back(stationPhi);
  stationEta_vec.push_back(stationEta);

  std::string monGroupName = "sTgc_sideGroup" + GeometricSectors::sTgc_Side[iside];
  std::string baseName = "_" + GeometricSectors::sTgc_Side[iside] + "_multiplet_" + std::to_string(multiplet) + "_gasgap_" + std::to_string(gasgap);
  std::string sPhiName = "_stationPhi_" + std::to_string(stationPhiComplete);

  std::string varName  = "";
  
  if (channel_type == 0)
    {

      varName = "pad_charge"+baseName;
      auto charge_perLayer = Monitored::Collection(varName, charge);

      varName = "pad_phi"+baseName;
      auto stationPhi = Monitored::Collection(varName, stationPhi_vec);

      varName = "pad_eta"+baseName;
      auto stationEta = Monitored::Collection(varName, stationEta_vec);

      fill(monGroupName, charge_perLayer, stationPhi, stationEta);    
    }
  
  else if (channel_type == 1)
    {      

      varName = "strip_charge"+baseName;
      auto charge_perLayer = Monitored::Collection(varName, charge);

      varName = "strip_phi"+baseName; 
      auto stationPhi = Monitored::Collection(varName, stationPhi_vec);

      varName = "strip_eta"+baseName;
      auto stationEta = Monitored::Collection(varName, stationEta_vec);
      
      varName = "strip_eta"+baseName+sPhiName;
      auto stationEta_perPhi = Monitored::Collection(varName, stationEta_vec);

      varName = "strip_number"+baseName+sPhiName;
      auto stripNumber_perLayer_perPhi = Monitored::Collection(varName, strip_numbers_perPhi_vec);

      varName = "strip_charge"+baseName+sPhiName;
      auto charge_perLayer_perPhi = Monitored::Collection(varName, strip_charges_vec);

      fill(monGroupName, charge_perLayer, stationPhi, stationEta, stationEta_perPhi, stripNumber_perLayer_perPhi, charge_perLayer_perPhi);    
    }
  
  else if (channel_type == 2)
    {
      varName = "wire_charge"+baseName;
      auto charge_perLayer = Monitored::Collection(varName, charge);

      varName = "wire_phi"+baseName;
      auto stationPhi = Monitored::Collection(varName, stationPhi_vec);

      varName = "wire_eta"+baseName;
      auto stationEta = Monitored::Collection(varName, stationEta_vec);

      fill(monGroupName, charge_perLayer, stationPhi, stationEta);    
    }
}

