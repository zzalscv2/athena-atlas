/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_DATAPREPARATION_STRIPCLUSTERIZATIONALG_H
#define ACTSTRK_DATAPREPARATION_STRIPCLUSTERIZATIONALG_H

#include <ActsToolInterfaces/IStripClusteringTool.h>
#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <AthenaMonitoringKernel/GenericMonitoringTool.h>
#include <GaudiKernel/ToolHandle.h>
#include <InDetConditionsSummaryService/IInDetConditionsTool.h>
#include <InDetIdentifier/SCT_ID.h>
#include <InDetRawData/SCT_RDO_Container.h>
#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/WriteHandleKey.h>
#include <xAODInDetMeasurement/StripClusterContainer.h>

namespace ActsTrk {

class StripClusterizationAlg : public AthReentrantAlgorithm {
public:
    StripClusterizationAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~StripClusterizationAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

private:

    using StripID = SCT_ID;

    StripClusterizationAlg() = delete;
    StripClusterizationAlg(const StripClusterizationAlg&) = delete;
    StripClusterizationAlg &operator=(const StripClusterizationAlg&) = delete;

    SG::ReadHandleKey<SCT_RDO_Container> m_rdoContainerKey {
	this,
	"StripRDOContainerKey",
	"ITkStripRDOs"
    };
    // TODO this one should be removed?
    SG::ReadHandleKey<InDet::SiDetectorElementStatus> m_stripDetElStatus {
	this,
	"StripDetElStatus",
	"",
	"SiDetectorElementStatus for strip"
    };
    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_stripDetEleCollKey{
	this,
	"StripDetEleCollKey",
	"ITkStripDetectorElementCollection",
	"SiDetectorElementCollection key for strip"
    };
    SG::WriteHandleKey<xAOD::StripClusterContainer> m_clusterContainerKey {
	this,
	"StripClustersKey",
	"ITkStripClusters",
	"Output xAOD strip cluster container"
    };
    ToolHandle<IStripClusteringTool> m_clusteringTool {
	this,
	"StripClusteringTool",
	"",
	"Strip Clustering Tool"
    };
    ToolHandle<IInDetConditionsTool> m_summaryTool{
	this,
	"conditionsTool",
	"SCT_ConditionsSummaryTool/ITkStripConditionsSummaryTool",
	"Conditions summary tool"
    };
    ToolHandle<GenericMonitoringTool> m_monTool {
	this,
	"MonTool",
	"",
	"Monitoring tool"
    };
    BooleanProperty m_checkBadModules{
	this,
	"checkBadModules",
	true,
	"Check bad modules using the conditions summary tool"
    };

    // expected number of clusters for RDO
    // This values is used for reserving enough memory of the cluster container
    // reserve = m_expectedClustersPerRDO * nRDOs
    // The default values has been computed on a tt-bar PU200 sample comparing the memory usage and the container capacity
    Gaudi::Property<int> m_expectedClustersPerRDO {this, "expectedClustersPerRDO", 6, "Expected number of clusters for RDO"};

    const StripID* m_idHelper = nullptr;
};

} // namespace ActsTrk

#endif
