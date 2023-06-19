/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_DATAPREPARATION_PIXELCLUSTERIZATIONALG_H
#define ACTSTRK_DATAPREPARATION_PIXELCLUSTERIZATIONALG_H

#include <ActsTrkToolInterfaces/IPixelClusteringTool.h>
#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <AthenaMonitoringKernel/GenericMonitoringTool.h>
#include <GaudiKernel/ToolHandle.h>
#include <InDetIdentifier/PixelID.h>
#include <InDetRawData/PixelRDO_Container.h>
#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/WriteHandleKey.h>
#include <xAODInDetMeasurement/PixelClusterContainer.h>


namespace ActsTrk {

class PixelClusterizationAlg : public AthReentrantAlgorithm {
public:
    PixelClusterizationAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~PixelClusterizationAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

private:
    PixelClusterizationAlg() = delete;
    PixelClusterizationAlg(const PixelClusterizationAlg&) = delete;
    PixelClusterizationAlg &operator=(const PixelClusterizationAlg&) = delete;

    SG::ReadHandleKey<PixelRDO_Container> m_rdoContainerKey {this, "PixelRDOContainerKey", "PixelRDOs"};
    SG::WriteHandleKey<xAOD::PixelClusterContainer> m_clusterContainerKey {this, "PixelClustersKey", "xAODpixelClusters", "Key of output xAOD pixel cluster container"};
    ToolHandle<IPixelClusteringTool> m_clusteringTool {this, "PixelClusteringTool", "", "Pixel Clustering Tool"};
    ToolHandle<GenericMonitoringTool> m_monTool {this, "MonTool", "", "Monitoring tool"};

    // expected number of clusters for RDO
    // This values is used for reserving enough memory of the cluster container
    // reserve = m_expectedClustersPerRDO * nRDOs
    // The default values has been computed on a tt-bar PU200 sample comparing the memory usage and the container capacity
    Gaudi::Property<int> m_expectedClustersPerRDO {this, "expectedClustersPerRDO", 32, "Expected number of clusters for RDO"};

    const PixelID* m_idHelper = nullptr;
};

}

#endif
