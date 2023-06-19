/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "PixelClusterizationAlg.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"

namespace ActsTrk {

PixelClusterizationAlg::PixelClusterizationAlg(const std::string& name,
							   ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}


StatusCode PixelClusterizationAlg::initialize()
{
    ATH_CHECK(m_rdoContainerKey.initialize());
    ATH_CHECK(m_clusterContainerKey.initialize());
    ATH_CHECK(m_clusteringTool.retrieve());
    ATH_CHECK(detStore()->retrieve(m_idHelper,"PixelID"));

    if (!m_monTool.empty())
	ATH_CHECK(m_monTool.retrieve());

    return StatusCode::SUCCESS;
}


StatusCode PixelClusterizationAlg::execute(const EventContext& ctx) const
{
    auto timer = Monitored::Timer<std::chrono::milliseconds>( "TIME_execute" );
    auto mon = Monitored::Group( m_monTool, timer );

    SG::ReadHandle<PixelRDO_Container> rdoContainer = SG::makeHandle(m_rdoContainerKey, ctx);
    ATH_CHECK(rdoContainer.isValid());

    SG::WriteHandle<xAOD::PixelClusterContainer> clusterHandle
	= SG::makeHandle(m_clusterContainerKey, ctx);

    std::unique_ptr<xAOD::PixelClusterContainer> pixelContainer = std::make_unique<xAOD::PixelClusterContainer>();
    std::unique_ptr<xAOD::PixelClusterAuxContainer> pixelAuxContainer = std::make_unique<xAOD::PixelClusterAuxContainer>();
    pixelContainer->setStore(pixelAuxContainer.get());

    // Reserve space, estimate of mean clusters to reduce re-allocations
    pixelContainer->reserve( m_expectedClustersPerRDO.value() * rdoContainer->size() );

    for (const InDetRawDataCollection<PixelRDORawData> *rdos: *rdoContainer) {
	if (rdos != nullptr && !rdos->empty())
	  ATH_CHECK(m_clusteringTool->clusterize(*rdos, *m_idHelper, ctx, *pixelContainer.get()));
	else
	    ATH_MSG_DEBUG("No input RDOs for this container element");
    }

    ATH_CHECK(clusterHandle.record(std::move(pixelContainer), std::move(pixelAuxContainer)));
    return StatusCode::SUCCESS;
}

}

