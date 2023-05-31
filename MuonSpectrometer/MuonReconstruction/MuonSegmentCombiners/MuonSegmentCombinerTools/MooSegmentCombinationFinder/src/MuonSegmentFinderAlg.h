/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MOOSEGMENTFINDERS_MUOSEGMENTFINDERALGS_H
#define MOOSEGMENTFINDERS_MUOSEGMENTFINDERALGS_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "CscSegmentMakers/ICscSegmentFinder.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPattern/MuonPatternChamberIntersect.h"
#include "MuonPrepRawData/CscPrepDataCollection.h"
#include "MuonPrepRawData/MdtPrepDataCollection.h"
#include "MuonPrepRawData/RpcPrepDataCollection.h"
#include "MuonPrepRawData/TgcPrepDataCollection.h"
#include "MuonRecHelperTools/MuonEDMPrinterTool.h"
#include "MuonRecToolInterfaces/IMuonClusterOnTrackCreator.h"
#include "MuonRecToolInterfaces/IMuonSegmentMaker.h"
#include "MuonSegment/MuonSegmentCombinationCollection.h"
#include "MuonSegmentCombinerToolInterfaces/IMuonCurvedSegmentCombiner.h"
#include "MuonSegmentMakerToolInterfaces/IMuonClusterSegmentFinder.h"
#include "MuonSegmentMakerToolInterfaces/IMuonSegmentSelectionTool.h"
#include "MuonSegmentMakerToolInterfaces/IMuonNSWSegmentFinderTool.h"
#include "MuonSegmentMakerToolInterfaces/IMuonPatternCalibration.h"
#include "MuonSegmentMakerToolInterfaces/IMuonSegmentOverlapRemovalTool.h"
#include "TrkSegment/SegmentCollection.h"
#include "TrkTruthData/PRD_MultiTruthCollection.h"

class MuonSegmentFinderAlg : public AthReentrantAlgorithm {
public:
    MuonSegmentFinderAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual ~MuonSegmentFinderAlg() = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this,
        "MuonIdHelperSvc",
        "Muon::MuonIdHelperSvc/MuonIdHelperSvc",
    };

    ToolHandle<Muon::MuonEDMPrinterTool> m_printer{
        this,
        "EDMPrinter",
        "Muon::MuonEDMPrinterTool/MuonEDMPrinterTool",
    };  //<! helper printer tool
    ToolHandle<Muon::IMuonPatternCalibration> m_patternCalibration{
        this,
        "MuonPatternCalibration",
        "Muon::MuonPatternCalibration/MuonPatternCalibration",
    };   
    ToolHandle<Muon::IMuonSegmentMaker> m_segmentMaker{
        this,
        "SegmentMaker",
        "Muon::DCMathSegmentMaker/DCMathSegmentMaker",
    };
    ToolHandle<Muon::IMuonClusterSegmentFinder> m_clusterSegMaker{
        this,
        "MuonClusterSegmentFinder",
        "Muon::MuonClusterSegmentFinder/MuonClusterSegmentFinder",
    };
    ToolHandle<Muon::IMuonSegmentOverlapRemovalTool> m_segmentOverlapRemovalTool{
        this,
        "MuonSegmentOverlapRemovalTool",
        "Muon::MuonSegmentOverlapRemovalTool/MuonSegmentOverlapRemovalTool",
    };
    ToolHandle<Muon::IMuonClusterOnTrackCreator> m_clusterCreator{
        this,
        "MuonClusterCreator",
        "Muon::MuonClusterOnTrackCreator/MuonClusterOnTrackCreator",
    };  //<! pointer to muon cluster rio ontrack creator
    ToolHandle<Muon::IMuonNSWSegmentFinderTool> m_clusterSegMakerNSW{
        this,
        "NSWSegmentMaker",
        "",
    };
    ToolHandle<ICscSegmentFinder> m_csc2dSegmentFinder{
        this,
        "Csc2dSegmentMaker",
        "Csc2dSegmentMaker/Csc2dSegmentMaker",
    };
    ToolHandle<ICscSegmentFinder> m_csc4dSegmentFinder{
        this,
        "Csc4dSegmentMaker",
        "Csc4dSegmentMaker/Csc4dSegmentMaker",
    };
    ToolHandle<Muon::IMuonCurvedSegmentCombiner> m_curvedSegmentCombiner{this, "SegmentCombiner",
                                                                       "Muon::MuonCurvedSegmentCombiner/MuonCurvedSegmentCombiner"};

    
    ToolHandle<Muon::IMuonSegmentSelectionTool> m_segmentSelector{this, "SegmentSelector",
                                                                "Muon::MuonSegmentSelectionTool/MuonSegmentSelectionTool"};

    // the following Trk::SegmentCollection MuonSegments are standard MuonSegments, the MuGirl segments are stored in MuonCreatorAlg.h
    SG::WriteHandleKey<Trk::SegmentCollection> m_segmentCollectionKey{
        this,
        "SegmentCollectionName",
        "TrackMuonSegments",
        "Muon Segments",
    };
    SG::WriteHandleKey<Trk::SegmentCollection> m_segmentNSWCollectionKey{ //this collection of segments are used to perform the alignment of the NSW
      this,
        "NSWSegmentCollectionName",
        "TrackMuonNSWSegments",
        "WriteHandleKey for NSW Segments",
    };
    SG::ReadHandleKey<Muon::CscPrepDataContainer> m_cscPrdsKey{
        this,
        "CSC_clusterkey",
        "CSC_Clusters",
        "CSC PRDs",
    };
    SG::ReadHandleKey<Muon::MdtPrepDataContainer> m_mdtPrdsKey{
        this,
        "MDT_PRDs",
        "MDT_DriftCircles",
        "MDT PRDs",
    };
    SG::ReadHandleKey<Muon::RpcPrepDataContainer> m_rpcPrdsKey{
        this,
        "RPC_PRDs",
        "RPC_Measurements",
        "RPC PRDs",
    };
    SG::ReadHandleKey<Muon::TgcPrepDataContainer> m_tgcPrdsKey{
        this,
        "TGC_PRDs",
        "TGC_Measurements",
        "TGC PRDs",
    };
    SG::ReadHandleKey<MuonPatternCombinationCollection> m_patternCollKey{
        this,
        "MuonLayerHoughCombisKey",
        "MuonLayerHoughCombis",
        "Hough combinations",
    };
    SG::ReadHandleKey<PRD_MultiTruthCollection> m_tgcTruth{
        this,
        "TGCTruth",
        "TGC_TruthMap",
        "TGC PRD Multi-truth Collection",
    };
    SG::ReadHandleKey<PRD_MultiTruthCollection> m_rpcTruth{
        this,
        "RPCTruth",
        "RPC_TruthMap",
        "RPC PRD Multi-truth Collection",
    };

    StatusCode createSegmentsWithMDTs(const EventContext& ctx, const Muon::MuonPatternCombination* patt, Trk::SegmentCollection* segs) const;
    
    
    using NSWSegmentCache = Muon::IMuonNSWSegmentFinderTool::SegmentMakingCache;
    void createNSWSegments(const EventContext& ctx, 
                           const Muon::MuonPatternCombination* patt, 
                           NSWSegmentCache& cache) const;
   
    /// Retrieve the raw outputs from the Csc segment makers for the curved combination
    StatusCode createCscSegments(const EventContext& ctx, 
                                std::unique_ptr<MuonSegmentCombinationCollection>& csc2dSegmentCombinations,
                                std::unique_ptr<MuonSegmentCombinationCollection>& csc4dSegmentCombinations) const;

    void appendSegmentsFromCombi(const std::unique_ptr<MuonSegmentCombinationCollection>& combi_coll, 
                                 Trk::SegmentCollection* segments) const;


    Gaudi::Property<bool> m_printSummary{this, "PrintSummary", false};
    Gaudi::Property<bool> m_doTGCClust{this, "doTGCClust", false, "selection flags for cluster based segment finding"};
    Gaudi::Property<bool> m_doRPCClust{this, "doRPCClust", false, "selection flags for cluster based segment finding"};
    Gaudi::Property<bool> m_doClusterTruth{this, "doClusterTruth", false, "selection flags for cluster based segment finding"};

    /// Run segment finding with eta / phi determination
    Gaudi::Property<bool> m_doFullFinder{this, "FullFinder", true}; 
    /// Combined the segments of several multilayers (Only Legacy systems)
    Gaudi::Property<bool> m_runSegCombiner{this, "RunSegmentCombiner", true};
    /// Run the Mdt segment maker (Switched of the NCB systems)
    Gaudi::Property<bool> m_runMdtSegments{this, "doMdtSegments", true};
    /// Run the NSW segment maker
    Gaudi::Property<bool> m_doSTgcSegments{this, "doStgcSegments", true};
    Gaudi::Property<bool> m_doMMSegments{this, "doMMSegments", true};
    /// If switched to true, hits that have been already successfully combined to a segment are removed from 
    /// future searches
    Gaudi::Property<bool> m_removeUsedNswHits{this, "removeUsedNSW", true};
    /// Apply a preselection on the segments
    Gaudi::Property<int> m_segQuality{this, "SegmentQuality", -1};
    
    /// load the container from storegate given a ReadHandleKey. If the key is empty
    /// a nullptr will be returned
    template <class ContType> StatusCode loadFromStoreGate(const EventContext& ctx,
                                                           const SG::ReadHandleKey<ContType>& key,
                                                           const ContType* & cont_ptr) const;
};

#endif
