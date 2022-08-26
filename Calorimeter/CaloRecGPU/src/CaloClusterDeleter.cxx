/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloClusterDeleter.h"

CaloClusterDeleter::CaloClusterDeleter(const std::string & type, const std::string & name, const IInterface * parent):
  AthAlgTool(type, name, parent)
{
  declareInterface<CaloClusterCollectionProcessor> (this);
}

StatusCode CaloClusterDeleter::execute (const EventContext &, xAOD::CaloClusterContainer * cluster_collection) const
{
  cluster_collection->clear();

  return StatusCode::SUCCESS;

}


CaloClusterDeleter::~CaloClusterDeleter()
{
  //Nothing!
}