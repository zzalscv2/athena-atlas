/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGGEPPERF_EMB1CELLSFROMCALOCLUSTERS_H
#define TRIGGEPPERF_EMB1CELLSFROMCALOCLUSTERS_H

/* Obtain CaloCells for GEP pi0 searchs from a Cell Collection.
   The method cells() returns a vector of vector of CaloCells.
   There is one inner vector of EMB1 cells per CaloCluster
 */

#include "ICaloCellsProducer.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODCaloEvent/CaloClusterContainer.h"

class EMB1CellsFromCaloClusters:
public extends<AthAlgTool, ICaloCellsProducer> {

 public:
  
  EMB1CellsFromCaloClusters(const std::string& type,
                          const std::string& name,
                          const IInterface* parent);

  virtual ~EMB1CellsFromCaloClusters(){};

  virtual StatusCode initialize() override;
  virtual StatusCode cells(std::vector<std::vector<const CaloCell*>>&,
			   const EventContext&) const override;

private:

  SG::ReadHandleKey<xAOD::CaloClusterContainer> m_caloClustersKey {
    this, "caloClusters", "CaloTopoClusters",
      "key to read in a CaloCluster container"};
};
#endif
