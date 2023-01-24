/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DCMATHSEGMENTMAKER_MDTMATHSEGMENTFINDER_H
#define DCMATHSEGMENTMAKER_MDTMATHSEGMENTFINDER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRecToolInterfaces/IMdtSegmentFinder.h"
#include "MuonSegmentMakerInterfaces/IDCSLFitProvider.h"

namespace Muon {
    class MdtMathSegmentFinder : virtual public IMdtSegmentFinder::IMdtSegmentFinder, public AthAlgTool {
    public:
        MdtMathSegmentFinder(const std::string& t, const std::string& n, const IInterface* p);

        ~MdtMathSegmentFinder() = default;

        virtual StatusCode initialize();

        /** IMdtMdtMathSegmentFinder interface implementation              */
        virtual const TrkDriftCircleMath::SegVec findSegments(const TrkDriftCircleMath::DCVec& dcvec,
                                                              const TrkDriftCircleMath::CLVec& clvec, const TrkDriftCircleMath::Road& road,
                                                              const TrkDriftCircleMath::DCStatistics& dcstats,
                                                              const TrkDriftCircleMath::ChamberGeometry* multiGeo) const;

    protected:
        ToolHandle<IDCSLFitProvider> m_dcslFitProvider{
            this,
            "DCFitProvider",
            ""
        };

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
            this,
            "MuonIdHelperSvc",
            "Muon::MuonIdHelperSvc/MuonIdHelperSvc"
        };
  
        Gaudi::Property<int> m_finderDebugLevel{this, "FinderDebugLevel", 0, "switch on debug output of finder"};
        Gaudi::Property<bool> m_doDrop{this, "DoDrop",  true, "Recursive outlier removal"};        
        Gaudi::Property<bool> m_useChamberTheta{this, "UseChamberTheta", true, "Always look for pointing segments"};
       
        Gaudi::Property<bool> m_enableSeedCleaning {this, "EnableSeedCleaning",  false, "only use dc witout neighbours as seeds"};
        Gaudi::Property<double> m_occupancyThreshold{this, "OccupancyThreshold", 0.3, "occupancy threshold before enabling seed cleaning"};
        Gaudi::Property<double> m_occupancyCutOff {this, "OccupancyCutoff", 0.8, "above the occupancy threshold no segment finding"};
        Gaudi::Property<double> m_roadWidth {this, "AssociationRoadWidth", 1.5, "Road width used during hit association with seed lines"};
        Gaudi::Property<double> m_chi2PerDofDrop {this, "Chi2PerDofDropping", 10., "Chi2 cut for recursive outlier removal"};
        Gaudi::Property<double> m_ratioEmptyTubesCut{this, "RatioEmptyTubeCut", 1.1, "holes/hits cut - holes are all non-hits along the line"};
        
        Gaudi::Property<double> m_rpcAssociationPullCut{this, "RPCAssocationPullcut", 5., "Association cut for RPCs"};
        Gaudi::Property<double> m_tgcAssociationPullCut{this, "TGCAssocationPullcut", 5., "Association cut for TGCs"};
        Gaudi::Property<double> m_mdtAssociationPullCut{this, "MDTAssocationPullcut", 5., "Association cut for MDTs"};
        
        Gaudi::Property<bool> m_doAllHitSort{this, "SortSegmentWithAllHits", true, "Including triggers in segment selection"};
       
        Gaudi::Property<bool> m_doRoadAngleSeeding{this, "DoRoadSeeding", true, "use angle of road to seed segment search"};
        
        
        Gaudi::Property<bool> m_doIPAngleSeeding{this, "DoIPSeeding",  true, "use angle of IP to seed segment search"};
        Gaudi::Property<double> m_tightRoadCut{this, "TightRoadCut",  0.1, 
                                                "tight cut on angle with prediction, used for very busy chambers"};
        
        Gaudi::Property<bool> m_doSingleMultiLayerScan{this, "DoSingleMultiLayerScan", true, "Look for segments in one multi layer"};
       
        Gaudi::Property<bool> m_recoverMdtOutliers{this, "RecoverMdtOutliers", true, "Recover MDT outliers after fit"};
       
        Gaudi::Property<bool> m_removeSingleOutliers{this, "RemoveSingleMdtOutliers", true, "Remove single MDT outliers"};        
        Gaudi::Property<bool> m_doCurvedSegmentFinder{this, "DoCurvedSegmentFinder", false, "Use the curved segment finding routine"};
        Gaudi::Property<double> m_deltaCutT0Segments{this, "DeltaCutT0Segments",  5., "Delta cut for segments with T0 fit"};
        Gaudi::Property<double> m_residualCutT0Segments {this, "ResidualCutT0Segments", 1., "Residual cut for segments with T0 fit"};
        Gaudi::Property<bool> m_useSegmentQuality{this, "UseSegmentQuality", false, "Use segment quality in hit dropping"};

        Gaudi::Property<unsigned int> m_maxHitsPerFullSearch{this, "MaxHitsPerFullSearch", 100, 
                                                        "maximum number of hits, above will use faster search mode"};
    };

}  // namespace Muon

#endif
