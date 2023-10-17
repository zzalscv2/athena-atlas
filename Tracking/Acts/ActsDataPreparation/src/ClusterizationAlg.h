/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_DATAPREPARATION_CLUSTERIZATIONALG_H
#define ACTSTRK_DATAPREPARATION_CLUSTERIZATIONALG_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <AthenaMonitoringKernel/GenericMonitoringTool.h>
#include <GaudiKernel/ToolHandle.h>
#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/WriteHandleKey.h>
#include <TrigSteeringEvent/TrigRoiDescriptorCollection.h>
#include <IRegionSelector/IRegSelTool.h>
#include <InDetReadoutGeometry/SiDetectorElementCollection.h>

namespace ActsTrk {

template <typename IClusteringTool>
class ClusterizationAlg : public AthReentrantAlgorithm {
public:
    using RDOContainer = typename IClusteringTool::RDOContainer;
    using RawDataCollection = typename RDOContainer::base_value_type;
    using ClusterContainer = typename IClusteringTool::ClusterContainer;
    using ClusterAuxContainer = typename IClusteringTool::ClusterAuxContainer;
    using IDHelper = typename IClusteringTool::IDHelper;

    ClusterizationAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~ClusterizationAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    
private:
    ClusterizationAlg() = delete;
    ClusterizationAlg(const ClusterizationAlg&) = delete;
    ClusterizationAlg &operator=(const ClusterizationAlg&) = delete;
    
    ToolHandle<IClusteringTool> m_clusteringTool {
	this, "ClusteringTool", "", "Clustering Tool"
    };

    ToolHandle<GenericMonitoringTool> m_monTool {
	this, "MonTool", "", "Monitoring tool"
    };

    ToolHandle<IRegSelTool> m_regionSelector {
	this, "RegSelTool", "", "Region selector tool"
    };

    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_detEleCollKey {
	this,
	"SiDetectorElementCollectionKey",
	"",
	"Si detector element collection key"
    };

    SG::ReadHandleKey<RDOContainer> m_rdoContainerKey {
	this,
	"RDOContainerKey",
	"",
	"Input RDO container key"
    };

    SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roiCollectionKey {
	this, "RoIs", "", "RoIs to read in"
    };

    SG::WriteHandleKey<ClusterContainer> m_clusterContainerKey {
	this,
	"ClustersKey",
	"",
	"Key of output xAOD pixel cluster container"
    };

    // expected number of clusters for RDO
    // This values is used for reserving enough memory of the cluster container
    // reserve = m_expectedClustersPerRDO * nRDOs
    // The default values has been computed on a tt-bar PU200 sample
    // comparing the memory usage and the container capacity
    Gaudi::Property<int> m_expectedClustersPerRDO {
      this,
      "expectedClustersPerRDO",
      32,
      "Expected number of clusters for RDO"
    };

    Gaudi::Property<std::string> m_idHelperName {
	this,
	"IDHelper",
	"",
	"Name of ID helper to fetch from detstore"
    };

    const IDHelper* m_idHelper = nullptr;
};

} // namespace ActsTrk

#include "ClusterizationAlg.ipp"

#endif
