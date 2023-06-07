/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTS_PIXEL_CLUSTERING_TOOL_H
#define ACTS_PIXEL_CLUSTERING_TOOL_H


#include "ActsTrkToolInterfaces/IPixelClusteringTool.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetRawData/InDetRawDataCollection.h"
#include "InDetRawData/PixelRDORawData.h"
#include "SiClusterizationTool/ClusterMakerTool.h"
#include "SiClusterizationTool/PixelRDOTool.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"

namespace ActsTrk {

// Helper functions for use with ACTS clusterization
//
inline int getCellRow(const InDet::UnpackedPixelRDO& cell)
{
    return cell.ROW;
}
    
inline int getCellColumn(const InDet::UnpackedPixelRDO& cell)
{
    return cell.COL;
}

inline int& getCellLabel(InDet::UnpackedPixelRDO& cell)
{
    return cell.NCL;
}


class PixelClusteringTool : public extends<AthAlgTool,IPixelClusteringTool> {
public:

    using Cell = InDet::UnpackedPixelRDO;
    using CellCollection = std::vector<Cell>;




    struct Cluster {
	std::vector<Identifier> ids;
	std::vector<int> tots;
	int lvl1min = std::numeric_limits<int>::max();
    };

    using ClusterCollection = std::vector<Cluster>;

    PixelClusteringTool(const std::string& type,
			const std::string& name,
			const IInterface* parent);

    virtual StatusCode
    clusterize(const InDetRawDataCollection<PixelRDORawData>& RDOs,
	       const PixelID& pixelID,
	       const EventContext& ctx,
	       xAOD::PixelClusterContainer& container) const override;

    virtual StatusCode initialize() override;

private:
    // N.B. the cluster is added to the container
    StatusCode makeCluster(const PixelClusteringTool::Cluster &cluster,
			   const PixelID& pixelID,
			   const InDetDD::SiDetectorElement* element,
			   xAOD::PixelClusterContainer& container) const;

    BooleanProperty m_addCorners{this, "AddCorners", true};
    ToolHandle<InDet::PixelRDOTool> m_pixelRDOTool {this, "PixelRDOTool", "InDet::PixelRDOTool"};
    ToolHandle<InDet::ClusterMakerTool> m_clusterMakerTool {this, "ClusterMakerTool", "InDet::ClusterMakerTool"};
    IntegerProperty m_errorStrategy{this, "ErrorStrategy", 1};

    SG::ReadCondHandleKey<PixelChargeCalibCondData> m_chargeDataKey {this, "PixelChargeCalibCondData", "PixelChargeCalibCondData", "Pixel charge calibration data"};

    SG::ReadCondHandleKey<PixelCalib::PixelOfflineCalibData> m_offlineCalibDataKey{this, "PixelOfflineCalibData", "PixelOfflineCalibData", "Pixel offline calibration data"};


};

} // namespace ActsTrk 

#endif // ACTS_PIXEL_CLUSTERING_TOOL_H
