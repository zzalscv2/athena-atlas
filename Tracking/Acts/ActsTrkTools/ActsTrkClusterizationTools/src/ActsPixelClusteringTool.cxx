/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsPixelClusteringTool.h"
using ActsTrk::getCellRow;
using ActsTrk::getCellColumn;
using ActsTrk::getCellLabel;

#include <Acts/Clusterization/Clusterization.hpp>

#include <PixelReadoutGeometry/PixelModuleDesign.h>
#include <xAODInDetMeasurement/PixelCluster.h>
#include <xAODInDetMeasurement/PixelClusterContainer.h>
#include <xAODInDetMeasurement/PixelClusterAuxContainer.h>

#include <unordered_set>

namespace ActsTrk {

void clusterAddCell(PixelClusteringTool::Cluster& cl, const PixelClusteringTool::Cell& cell)
{
    cl.ids.push_back(cell.ID);
    cl.tots.push_back(cell.TOT);
    if (cell.LVL1 < cl.lvl1min)
	cl.lvl1min = cell.LVL1;
}

StatusCode PixelClusteringTool::initialize()
{
    ATH_CHECK(m_pixelRDOTool.retrieve());

    ATH_CHECK(m_clusterMakerTool.retrieve());
    ATH_CHECK(m_chargeDataKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_offlineCalibDataKey.initialize(SG::AllowEmpty));

    ATH_MSG_DEBUG("Acts::PixelClusteringTool successfully initialized");
    return StatusCode::SUCCESS;
}

PixelClusteringTool::PixelClusteringTool(
    const std::string& type, const std::string& name, const IInterface* parent)
    : base_class(type,name,parent)
{
}

StatusCode
PixelClusteringTool::makeCluster(const PixelClusteringTool::Cluster &cluster,
				 const PixelID& pixelID,
				 const InDetDD::SiDetectorElement* element,
				 xAOD::PixelClusterContainer& container) const
{
    const InDetDD::PixelModuleDesign& design = 
	dynamic_cast<const InDetDD::PixelModuleDesign&>(element->design());

    InDetDD::SiLocalPosition pos_acc(0,0);
    int tot_acc = 0;

    int colmax = std::numeric_limits<int>::min();
    int rowmax = std::numeric_limits<int>::min();
    int colmin = std::numeric_limits<int>::max();
    int rowmin = std::numeric_limits<int>::max();

    bool hasGanged = false;

    for (size_t i = 0; i < cluster.ids.size(); i++) {
	Identifier id = cluster.ids.at(i);
	hasGanged = hasGanged ||
		    m_pixelRDOTool->isGanged(id, element).has_value();

	const int row = pixelID.phi_index(id);
	if (row > rowmax)
	    rowmax = row;
	if (row < rowmin)
	    rowmin = row;

	const int col = pixelID.eta_index(id);
	if (col > colmax)
	    colmax = col;
	if (col < colmin)
	    colmin = col;

	InDetDD::SiCellId si_cell = element->cellIdFromIdentifier(id);
	InDetDD::SiLocalPosition pos = design.localPositionOfCell(si_cell);
	int tot = cluster.tots.at(i);
	pos_acc += tot * pos;
	tot_acc += tot;
    }

    if (tot_acc > 0)
	pos_acc /= tot_acc;

    const int colWidth = colmax - colmin + 1;
    const int rowWidth = rowmax - rowmin + 1;
    double etaWidth = design.widthFromColumnRange(colmin, colmax);
    double phiWidth = design.widthFromRowRange(rowmin, rowmax);
    InDet::SiWidth siWidth(Amg::Vector2D(rowWidth,colWidth), Amg::Vector2D(phiWidth,etaWidth));


    const PixelChargeCalibCondData *calibData = nullptr;
    if (!m_chargeDataKey.empty()) {
      SG::ReadCondHandle<PixelChargeCalibCondData> calibDataHandle(m_chargeDataKey);
      calibData = *calibDataHandle;
    }

    const PixelCalib::PixelOfflineCalibData *offlineCalibData = nullptr;
    if (!m_offlineCalibDataKey.empty()) {
      SG::ReadCondHandle<PixelCalib::PixelOfflineCalibData> offlineCalibDataHandle(m_offlineCalibDataKey);
      offlineCalibData = *offlineCalibDataHandle;
    }



    // N.B. the cluster is automatically added to the container
    xAOD::PixelCluster *cl =
	m_clusterMakerTool->xAODpixelCluster(
	    container,
	    pos_acc,
	    cluster.ids,
	    cluster.lvl1min,
	    cluster.tots,
	    siWidth,
	    element,
	    hasGanged,
	    m_errorStrategy,
	    pixelID,
      false,
      0.0,
      0.0,
      calibData,
      offlineCalibData
	    );

    return (cl != nullptr) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}


StatusCode
PixelClusteringTool::clusterize(const InDetRawDataCollection<PixelRDORawData>& RDOs,
				const PixelID& pixelID,
				const EventContext& ctx,
				xAOD::PixelClusterContainer& container) const
{
    const InDetDD::SiDetectorElement* element = m_pixelRDOTool->checkCollection(RDOs, ctx);
    if (element == nullptr)
	return StatusCode::FAILURE;

    std::vector<InDet::UnpackedPixelRDO> cells =
	m_pixelRDOTool->getUnpackedPixelRDOs(RDOs, pixelID, element, ctx);

    ClusterCollection clusters =
      Acts::Ccl::createClusters<CellCollection, ClusterCollection, 2>
      (cells, Acts::Ccl::DefaultConnect<Cell, 2>(m_addCorners));

    for (const Cluster& cluster : clusters)
	ATH_CHECK(makeCluster(cluster, pixelID, element, container));

    return StatusCode::SUCCESS;
}

} // namespace ActsTrk
