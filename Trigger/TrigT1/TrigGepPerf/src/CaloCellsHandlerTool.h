/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGL0GEPPERF_CaloCellsHandlerTool_H
#define TRIGL0GEPPERF_CaloCellsHandlerTool_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "CaloEvent/CaloCellContainer.h"
#include "CaloEvent/CaloCell.h"
#include "GaudiKernel/ToolHandle.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloConditions/CaloNoise.h"

#include "./CustomCaloCell.h"

#include <vector>


using  GepCellMap = std::map<unsigned int,Gep::CustomCaloCell>;
using  pGepCellMap = std::unique_ptr<GepCellMap>;

class CaloCellsHandlerTool: public AthAlgTool {
  
public:
  
  CaloCellsHandlerTool(const std::string& type,
		       const std::string& name,
		       const IInterface* parent);

  virtual ~CaloCellsHandlerTool();

  StatusCode initialize();
  
  StatusCode getGepCellMap(const CaloCellContainer& cells,
			   pGepCellMap&,
			   const EventContext& ctx) const;

  
 private:
  
  /** @brief Key of the CaloNoise Conditions data object. Typical values 
      are '"electronicNoise', 'pileupNoise', or '"totalNoise' (default) */

  SG::ReadCondHandleKey<CaloNoise>
  m_electronicNoiseKey{this,
      "electronicNoiseKey",
      "totalNoise",
      "SG Key of CaloNoise data object"};

  
  SG::ReadCondHandleKey<CaloNoise>
  m_totalNoiseKey{this,
		  "totalNoiseKey",
		  "totalNoise",
		  "SG Key of CaloNoise data object"};
  
  
  // const mutable CaloCell_ID* m_ccIdHelper;
  const CaloCell_ID* m_CaloCell_ID;
  std::vector<unsigned int> getNeighbours(const CaloCellContainer& allcells,
					  const CaloCell* acell,
					  const EventContext&) const;

};

#endif
