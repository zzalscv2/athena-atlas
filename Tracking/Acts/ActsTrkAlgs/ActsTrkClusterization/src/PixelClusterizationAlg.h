/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKCLUSTERIZATION_PIXELCLUSTERIZATIONALG_H
#define ACTSTRKCLUSTERIZATION_PIXELCLUSTERIZATIONALG_H

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

    const PixelID* m_idHelper = nullptr;
};

}

#endif