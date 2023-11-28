/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file ISCT_ClusteringTool.h
 * Header file for the ISCT_ClusteringTool class
 * @author Paul J. Bell, 2004
 */

#ifndef SICLUSTERIZATIONTOOL_ISCT_CLUSTERINGTOOL_H
#define SICLUSTERIZATIONTOOL_ISCT_CLUSTERINGTOOL_H

/// Base class
#include "GaudiKernel/IAlgTool.h"
#include "InDetRawData/InDetRawDataCollection.h"
#include "InDetRawData/SCT_RDORawData.h"

//// no forward declare ... typedef
#include "AthAllocators/DataPool.h"
#include "InDetPrepRawData/SCT_ClusterCollection.h"
class SCT_ID;

class SCT_ChannelStatusAlg;

namespace InDet {
class SiDetectorElementStatus;

struct SCTClusteringCache {
  using IdVec_t = std::vector<Identifier>;
  // Vector of identifiers in a cluster (most likely is that there is one strip
  // in the cluster)
  IdVec_t currentVector;
  // Vector of clusters to make the cluster collection (most likely equal to
  // collection size)
  std::vector<IdVec_t> idGroups;
  std::vector<uint16_t> tbinGroups;
};

/// Interface for SCT clustering algorithms, taking input from RDOs
class ISCT_ClusteringTool : virtual public IAlgTool {
 public:
  /// InterfaceID
  DeclareInterfaceID(ISCT_ClusteringTool, 1, 0);

  /// Default destructor
  virtual ~ISCT_ClusteringTool() = default;

  /**
   * Clusterize a collection of SCT raw data objects
   * Called from SCT_clusterization algorithm in InDetPrepRawDataFormation,
   * once per module
   * @param[in] rdoCollection the raw data objects
   * @param[in] idHelper the SCT helper class
   * @param[in] sctDetElementStatus information the module, chip and strip
   * status or nullptr
   */
  virtual SCT_ClusterCollection* clusterize(
      const InDetRawDataCollection<SCT_RDORawData>& rdoCollection,
      const SCT_ID& idHelper,
      const InDet::SiDetectorElementStatus* sctDetElementStatus,
      SCTClusteringCache& cache, DataPool<SCT_Cluster>* dataItemsPool,
      const EventContext& ctx) const = 0;
};
}  // namespace InDet

#endif  // SICLUSTERIZATIONTOOL_I_SCT_CLUSTERINGALG_H
