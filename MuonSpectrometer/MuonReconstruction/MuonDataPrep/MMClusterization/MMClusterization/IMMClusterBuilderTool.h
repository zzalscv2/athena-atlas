/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef IMMClusterBuilderTool_h
#define IMMClusterBuilderTool_h

//
// Interface class for MM clustering
//
#include <vector>

#include "EventPrimitives/EventPrimitives.h"
#include "GaudiKernel/IAlgTool.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "NSWCalibTools/INSWCalibTool.h"
#include "MuonPrepRawData/MMPrepData.h"
#include "MuonRIO_OnTrack/MMClusterOnTrack.h"


namespace Muon {

    class IMMClusterBuilderTool : virtual public IAlgTool {
    public:  // static methods
        DeclareInterfaceID(Muon::IMMClusterBuilderTool, 1, 0);

        /** @brief  Standard Interface to produce Micromega clusters from raw Input hits without external contstaint. 
                    Executes the clusterization algorithm & fills the results in the clusterVect
                    Returns a failure in case of conditions or unexpected problems
          * @param ctx: EventContext used for StoreGate access
          * @param stripsVect: Vector of the single Micromega hits.
          * @param clusterVect: Output vector of the merged clusters
        */
        virtual StatusCode getClusters(const EventContext& ctx,
                                       std::vector<Muon::MMPrepData>&& stripsVect,
                                       std::vector<std::unique_ptr<Muon::MMPrepData>>& clustersVect) const = 0;
        
        /** @brief Refinement of the cluster position after the cluster calibration loop is ran with a complete external constraint
                   (e.g. from the track fit). The cluster local position and the associated covariance are updated during this procedure
                   Returns the Author of the ClusterBuilderTool, in case that the calibrated strip passes all internal constraints of
                   the ClusterBuilder, otherwise Unknown author is returned. In the latter case, the Cluster should not be transformed
                   into a calibrated RIO object.
          * @param ctx: EventContext used for StoreGate access
            @param calibratedStrips: Cluster constitutents after the calibration stage
            @param directionEstimate: Estimate of the direction given in the local frame
            @param clusterLocalPosition: Vector to which the final cluster position is saved to
            @param covMatrix: Uncertainty associated with the local position
        */
        using RIO_Author = MMClusterOnTrack::Author;
        virtual RIO_Author getCalibratedClusterPosition(const EventContext& ctx, 
                                                        const std::vector<NSWCalib::CalibratedStrip>& calibratedStrips,
                                                        const Amg::Vector3D& directionEstimate, 
                                                        Amg::Vector2D& clusterLocalPosition,
                                                        Amg::MatrixX& covMatrix) const = 0; 
    };   

}  // namespace Muon

#endif
