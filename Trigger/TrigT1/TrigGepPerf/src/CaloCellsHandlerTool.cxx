/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include <vector>
#include "TMath.h"
#include "./CaloCellsHandlerTool.h"

CaloCellsHandlerTool::CaloCellsHandlerTool(const std::string& type,
					   const std::string& name,
					   const IInterface* parent): AthAlgTool(type, name, parent){
}

CaloCellsHandlerTool::~CaloCellsHandlerTool() {}

StatusCode CaloCellsHandlerTool::initialize() {
  ATH_CHECK(m_electronicNoiseKey.initialize());
  ATH_CHECK(m_totalNoiseKey.initialize());
  
  // CaloIndentifier
  CHECK( detStore()->retrieve (m_CaloCell_ID, "CaloCell_ID") );

  return StatusCode::SUCCESS;
}

// Get calo cells map

StatusCode CaloCellsHandlerTool::getGepCellMap(const CaloCellContainer& cells,
					       pGepCellMap& gepCellsMap, 
					       const EventContext& ctx) const {

  // PS this function creates and returns a map which has Gep::GustomCells as its values
  // The cells are made up of data which is envariant for all events, and dynamic
  // data which varies with event.
  // The invariant data should be setup in initialize(), and it the dynamic
  // data should be updated here in a way which is compatible with the const-ness of this
  // function.
  // This will be attended to in the future.

  SG::ReadCondHandle<CaloNoise> electronicNoiseHdl{m_electronicNoiseKey,  ctx};
  if (!electronicNoiseHdl.isValid()) {return StatusCode::FAILURE;}
  const CaloNoise* electronicNoiseCDO = *electronicNoiseHdl;

  SG::ReadCondHandle<CaloNoise> totalNoiseHdl{m_totalNoiseKey, ctx};
  if (!totalNoiseHdl.isValid()) {return StatusCode::FAILURE;}
  const CaloNoise* totalNoiseCDO = *totalNoiseHdl;

  
  for(const auto *cell: cells){
    Gep::CustomCaloCell caloCell; 
    
    caloCell.e          = cell->energy();
    caloCell.et         = cell->energy() * 1.0/TMath::CosH(cell->eta());
    caloCell.time       = cell->time();
    caloCell.quality    = cell->quality();
    caloCell.provenance = cell->provenance();    
          
    float electronicNoise = electronicNoiseCDO->getNoise(cell->ID(), cell->gain());
    float totalNoise = totalNoiseCDO->getNoise(cell->ID(), cell->gain());
   
    caloCell.totalNoise = totalNoise;
    caloCell.electronicNoise = electronicNoise;
    caloCell.sigma = caloCell.e / totalNoise;

    caloCell.isBad = cell->badcell();    
    caloCell.eta    = cell->eta();
    caloCell.phi    = cell->phi();
    caloCell.sinTh  = cell->sinTh();
    caloCell.cosTh  = cell->cosTh();
    caloCell.sinPhi = cell->sinPhi();
    caloCell.cosPhi = cell->cosPhi();
    caloCell.cotTh  = cell->cotTh();
    caloCell.x = cell->x();
    caloCell.y = cell->y();
    caloCell.z = cell->z();
    
    unsigned int samplingEnum = m_CaloCell_ID->calo_sample(cell->ID());
    
    bool IsEM = m_CaloCell_ID->is_em(cell->ID());
    bool IsEM_Barrel=false;
    bool IsEM_EndCap=false;
    bool IsEM_BarrelPos=false;
    bool IsEM_BarrelNeg=false;
    if(IsEM){
      IsEM_Barrel=m_CaloCell_ID->is_em_barrel(cell->ID());
      if(IsEM_Barrel){
	if(m_CaloCell_ID->pos_neg(cell->ID())>0) IsEM_BarrelPos=true;
      }
      IsEM_EndCap=m_CaloCell_ID->is_em_endcap(cell->ID());
    }
    
    caloCell.isEM           = IsEM;
    caloCell.isEM_barrel    = IsEM_Barrel;
    caloCell.isEM_endCap    = IsEM_EndCap;
    caloCell.isEM_barrelPos = IsEM_BarrelPos;
    caloCell.isEM_barrelNeg = IsEM_BarrelNeg;  //always false?
    caloCell.isFCAL = m_CaloCell_ID->is_fcal(cell->ID());
    caloCell.isHEC  = m_CaloCell_ID->is_hec(cell->ID());
    caloCell.isTile = m_CaloCell_ID->is_tile(cell->ID());
    
    caloCell.sampling = samplingEnum;
    caloCell.detName = CaloSampling::getSamplingName(samplingEnum);
    
    caloCell.neighbours = getNeighbours(cells, cell, ctx);
    caloCell.id = (cell->ID().get_identifier32()).get_compact(); 
    
    const CaloDetDescriptor *elt = cell->caloDDE()->descriptor();
    caloCell.layer = cell->caloDDE()->getLayer();
    
    float deta = elt->deta();
    float dphi = elt->dphi();
    
    float etamin = caloCell.eta - (0.5*deta);
    float etamax = caloCell.eta + (0.5*deta);
    
    float phimin = caloCell.phi - (0.5*dphi);
    float phimax = caloCell.phi + (0.5*dphi);
    
    caloCell.etaMin = etamin;
    caloCell.etaMax = etamax;
    caloCell.phiMin = phimin;
    caloCell.phiMax = phimax;
    caloCell.etaGranularity = deta;
    caloCell.phiGranularity = dphi;
    
    // store cells map
    gepCellsMap->insert(std::pair<unsigned int, Gep::CustomCaloCell>(caloCell.id, caloCell));
    
  }
  
  return StatusCode::SUCCESS;
}



// Get neighbours of a given calo cell
std::vector<unsigned int>
CaloCellsHandlerTool::getNeighbours(const CaloCellContainer& allcells,
				    const CaloCell* acell,
				    const EventContext&) const {

  // get all neighboring cells
  std::vector<IdentifierHash> cellNeighbours;

  IdentifierHash cellHashID = m_CaloCell_ID->calo_cell_hash(acell->ID());
  m_CaloCell_ID->get_neighbours(cellHashID,LArNeighbours::super3D,cellNeighbours);

  std::vector<unsigned int> neighbour_ids;
  for (unsigned int iNeighbour = 0;
       iNeighbour < cellNeighbours.size();
       ++iNeighbour) {
    
    const CaloCell* neighbour = allcells.findCell(cellNeighbours[iNeighbour]);
    if (neighbour) {
      neighbour_ids.push_back((neighbour->ID().get_identifier32()).get_compact());
    } else {
      ATH_MSG_ERROR("Couldn't access neighbour #" << iNeighbour
		    << " for cell ID "
		    << (acell->ID().get_identifier32()).get_compact());
    }
  }
  return neighbour_ids;
}



