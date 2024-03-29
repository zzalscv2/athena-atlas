/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: CaloClusterProcessor.cxx,v 1.2 2006-03-23 23:29:13 ssnyder Exp $
/**
 * @file  CaloClusterProcessor.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date March, 2006
 * @brief Base class for cluster processing tools called from
 *        @c CaloClusterMaker that operate on individual clusters.
 */

#include "CaloUtils/CaloClusterProcessor.h"
#include "CaloEvent/CaloClusterContainer.h"
#include "AthenaKernel/errorcheck.h"
#include "GaudiKernel/ThreadLocalContext.h"


/**
 * @brief Constructor.
 * @param type The type of the tool.
 * @param name The name of the tool.
 * @param parent The parent algorithm of the tool.
 *
 * This just forwards on to the base class constructor.
 */
CaloClusterProcessor::CaloClusterProcessor (const std::string& type,
                                            const std::string& name,
                                            const IInterface* parent)
  : AthAlgTool (type, name, parent)
{
  declareInterface<CaloClusterCollectionProcessor> (this);
  declareInterface<CaloClusterProcessor> (this);
}


/**
 * @brief Execute on a single cluster.
 * @param cluster The cluster to process.
 * (deprecated)
 */
StatusCode CaloClusterProcessor::execute (xAOD::CaloCluster* cluster)
{
  return execute (Gaudi::Hive::currentContext(), cluster);
}


/**
 * @brief Execute on an entire collection of clusters.
 * @param collection The container of clusters.
 * @param ctx The event context.
 *
 * This will iterate over all the clusters in @c collection
 * and call @c execute on each one individually.
 */
StatusCode CaloClusterProcessor::execute (const EventContext& ctx,
                                          xAOD::CaloClusterContainer* collection) const
{
  xAOD::CaloClusterContainer::iterator beg = collection->begin();
  xAOD::CaloClusterContainer::iterator end = collection->end();
  for (; beg != end; ++beg) {
    CHECK( execute (ctx, *beg) );
  }
  return StatusCode::SUCCESS;
}
