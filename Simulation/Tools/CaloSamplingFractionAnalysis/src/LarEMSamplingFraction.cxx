/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "LarEMSamplingFraction.h"

#include "CaloSimEvent/CaloCalibrationHit.h"
#include "CaloSimEvent/CaloCalibrationHitContainer.h"
#include "CaloEvent/CaloCellContainer.h"
#include "LArSimEvent/LArHitContainer.h"
#include "TileSimEvent/TileHitVector.h"

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"
#include "Gaudi/Property.h"

#include "GeneratorObjects/McEventCollection.h"

#include "TString.h"
#include <iterator>
#include <cmath>
#include <map>

using namespace std;

//###############################################################################
LarEMSamplingFraction::LarEMSamplingFraction(const std::string& name
					     , ISvcLocator* pSvcLocator)
  : AthAlgorithm(name, pSvcLocator)
{
  // Name of ClusterContainer to use
  declareProperty("DoCells",m_docells); 
  declareProperty("CalibrationHitContainerNames",m_CalibrationHitContainerNames); 
}

//###############################################################################

LarEMSamplingFraction::~LarEMSamplingFraction()
{ }

//###############################################################################

StatusCode LarEMSamplingFraction::initialize()
{
  //---- initialize the StoreGateSvc ptr ----------------
  
  ServiceHandle<ITHistSvc> histSvc("THistSvc",name()); 
  ATH_CHECK( histSvc.retrieve() );

  m_mytree = new TTree("mytree","mytree");
  m_mytree->Branch("energy_reco",          &m_energy_reco);
  m_mytree->Branch("energy_hit",           &m_energy_hit);
  m_mytree->Branch("energy_inactive_total",&m_energy_inactive_total);
  m_mytree->Branch("energy_inactive_em",   &m_energy_inactive_em);
  m_mytree->Branch("energy_inactive_nonem",&m_energy_inactive_nonem);
  m_mytree->Branch("energy_inactive_inv",  &m_energy_inactive_inv);
  m_mytree->Branch("energy_inactive_esc",  &m_energy_inactive_esc);
  m_mytree->Branch("energy_active_total_corrected",&m_energy_active_total_corrected);
  m_mytree->Branch("energy_active_total",&m_energy_active_total);
  m_mytree->Branch("energy_active_em",   &m_energy_active_em);
  m_mytree->Branch("energy_active_nonem",&m_energy_active_nonem);
  m_mytree->Branch("energy_active_inv",  &m_energy_active_inv);
  m_mytree->Branch("energy_active_esc",  &m_energy_active_esc);
  m_mytree->Branch("mc_pdg", &m_mc_pdg);
  m_mytree->Branch("mc_eta", &m_mc_eta);
  m_mytree->Branch("mc_phi", &m_mc_phi);
  m_mytree->Branch("mc_e",   &m_mc_e);
  m_mytree->Branch("mc_pt",  &m_mc_pt);
  
  if(m_docells) {
    m_mytree->Branch("cell_identifier",&m_cell_identifier);
    m_mytree->Branch("cell_energy_reco",&m_cell_energy_reco);
    m_mytree->Branch("cell_energy_inactive_total",&m_cell_energy_inactive_total);
    m_mytree->Branch("cell_energy_active_total_corrected",&m_cell_energy_active_total_corrected);
    m_mytree->Branch("cell_energy_active_total",&m_cell_energy_active_total);
    m_mytree->Branch("cell_sampling",&m_cell_sampling);
    m_mytree->Branch("cell_eta",&m_cell_eta);
    m_mytree->Branch("cell_phi",&m_cell_phi);
  }  

  histSvc->regTree("/MYSTREAM/myTree",m_mytree).ignore();

  // pointer to detector manager:
  ATH_CHECK(m_caloMgrKey.initialize());
  ATH_CHECK(detStore()->retrieve(m_calo_id, "CaloCell_ID"));
  
  const CaloIdManager* caloIdManager;
  ATH_CHECK(detStore()->retrieve(caloIdManager));

  m_tileID=caloIdManager->getTileID();
  if(m_tileID==0) throw std::runtime_error("ISF_HitAnalysis: Invalid Tile ID helper");

  ATH_CHECK(detStore()->retrieve(m_tileHWID));

  ATH_CHECK(m_fSamplKey.initialize());
  ATH_CHECK( m_tileSamplingFractionKey.initialize() );

  ATH_CHECK( m_tileCablingSvc.retrieve() );
  m_tileCabling = m_tileCablingSvc->cablingService();

  return StatusCode::SUCCESS;
}

//###############################################################################

StatusCode LarEMSamplingFraction::finalize()
{

  return StatusCode::SUCCESS;
}

//###############################################################################

StatusCode LarEMSamplingFraction::execute()
{
  SG::ReadCondHandle<ILArfSampl> fSamplHdl(m_fSamplKey);
  const ILArfSampl* fSampl=*fSamplHdl;

  SG::ReadCondHandle<TileSamplingFraction> tileSamplingFraction(m_tileSamplingFractionKey);
  ATH_CHECK( tileSamplingFraction.isValid() );

  const CaloCalibrationHitContainer* cchc;
  std::vector<const CaloCalibrationHitContainer *> v_cchc;
  for (const std::string& containerName : m_CalibrationHitContainerNames) {
    if ( !evtStore()->contains<CaloCalibrationHitContainer>(containerName))
    {
      ATH_MSG_ERROR("SG does not contain calibration hit container " << containerName);
      return StatusCode::FAILURE;
    }
    else
      {
	StatusCode sc = evtStore()->retrieve(cchc,containerName);
	if (sc.isFailure() )
	  {
	    ATH_MSG_ERROR("Cannot retrieve calibration hit container " << containerName);
	    return sc;
	  } 
	else
	  {
	    v_cchc.push_back(cchc);
	  }
      }
  }
	
  const McEventCollection* truthEvent{};
  StatusCode sc = evtStore()->retrieve(truthEvent, "TruthEvent");
  if (sc.isFailure()||!truthEvent)
    {
      ATH_MSG_ERROR("No McEventCollection found");
      return StatusCode::FAILURE;
    }
#ifdef HEPMC3
  const HepMC::ConstGenParticlePtr&  gen = truthEvent->at(0)->particles().front();
#else
  HepMC::ConstGenParticlePtr   gen  = *(truthEvent->at(0)->particles_begin());
#endif
  m_mc_pdg = gen->pdg_id();
  m_mc_eta = gen->momentum().pseudoRapidity();
  m_mc_phi = gen->momentum().phi();
  m_mc_e = gen->momentum().e();
  m_mc_pt = sqrt(pow(gen->momentum().px(),2)+pow(gen->momentum().py(),2));
  
  //inspiration:
  //see https://gitlab.cern.ch/atlas/athena/blob/master/Calorimeter/CaloCalibHitRec/src/CalibHitToCaloCell.cxx
  //and https://gitlab.cern.ch/atlas/athena/blob/master/Calorimeter/CaloCalibHitRec/src/CaloDmEnergy.cxx
  
  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey};
  ATH_CHECK(caloMgrHandle.isValid());
  const CaloDetDescrManager* caloMgr = *caloMgrHandle;
	
  m_energy_reco          =new vector<float>;
  m_energy_hit          =new vector<float>;
  
  m_energy_inactive_total=new vector<float>;
  m_energy_inactive_em   =new vector<float>;
  m_energy_inactive_nonem=new vector<float>;
  m_energy_inactive_inv  =new vector<float>;
  m_energy_inactive_esc  =new vector<float>;
  
  m_energy_active_total_corrected=new vector<float>;
  m_energy_active_total=new vector<float>;
  m_energy_active_em   =new vector<float>;
  m_energy_active_nonem=new vector<float>;
  m_energy_active_inv  =new vector<float>;
  m_energy_active_esc  =new vector<float>;
  
  if(m_docells) {
    m_cell_identifier                   =new std::vector<Long64_t>;
    m_cell_energy_reco                  =new std::vector<float>;
    m_cell_energy_active_total_corrected=new std::vector<float>;
    m_cell_energy_active_total          =new std::vector<float>;
    m_cell_energy_inactive_total        =new std::vector<float>;
    m_cell_sampling                     =new std::vector<int>;
    m_cell_eta                          =new std::vector<float>;
    m_cell_phi                          =new std::vector<float>;
  } 

  struct cell_info {
    Long64_t cell_identifier=0;
    float    cell_energy_reco=0;
    float    cell_energy_active_total_corrected=0;
    float    cell_energy_active_total=0;
    float    cell_energy_inactive_total=0;
    int      cell_sampling=0;
    float    cell_eta=0;
    float    cell_phi=0;
  };
    
  std::map< Long64_t , cell_info > cell_info_map;
  
  for(int s=0;s<24;s++)
    {
      m_energy_reco->push_back(0.0);
      m_energy_hit->push_back(0.0);
      
      m_energy_inactive_total->push_back(0.0);
      m_energy_inactive_em   ->push_back(0.0);
      m_energy_inactive_nonem->push_back(0.0);
      m_energy_inactive_inv  ->push_back(0.0);
      m_energy_inactive_esc  ->push_back(0.0);
      
      m_energy_active_total_corrected->push_back(0.0);
      m_energy_active_total->push_back(0.0);
      m_energy_active_em   ->push_back(0.0);
      m_energy_active_nonem->push_back(0.0);
      m_energy_active_inv  ->push_back(0.0);
      m_energy_active_esc  ->push_back(0.0);
    }
  
  int count=0;
  for (const CaloCalibrationHitContainer* calibHitContainer: v_cchc)
    {
      ATH_MSG_DEBUG( "loop on "<<m_CalibrationHitContainerNames[count]);
      for(const CaloCalibrationHit* calibHit : *calibHitContainer)
	{
	  Identifier id=calibHit->cellID();
	  
	  double Etot   = calibHit->energyTotal();
	  double Eem    = calibHit->energyEM();
	  double Enonem = calibHit->energyNonEM();
	  double Einv   = calibHit->energyInvisible();
	  double Eesc   = calibHit->energyEscaped();
	  
	  double Efactor=1.0;
	    
	  const CaloDetDescrElement* caloDDE = caloMgr->get_element(id);
	  int sampling=-1;
	  if(caloDDE) {
	    sampling = caloDDE->getSampling();
	    
	    if((sampling>=0 && sampling<=11) || (sampling>=21 && sampling<=23)) Efactor=1/fSampl->FSAMPL(id);
	    if((sampling>=12 && sampling<=20)) {
        HWIdentifier channel_id = m_tileCabling->s2h_channel_id(id);
        int channel = m_tileHWID->channel(channel_id);
        int drawerIdx = m_tileHWID->drawerIdx(channel_id);
        Efactor = tileSamplingFraction->getSamplingFraction(drawerIdx, channel);
	      Identifier cell_id = m_tileID->cell_id(id);
	      if(caloMgr->get_element(cell_id)) {
          id=cell_id;
	      }
	    }  
	  }  
	  
	  ATH_MSG_VERBOSE( "cellID "<<id<<" layer "<<sampling<<" energyTotal "<<Etot<<" Eem "<<Eem<<" Enonem "<<Enonem<<" Einv "<<Einv<<" Eesc "<<Eesc<<" Efactor="<<Efactor);
	  
	  if(sampling>=0 && sampling<=23)
	    {
	      if(m_docells) {
		cell_info_map[id.get_compact()].cell_identifier=id.get_compact();
		cell_info_map[id.get_compact()].cell_sampling=sampling;
		cell_info_map[id.get_compact()].cell_eta=caloDDE->eta_raw();
		cell_info_map[id.get_compact()].cell_phi=caloDDE->phi_raw();
	      }  

	      if(m_CalibrationHitContainerNames[count]=="LArCalibrationHitInactive" || m_CalibrationHitContainerNames[count]=="TileCalibHitInactiveCell")
		{
		  m_energy_inactive_total->at(sampling)+=Etot;
		    m_energy_inactive_em   ->at(sampling)+=Eem;
		    m_energy_inactive_nonem->at(sampling)+=Enonem;
		    m_energy_inactive_inv  ->at(sampling)+=Einv;
		    m_energy_inactive_esc  ->at(sampling)+=Eesc;
		    
		    if(m_docells) cell_info_map[id.get_compact()].cell_energy_inactive_total+=Etot;
		}

	      if(m_CalibrationHitContainerNames[count]=="LArCalibrationHitActive" || m_CalibrationHitContainerNames[count]=="TileCalibHitActiveCell")
		{
		  m_energy_active_total_corrected->at(sampling)+=Etot*Efactor;
		  m_energy_active_total->at(sampling)+=Etot;
		  m_energy_active_em   ->at(sampling)+=Eem;
		  m_energy_active_nonem->at(sampling)+=Enonem;
		  m_energy_active_inv  ->at(sampling)+=Einv;
		  m_energy_active_esc  ->at(sampling)+=Eesc;
		  
		  if(m_docells) {
		    cell_info_map[id.get_compact()].cell_energy_active_total_corrected+=Etot*Efactor;
		    cell_info_map[id.get_compact()].cell_energy_active_total+=Etot;
		  }  
		}
	      
	    }
	  
	}
      
      count++;
    }
  
  //Get reco cells if available
  const CaloCellContainer *cellColl{};
  sc = evtStore()->retrieve(cellColl, "AllCalo");
  
  if (sc.isFailure()) {
    ATH_MSG_WARNING( "Couldn't read AllCalo cells from StoreGate");
    //return NULL;
  } else {
    ATH_MSG_DEBUG( "Found: "<<cellColl->size()<<" calorimeter cells");
    for (const CaloCell* cell : *cellColl) {
      Identifier id=cell->ID();
      const CaloDetDescrElement* caloDDE = caloMgr->get_element(id);
      int sampling=-1;
      if(caloDDE) {
	sampling = caloDDE->getSampling();
	m_energy_reco->at(sampling)+=cell->energy();
	if((sampling>=12 && sampling<=20)) {
	  Identifier cell_id = m_tileID->cell_id(id);
	  if(caloMgr->get_element(cell_id)) {
            id=cell_id;
	  }  
	}  
      } 
      if(m_docells) {
	cell_info_map[id.get_compact()].cell_identifier=id.get_compact();
	cell_info_map[id.get_compact()].cell_sampling=sampling;
	cell_info_map[id.get_compact()].cell_eta=caloDDE->eta_raw();
	cell_info_map[id.get_compact()].cell_phi=caloDDE->phi_raw();
	cell_info_map[id.get_compact()].cell_energy_reco+=cell->energy();
      }  
    }
  } //calorimeter cells
  
  
  //Get all G4Hits (from CaloHitAnalysis)
  const std::vector<std::string>  lArKeys = {"LArHitEMB", "LArHitEMEC", "LArHitFCAL", "LArHitHEC"};
  for (const std::string& containerName: lArKeys) {
    const LArHitContainer* larContainer{};
    ATH_MSG_DEBUG( "Checking G4Hits: "<<containerName);
    if(evtStore()->retrieve(larContainer,containerName)==StatusCode::SUCCESS) {
      int hitnumber = 0;
      for (const LArHit* larHit : *larContainer) {
	hitnumber++;
	const CaloDetDescrElement *hitElement = caloMgr->get_element(larHit->cellID());
	if(!hitElement) continue;
	Identifier larhitid = hitElement->identify();
	if(caloMgr->get_element(larhitid)) {
	  CaloCell_ID::CaloSample larlayer = caloMgr->get_element(larhitid)->getSampling();
	  m_energy_hit->at(larlayer)+=larHit->energy();
	}
      } // End while LAr hits
      ATH_MSG_DEBUG( "Read "<<hitnumber<<" G4Hits from "<<containerName);
    }
    else {
      ATH_MSG_INFO( "Can't retrieve LAr hits");
    }// End statuscode success upon retrieval of hits
  }// End detector type loop
  
  const TileHitVector * hitVec{};
  if (evtStore()->retrieve(hitVec,"TileHitVec")==StatusCode::SUCCESS &&  m_tileID ) {
    int hitnumber = 0;
    for(const TileHit& hit : *hitVec) {
      ++hitnumber;
      Identifier pmt_id = hit.identify();
      Identifier cell_id = m_tileID->cell_id(pmt_id);
      
      if (caloMgr->get_element(cell_id)) {
	CaloCell_ID::CaloSample layer = caloMgr->get_element(cell_id)->getSampling();
	
	//could there be more subhits??
	for (int tilesubhit_i = 0; tilesubhit_i<hit.size(); tilesubhit_i++) {
	  //!!
	  ATH_MSG_DEBUG( "Tile subhit: "<<tilesubhit_i<<"/"<<hit.size()<< " E: "<<hit.energy(tilesubhit_i) );
	  m_energy_hit->at(layer) += hit.energy(tilesubhit_i);
	}
      }
    }
    ATH_MSG_DEBUG( "Read "<<hitnumber<<" G4Hits from TileHitVec");
  }
    
  for(auto& cell:cell_info_map) {
    m_cell_identifier                   ->push_back(cell.second.cell_identifier);
    m_cell_sampling                     ->push_back(cell.second.cell_sampling);
    m_cell_eta                          ->push_back(cell.second.cell_eta);
    m_cell_phi                          ->push_back(cell.second.cell_phi);
    m_cell_energy_reco                  ->push_back(cell.second.cell_energy_reco);
    m_cell_energy_active_total_corrected->push_back(cell.second.cell_energy_active_total_corrected);
    m_cell_energy_active_total          ->push_back(cell.second.cell_energy_active_total);
    m_cell_energy_inactive_total        ->push_back(cell.second.cell_energy_inactive_total);
  }
    
  m_mytree->Fill();
  
  delete m_energy_reco;
  delete m_energy_hit;
  delete m_energy_inactive_total;
  delete m_energy_inactive_em;
  delete m_energy_inactive_nonem;
  delete m_energy_inactive_inv;
  delete m_energy_inactive_esc;
  
  delete m_energy_active_total_corrected;
  delete m_energy_active_total;
  delete m_energy_active_em;
  delete m_energy_active_nonem;
  delete m_energy_active_inv;
  delete m_energy_active_esc;
  
  if(m_docells) {
    delete m_cell_identifier;
    delete m_cell_energy_reco;
    delete m_cell_energy_active_total_corrected;
    delete m_cell_energy_active_total;
    delete m_cell_energy_inactive_total;
    delete m_cell_sampling;
    delete m_cell_eta;
    delete m_cell_phi;
  }  
  
  return StatusCode::SUCCESS;
}
