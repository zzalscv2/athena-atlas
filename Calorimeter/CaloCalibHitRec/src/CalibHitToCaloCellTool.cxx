/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloCalibHitRec/CalibHitToCaloCellTool.h"


// Calo include
#include "CaloIdentifier/CaloDM_ID.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloIdentifier/CaloGain.h"
#include "CaloSimEvent/CaloCalibrationHitContainer.h"  
#include "CaloEvent/CaloCellContainer.h"
#include "CaloDetDescr/CaloDetDescrElement.h"

#include "CaloUtils/CaloClusterStoreHelper.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODCaloEvent/CaloClusterKineHelper.h"
#include "CaloEvent/CaloClusterCellLinkContainer.h"

#include "LArRecEvent/LArCell.h"
#include "TileEvent/TileCell.h"
#include "TruthUtils/HepMCHelpers.h"

CalibHitToCaloCellTool::CalibHitToCaloCellTool(const std::string& t, const std::string& n, const IInterface* p)
  : AthAlgTool(t,n,p),
    m_caloGain((int)CaloGain::LARLOWGAIN),
    m_caloCell_Tot("TotalCalibCell"), m_caloCell_Vis("VisCalibCell"), 
    m_caloCell_Em(""), m_caloCell_NonEm("")
{
  declareInterface<CalibHitToCaloCellTool>(this);

  declareProperty("CaloGain", m_caloGain);
  declareProperty("CalibHitContainers", m_calibHitContainerNames);
 
  declareProperty("CellTotEne",    m_caloCell_Tot);
  declareProperty("CellVisEne",    m_caloCell_Vis);
  declareProperty("CellEmEne",     m_caloCell_Em);
  declareProperty("CellNonEmEne",  m_caloCell_NonEm);
  declareProperty("DoTile",        m_doTile=false);

  declareProperty("OutputCellContainerName", m_outputCellContainerName = "TruthCells");
  declareProperty("OutputClusterContainerName", m_outputClusterContainerName = "TruthClusters");
  
  m_tileActiveHitCnt   = "TileCalibHitActiveCell";
  m_tileInactiveHitCnt = "TileCalibHitInactiveCell";
  m_tileDMHitCnt       = "TileCalibHitDeadMaterial";
  m_larActHitCnt   = "LArCalibrationHitActive";
  m_larInactHitCnt = "LArCalibrationHitInactive";
  m_larDMHitCnt    = "LArCalibrationHitDeadMaterial";

}


CalibHitToCaloCellTool::~CalibHitToCaloCellTool() = default;


////////////////   INITIALIZE   ///////////////////////
StatusCode CalibHitToCaloCellTool::initialize() 
{ 
  // retrieve ID helpers from det store
  ATH_MSG_INFO("initialisation ID helpers" );

  ATH_CHECK( detStore()->retrieve(m_caloCell_ID) );
  ATH_CHECK( detStore()->retrieve(m_caloDM_ID) );
  ATH_CHECK( m_caloMgrKey.initialize() );

  
  for (unsigned int i=0; i<CalibHitUtils::nEnergyTypes; i++) {
    m_cellContKeys.push_back(m_outputCellContainerName+m_energyTypeToStr[i]);
    m_clusterContKeys.push_back(m_outputClusterContainerName+m_energyTypeToStr[i]);
    m_cellLinkKeys.push_back(m_outputClusterContainerName+m_energyTypeToStr[i]+"_links");
  }

  ATH_CHECK(m_cellContKeys.initialize());
  ATH_CHECK(m_clusterContKeys.initialize());
  ATH_CHECK(m_cellLinkKeys.initialize());

  ATH_MSG_INFO("initialisation completed" );
  return StatusCode::SUCCESS;
}


/////////////////   EXECUTE   //////////////////////
StatusCode CalibHitToCaloCellTool::processCalibHitsFromParticle(int barcode) const
{
  ATH_MSG_DEBUG("in calibHitToCaloCellTool");

  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey};
  ATH_CHECK(caloMgrHandle.isValid());
  const CaloDetDescrManager* caloDDMgr = *caloMgrHandle;

  //CaloCellContainer* truthCells[3];                                                                                                                                                                       
  //xAOD::CaloClusterContainer* truthClusters[3];                                                                                                                                                           
  
  std::vector<SG::WriteHandle<CaloCellContainer> > truthCells;
  std::vector<SG::WriteHandle<xAOD::CaloClusterContainer> > truthClusters;
  std::vector<SG::WriteHandle<CaloClusterCellLinkContainer> > truthLinks;


  // register containers for cells and clusters
  for (unsigned int i=0; i<CalibHitUtils::nEnergyTypes; i++) {

    truthCells.emplace_back(m_cellContKeys[i]);
    ATH_CHECK(truthCells.back().record(std::make_unique<CaloCellContainer>())); 

    truthClusters.emplace_back(m_clusterContKeys[i]);
    ATH_CHECK(CaloClusterStoreHelper::AddContainerWriteHandle(truthClusters.back()));

    truthLinks.emplace_back(m_cellLinkKeys[i]);
    ATH_CHECK(truthLinks.back().record(std::make_unique<CaloClusterCellLinkContainer>()));
  }    
 
  // retrieve calibration hit containers
  const unsigned int nCont = m_calibHitContainerNames.size();
  std::vector<const CaloCalibrationHitContainer*> calibHitContainers(nCont,nullptr);
  for (unsigned int i=0; i<nCont; i++) {
    ATH_MSG_DEBUG("Retrieving " << m_calibHitContainerNames[i]);
    ATH_CHECK( evtStore()->retrieve(calibHitContainers[i], m_calibHitContainerNames[i].c_str()) );
    ATH_MSG_DEBUG("  Retrieved container " << calibHitContainers[i]->Name() << " with size " << calibHitContainers[i]->Size() );
    if( calibHitContainers[i]->Size() == 0 ) {
      ATH_MSG_DEBUG("Container " << calibHitContainers[i]->Name() << " is empty");
    }
  }  
  ATH_MSG_DEBUG("CaloCalibrationHitContainers retrieved successfuly" );

  //count
  int nchan=0;
  int em_nchan=0;
  int hec_nchan=0;
  int fcal_nchan=0;
  int tile_nchan=0; 
  int unknown_nchan = 0 ;
  
  std::vector<Identifier> ID;
  if (barcode<0) barcode = HepMC::SINGLE_PARTICLE; // if no barcode is specified for this event, use the default


  std::vector<CaloCell*> CellsEtot;
  std::vector<CaloCell*> CellsEvis;
  std::vector<CaloCell*> CellsEem;
  	    
  int nhitsInactive = 0;

  for (unsigned int i=0; i<calibHitContainers.size(); i++) {
    for( const auto *const calibhit: *(calibHitContainers[i])) {
      //care only for deposits of the given truth particle
      if ((int)calibhit->particleID()!=barcode) continue;

      double Etot   = calibhit->energyTotal();
      double Eem    = calibhit->energy(0);
      double Enonem = calibhit->energy(1);
      double Evis   = Eem + Enonem;
      
      Identifier id=calibhit->cellID();

      //merge inactive and active hits from the same cell together
      if (i>0) { 
	//find if have already created cell..
	bool isNewId = true;
	for (int n=0; n<nhitsInactive; n++) {
	  if( id == ID[n] ) { //found
	    CellsEtot[n]->addEnergy(Etot);
	    CellsEvis[n]->addEnergy(Evis);
	    CellsEem[n]->addEnergy(Eem);
	    isNewId = false;
	    break;
	  }
	}
	if(!isNewId) continue; //go to next hit, else create new cell for this hit
      }

      //check if this ID is LAr or Tile
      if(m_caloCell_ID->is_lar(id)) {
	ATH_MSG_VERBOSE( "Found LAr cell" );	
	const CaloDetDescrElement* caloDDE = caloDDMgr->get_element(id);	  
	CellsEtot.push_back(new LArCell(caloDDE, id, Etot, 0., 0, 0, (CaloGain::CaloGain)m_caloGain)) ;
	CellsEvis.push_back(new LArCell(caloDDE, id, Evis, 0., 0, 0, (CaloGain::CaloGain)m_caloGain));
	CellsEem.push_back(new LArCell(caloDDE, id, Eem, 0., 0, 0, (CaloGain::CaloGain)m_caloGain)); 
	ID.push_back(id);
	++nchan;
      }
      else if(m_caloCell_ID->is_tile(id)) {
	ATH_MSG_VERBOSE( "Found Tile cell" );
	const CaloDetDescrElement* caloDDE = caloDDMgr->get_element(id);
	CellsEtot.push_back(new TileCell(caloDDE, id, Etot, 0., 0, 0, (CaloGain::CaloGain)m_caloGain)) ;
	CellsEvis.push_back(new TileCell(caloDDE, id, Evis, 0., 0, 0, (CaloGain::CaloGain)m_caloGain));
	CellsEem.push_back(new TileCell(caloDDE, id, Eem, 0., 0, 0, (CaloGain::CaloGain)m_caloGain)); 
	ID.push_back(id);
	++nchan;
      }
      else { //other, DeadMaterial
	//// FIXME DeadMaterial deposits not used; need to use m_caloDDMg;	
	ATH_MSG_VERBOSE( "Found unknown cell" );
	continue;
      }
    }    
    if (i==0) nhitsInactive = (int)ID.size();
  }
  
  //Now, put cells in the containers keeping the order. First goes EM, then HEC and so on
  // if(CellsEtot.size()==0) {
  //   ID.clear();
  //   return StatusCode::SUCCESS;
  // }

  ATH_MSG_DEBUG("N cells : " << nchan );
  
  for(int itr=0; itr!=nchan; itr++) {
    if(m_caloCell_ID->is_em(CellsEtot[itr]->ID())) {
      truthCells[CalibHitUtils::EnergyTotal]->push_back(CellsEtot[itr]);
      truthCells[CalibHitUtils::EnergyVisible]->push_back(CellsEvis[itr]);
      truthCells[CalibHitUtils::EnergyEM]->push_back(CellsEem[itr]);
      ++em_nchan;
    }
  }
  if(em_nchan) {
    for (int i=0;i<CalibHitUtils::nEnergyTypes;i++) truthCells[i]->setHasCalo(CaloCell_ID::LAREM);
  }

  for(int itr=0; itr!=nchan; itr++)  {
    if(m_caloCell_ID->is_hec(CellsEtot[itr]->ID())) {
      truthCells[CalibHitUtils::EnergyTotal]->push_back(CellsEtot[itr]);
      truthCells[CalibHitUtils::EnergyVisible]->push_back(CellsEvis[itr]);
      truthCells[CalibHitUtils::EnergyEM]->push_back(CellsEem[itr]);
      ++hec_nchan;
    }
  }
  if(hec_nchan){
    for (int i=0;i<CalibHitUtils::nEnergyTypes;i++) truthCells[i]->setHasCalo(CaloCell_ID::LARHEC);
  }

  for(int itr=0; itr!=nchan; itr++) {
    if(m_caloCell_ID->is_fcal(CellsEtot[itr]->ID())) {
      truthCells[CalibHitUtils::EnergyTotal]->push_back(CellsEtot[itr]);
      truthCells[CalibHitUtils::EnergyVisible]->push_back(CellsEvis[itr]);
      truthCells[CalibHitUtils::EnergyEM]->push_back(CellsEem[itr]);
      ++fcal_nchan;
    }
  }
  if(fcal_nchan) {
    for (int i=0;i<CalibHitUtils::nEnergyTypes;i++) truthCells[i]->setHasCalo(CaloCell_ID::LARFCAL);
  }

  for(int itr=0; itr!=nchan; itr++) {
    if((m_caloCell_ID->is_tile(CellsEtot[itr]->ID()))) {
      truthCells[CalibHitUtils::EnergyTotal]->push_back(CellsEtot[itr]);
      truthCells[CalibHitUtils::EnergyVisible]->push_back(CellsEvis[itr]);
      truthCells[CalibHitUtils::EnergyEM]->push_back(CellsEem[itr]);
      ++tile_nchan;
    }
  }
  if(tile_nchan) {
    for (int i=0;i<CalibHitUtils::nEnergyTypes;i++) truthCells[i]->setHasCalo(CaloCell_ID::TILE);
  }
  if(unknown_nchan) {
    for (int i=0;i<CalibHitUtils::nEnergyTypes;i++) truthCells[i]->setHasCalo(CaloCell_ID::NOT_VALID);
  }
  ATH_MSG_DEBUG("--- LAr INFO --- "<<nchan );
  ATH_MSG_DEBUG("LArCells  = "<<nchan );
  ATH_MSG_DEBUG("EMCells   = "<<em_nchan );
  ATH_MSG_DEBUG("HECCells  = "<<hec_nchan );
  ATH_MSG_DEBUG("FCALCells = "<<fcal_nchan );
  ATH_MSG_DEBUG("TileCells = "<<tile_nchan );
  ATH_MSG_DEBUG("NOT_VALID = "<<unknown_nchan );
    
  ID.clear();

  ///..........................................................

  ATH_MSG_DEBUG("making truth cluster");
  xAOD::CaloCluster* truthCluster[3] = {nullptr,nullptr,nullptr};
  for (int i=0;i<CalibHitUtils::nEnergyTypes;i++) {
    truthCluster[i] = CaloClusterStoreHelper::makeCluster(truthClusters[i].ptr(),truthCells[i].ptr());
    if (!truthCluster[i]) {
      ATH_MSG_FATAL("makeCluster failed");
      return StatusCode::FAILURE;
    }
    for (const auto cell: *truthCells[i]) {
      if(m_caloCell_ID->is_lar(cell->ID()) || m_caloCell_ID->is_tile(cell->ID())) 
	truthCluster[i]->addCell( truthCells[i]->findIndex(cell->caloDDE()->calo_hash()) , 1.);
    }
    
    truthCluster[i]->setClusterSize(xAOD::CaloCluster::CSize_Unknown);
    CaloClusterKineHelper::calculateKine(truthCluster[i], true, true);
    ATH_CHECK(CaloClusterStoreHelper::finalizeClusters (truthLinks[i],
							truthClusters[i].ptr()));
    
    ATH_MSG_INFO("Created truth cluster with " << m_energyTypeToStr[i] <<" " << truthCluster[i]->e());
  }

  ATH_MSG_DEBUG("execute() completed successfully" );
  return StatusCode::SUCCESS;
}


/////////////////   FINALIZE   //////////////////////
StatusCode CalibHitToCaloCellTool::finalize()
{
  ATH_MSG_INFO("finalize() successfully" );
  return StatusCode::SUCCESS;
}

