/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloEvent/CaloCellContainer.h"
#include "CaloCellContCopyTool.h"
#include "TileEvent/TileCell.h"
#include "LArRecEvent/LArCell.h"
#include "AthAllocators/DataPool.h"     

StatusCode CaloCellContCopyTool::initialize() {

  ATH_MSG_INFO( "In initialize ");
  ATH_CHECK(m_srcCellContainerKey  .initialize());
  return StatusCode::SUCCESS;
}


StatusCode CaloCellContCopyTool::process (CaloCellContainer* theCont,
                                          const EventContext& ctx) const {
  // Retrieve source cell container
  SG::ReadHandle<CaloCellContainer> srcCont(m_srcCellContainerKey, ctx);

  //Copy indivial cells - profiting from DataPool to avoid small allocations
  DataPool<LArCell> lArPool(ctx,182468);
  DataPool<TileCell> tilePool(ctx,5217);
  for (const CaloCell* oldCell : *srcCont) {
    const CaloDetDescrElement* dde=oldCell->caloDDE();
    if (dde->is_tile()) {
      TileCell* tCell=tilePool.nextElementPtr();
      const TileCell* oldtCell=static_cast<const TileCell*>(oldCell);
      *tCell=*oldtCell;
      theCont->push_back_fast(tCell); 
    } 
    else {
      LArCell* pCell = lArPool.nextElementPtr();
       const LArCell* oldlCell=static_cast<const LArCell*>(oldCell);
      *pCell=*oldlCell;
      theCont->push_back_fast(pCell);
    }
  }

  //Copy container-properties
  theCont->setIsOrderedAndComplete(srcCont->isOrderedAndComplete());
  theCont->setIsOrdered(srcCont->isOrdered());
  theCont->setHasTotalSize(srcCont->hasTotalSize());
  theCont->m_hasCalo=srcCont->m_hasCalo;
  theCont->updateCaloIterators();
  theCont->resetLookUpTable();

  return StatusCode::SUCCESS;
}