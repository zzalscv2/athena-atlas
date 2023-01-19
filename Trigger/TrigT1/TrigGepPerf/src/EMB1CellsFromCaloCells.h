/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGGEPPERF_EMB1CELLSFROMCALOCELLS_H
#define TRIGGEPPERF_EMB1CELLSFROMCALOCELLS_H


/* Obtain CaloCells from the  EMB1 sampling layer of the LArEM
   calorimeter for GEP pi0 searchs.
   The method cells() returns a vector of vector of CaloCells.
   There is one inner vector, which contains all the cells of interest.
 */

#include "ICaloCellsProducer.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloEvent/CaloCellContainer.h"

class EMB1CellsFromCaloCells:
public extends<AthAlgTool, ICaloCellsProducer> {

 public:
  
  EMB1CellsFromCaloCells(const std::string& type,
                          const std::string& name,
                          const IInterface* parent);

  virtual ~EMB1CellsFromCaloCells(){};

  virtual StatusCode initialize() override;
  virtual StatusCode cells(std::vector<std::vector<const CaloCell*>>&,
			   const EventContext&) const override;

private:

  SG::ReadHandleKey<CaloCellContainer> m_caloCellsKey {
    this, "caloCells", "AllCalo", "key to read in a CaloCell constainer"};  
  
  const CaloCell_ID* m_calocell_id{nullptr};
};
#endif
