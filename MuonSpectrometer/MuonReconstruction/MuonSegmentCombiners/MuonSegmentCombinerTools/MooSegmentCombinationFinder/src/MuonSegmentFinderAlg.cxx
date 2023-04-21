/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonSegmentFinderAlg.h"

#include "MuonPattern/MuonPatternChamberIntersect.h"
#include "MuonPattern/MuonPatternCombination.h"
#include "MuonPattern/MuonPatternCombinationCollection.h"
#include "MuonPatternSegmentMaker/MuonPatternCalibration.h"
#include "MuonPrepRawData/MuonCluster.h"
#include "MuonPrepRawData/MuonPrepDataCollection.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonRIO_OnTrack/MuonClusterOnTrack.h"
#include "MuonReadoutGeometry/MuonReadoutElement.h"
#include "MuonSegment/MuonSegment.h"
#include "MuonSegment/MuonSegmentCombination.h"
#include "MuonSegment/MuonSegmentCombinationCollection.h"
#include "TrkParameters/TrackParameters.h"
#include "MuonEDM_AssociationObjects/MuonSegPatAssMap.h"
#include "EventPrimitives/EventPrimitivesToStringConverter.h"

MuonSegmentFinderAlg::MuonSegmentFinderAlg(const std::string& name, ISvcLocator* pSvcLocator) : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode MuonSegmentFinderAlg::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_printer.retrieve());
    ATH_CHECK(m_segmentOverlapRemovalTool.retrieve());
    ATH_CHECK(m_segmentSelector.retrieve(DisableTool{m_segQuality<0}));
    /// MDT segments
    ATH_CHECK(m_patternCalibration.retrieve(DisableTool{!m_runMdtSegments}));
    ATH_CHECK(m_segmentMaker.retrieve(DisableTool{!m_runMdtSegments}));
    ATH_CHECK(m_curvedSegmentCombiner.retrieve(DisableTool{!m_runSegCombiner}));

    ATH_CHECK(m_clusterSegMaker.retrieve(DisableTool{!m_doTGCClust && !m_doRPCClust}));
    
    const bool doNSW = m_doSTgcSegments || m_doMMSegments;
    ATH_CHECK(m_clusterCreator.retrieve(DisableTool{!doNSW}));
    ATH_CHECK(m_clusterSegMakerNSW.retrieve(DisableTool{!doNSW}));   
    ATH_CHECK(m_csc2dSegmentFinder.retrieve(DisableTool{m_cscPrdsKey.empty()}));
    ATH_CHECK(m_csc4dSegmentFinder.retrieve(DisableTool{m_cscPrdsKey.empty()}));
    
    ATH_CHECK(m_segmentCollectionKey.initialize());
    /// Initialize the alignment container in the NSW
    ATH_CHECK(m_segmentNSWCollectionKey.initialize(doNSW && !m_segmentNSWCollectionKey.empty()));
    ATH_CHECK(m_cscPrdsKey.initialize(!m_cscPrdsKey.empty()));  // check for layouts without CSCs
    ATH_CHECK(m_mdtPrdsKey.initialize(m_doTGCClust || m_doRPCClust));
    ATH_CHECK(m_rpcPrdsKey.initialize(m_doRPCClust));
    ATH_CHECK(m_tgcPrdsKey.initialize(m_doTGCClust));
    ATH_CHECK(m_patternCollKey.initialize());
    ATH_CHECK(m_tgcTruth.initialize(m_doClusterTruth));
    ATH_CHECK(m_rpcTruth.initialize(m_doClusterTruth));

    return StatusCode::SUCCESS;
}

StatusCode MuonSegmentFinderAlg::execute(const EventContext& ctx) const {
    
    NSWSegmentCache nswCache{};
    nswCache.buildQuads = !m_segmentNSWCollectionKey.empty();

    std::unique_ptr<Trk::SegmentCollection> segmentContainer = std::make_unique<Trk::SegmentCollection>();
    std::unique_ptr<Trk::SegmentCollection> nswSegmentContainer = !m_segmentNSWCollectionKey.empty() ? std::make_unique<Trk::SegmentCollection>()
                                                                                                     : nullptr;   
    
    
    const MuonPatternCombinationCollection* patternColl{nullptr};
    ATH_CHECK(loadFromStoreGate(ctx, m_patternCollKey, patternColl));    
    ATH_MSG_DEBUG("Processing the pattern collections with  " << patternColl->size() << " Collections ");

    
    std::unique_ptr<MuonSegmentCombinationCollection> combiSegColl = m_runSegCombiner ? std ::make_unique<MuonSegmentCombinationCollection>() 
                                                                                      : nullptr;

    std::unique_ptr<Muon::MuonSegmentCombPatternCombAssociationMap> patternAssocMap = 
                                                             m_runSegCombiner ? std::make_unique<Muon::MuonSegmentCombPatternCombAssociationMap>()
                                                                              : nullptr;

    for (const Muon::MuonPatternCombination* patt :  *patternColl) {
        ATH_MSG_DEBUG("Working on pattern combination " << m_printer->print(*patt));
        // check the technology & call the corresponding segment finder        
        
        if (m_runSegCombiner) {
            std::unique_ptr<Trk::SegmentCollection> segsToComb = std::make_unique<Trk::SegmentCollection>(SG::VIEW_ELEMENTS);
            ATH_CHECK(createSegmentsWithMDTs(ctx, patt, segsToComb.get()));

            std::unique_ptr<Muon::MuonSegmentCombination> combContainer = std::make_unique<Muon::MuonSegmentCombination>();
            
            /// Transform the segment collection into a vector of unique_ptrs 
            std::unique_ptr<std::vector<std::unique_ptr<Muon::MuonSegment>>> muonSegVec = std::make_unique<std::vector<std::unique_ptr<Muon::MuonSegment>>>();
            for (Trk::Segment* trk_seg : *segsToComb) {
                Muon::MuonSegment* muo_seg = static_cast<Muon::MuonSegment*>(trk_seg);
                ATH_MSG_VERBOSE("Found new segment for combination "<<std::endl<<m_printer->print(muo_seg->containedMeasurements()));
                muonSegVec->emplace_back(muo_seg);
            }
            combContainer->addSegments(std::move(muonSegVec));
            if (!segsToComb->empty()) {
                patternAssocMap->insert(std::make_pair(combContainer.get(), patt));
                combiSegColl->push_back(combContainer.release());
            }       
        } else {
            ATH_CHECK(createSegmentsWithMDTs(ctx, patt, segmentContainer.get()));
        }
       
        createNSWSegments(ctx, patt, nswCache);
        
        /// Move the segments into the output
        segmentContainer->insert(segmentContainer->end(), std::make_move_iterator(nswCache.constructedSegs.begin()),
                                                          std::make_move_iterator(nswCache.constructedSegs.end()));
        
        nswCache.constructedSegs.clear();

        if (!nswSegmentContainer) continue;
        nswSegmentContainer->insert(nswSegmentContainer->end(), std::make_move_iterator(nswCache.quadSegs.begin()),
                                                                std::make_move_iterator(nswCache.quadSegs.end()));
        nswCache.quadSegs.clear();
               
    }  // end loop on pattern combinations

    // do cluster based segment finding
    if (m_doTGCClust || m_doRPCClust) {
        const Muon::MdtPrepDataContainer* mdtPrds{nullptr};
        const PRD_MultiTruthCollection* tgcTruthColl{nullptr};
        const PRD_MultiTruthCollection* rpcTruthColl{nullptr};
        const Muon::TgcPrepDataContainer* tgcPrdCont{nullptr};
        const Muon::RpcPrepDataContainer* rpcPrdCont{nullptr};
        ATH_CHECK(loadFromStoreGate(ctx, m_rpcPrdsKey, rpcPrdCont));
        ATH_CHECK(loadFromStoreGate(ctx, m_tgcPrdsKey, tgcPrdCont));
        ATH_CHECK(loadFromStoreGate(ctx,m_mdtPrdsKey, mdtPrds));
        ATH_CHECK(loadFromStoreGate(ctx,m_tgcTruth, tgcTruthColl));
        ATH_CHECK(loadFromStoreGate(ctx,m_rpcTruth, rpcTruthColl));
       
        m_clusterSegMaker->getClusterSegments(mdtPrds, rpcPrdCont, tgcPrdCont, tgcTruthColl,
                                              rpcTruthColl, segmentContainer.get());
    }

    m_segmentOverlapRemovalTool->removeDuplicates(*segmentContainer);

    std::unique_ptr<MuonSegmentCombinationCollection> csc2dSegmentCombinations{}, csc4dSegmentCombinations{};
    ATH_CHECK(createCscSegments(ctx, csc2dSegmentCombinations, csc4dSegmentCombinations));
    if (m_runSegCombiner) {        
        if (!csc2dSegmentCombinations) csc2dSegmentCombinations = std::make_unique<MuonSegmentCombinationCollection>();
        if (!csc4dSegmentCombinations) csc4dSegmentCombinations = std::make_unique<MuonSegmentCombinationCollection>();
        
        std::unique_ptr<MuonSegmentCombinationCollection> curvedSegmentCombinations =  
              m_curvedSegmentCombiner->combineSegments(*combiSegColl, 
                                                       *csc4dSegmentCombinations,
                                                       *csc2dSegmentCombinations, patternAssocMap.get());
        appendSegmentsFromCombi(curvedSegmentCombinations, segmentContainer.get());
    
    } else {
        appendSegmentsFromCombi(csc4dSegmentCombinations, segmentContainer.get());        
    }
    
    if (msgLvl(MSG::VERBOSE)){
        ATH_MSG_VERBOSE("Number of segments found " << segmentContainer->size());
        for (Trk::Segment* tseg : *segmentContainer) {
            const Muon::MuonSegment* mseg{dynamic_cast<Muon::MuonSegment*>(tseg)};
            ATH_MSG_VERBOSE(m_printer->print(*mseg)<<std::endl<<m_printer->print(mseg->containedMeasurements())<<std::endl);
        }
    }

    /// Get rid of all the duplicates in the segment container
    ATH_MSG_DEBUG("segments before overlap removal: " << segmentContainer->size());
    m_segmentOverlapRemovalTool->removeDuplicates(*segmentContainer);
    ATH_MSG_DEBUG(" Segments after overlap removal: " << segmentContainer->size());

    SG::WriteHandle<Trk::SegmentCollection> handle(m_segmentCollectionKey, ctx);
    ATH_CHECK(handle.record(std::move(segmentContainer)));
    
    if (!m_segmentNSWCollectionKey.empty()) {
        m_segmentOverlapRemovalTool->removeDuplicates(*nswSegmentContainer);
        SG::WriteHandle<Trk::SegmentCollection> handle_segNSW(m_segmentNSWCollectionKey, ctx);
        ATH_CHECK(handle_segNSW.record(std::move(nswSegmentContainer)));  
    }
    return StatusCode::SUCCESS;
}  // execute

StatusCode MuonSegmentFinderAlg::createCscSegments(const EventContext& ctx, 
                                std::unique_ptr<MuonSegmentCombinationCollection>& csc2dSegmentCombinations,
                                std::unique_ptr<MuonSegmentCombinationCollection>& csc4dSegmentCombinations) const {
    
    const Muon::CscPrepDataContainer* cscPrds{nullptr};
    ATH_CHECK(loadFromStoreGate(ctx,m_cscPrdsKey, cscPrds));
    if (!cscPrds) return StatusCode::SUCCESS;

    std::vector<const Muon::CscPrepDataCollection*> cscCols;
    std::copy_if(cscPrds->begin(),cscPrds->end(), std::back_inserter(cscCols), [](const Muon::CscPrepDataCollection* coll) {return !coll->empty();});
    ATH_MSG_DEBUG("Retrieved CscPrepDataContainer " << cscCols.size());
    if (cscCols.empty()) return StatusCode::SUCCESS;
    
    csc2dSegmentCombinations = m_csc2dSegmentFinder->find(cscCols, ctx);
    if (!csc2dSegmentCombinations) return StatusCode::SUCCESS;

    csc4dSegmentCombinations = m_csc4dSegmentFinder->find(*csc2dSegmentCombinations, ctx);
    return StatusCode::SUCCESS;
}
void MuonSegmentFinderAlg::appendSegmentsFromCombi(const std::unique_ptr<MuonSegmentCombinationCollection>& combi_coll, 
                                 Trk::SegmentCollection* segmentContainer) const {
    if (!combi_coll) return;
     /// Push back the output containers
    for (const Muon::MuonSegmentCombination* combi: *combi_coll) {
        if (!combi) {
            ATH_MSG_WARNING(" empty MuonSegmentCombination!!! ");
            continue;
        }
        const unsigned int nstations = combi->numberOfStations();
        const bool useEta = combi->useStripsInSegment(1);
        const bool usePhi = combi->useStripsInSegment(0);

        // loop over chambers in combi and extract segments
        for (unsigned int i = 0; i < nstations; ++i) {
            // loop over segments in station
            Muon::MuonSegmentCombination::SegmentVec* segments = combi->stationSegments(i);
            // check if not empty
            if (!segments || segments->empty()) continue;
            // loop over new segments, copy them into collection
            for (std::unique_ptr<Muon::MuonSegment>& seg : *segments) {                
                if(m_segQuality >=0 && !m_segmentSelector->select(*seg, false, m_segQuality, useEta, usePhi)) continue;                
                ATH_MSG_VERBOSE("Append segment "<<std::endl<<m_printer->print(seg->containedMeasurements()));
                segmentContainer->push_back(std::move(seg));
            }
                
        }
    }
}
void MuonSegmentFinderAlg::createNSWSegments(const EventContext& ctx, 
                           const Muon::MuonPatternCombination* patt, 
                           NSWSegmentCache& cache) const {
    // turn the PRD into MuonCluster
    if (!m_doSTgcSegments && !m_doMMSegments) return;
    std::map<int, std::vector<std::unique_ptr<const Muon::MuonClusterOnTrack>> > clustersPerSector;

    
    for (const Muon::MuonPatternChamberIntersect&  it :patt->chamberData()) {
        if (it.prepRawDataVec().empty()) continue;
        
        const Identifier id = it.prepRawDataVec().front()->identify();
        const int sector = m_idHelperSvc->sector(id);
        /// Constrain to NSW hits
        if (!m_idHelperSvc->isMM(id) && !m_idHelperSvc->issTgc(id)) continue;
        for (const Trk::PrepRawData* pit : it.prepRawDataVec()) {
            const Muon::MuonCluster* cl = dynamic_cast<const Muon::MuonCluster*>(pit);
            if (!cl) continue;           
            else if (!m_doMMSegments && m_idHelperSvc->isMM(cl->identify())) continue;
            else if (!m_doSTgcSegments && m_idHelperSvc->issTgc(cl->identify())) continue;
            const Muon::MuonClusterOnTrack* newCluster = m_clusterCreator->createRIO_OnTrack(*cl, cl->globalPosition());
            if (!newCluster) continue;
            std::vector<std::unique_ptr<const Muon::MuonClusterOnTrack>>& clusters = clustersPerSector[sector];
            clusters.emplace_back(newCluster);          
        }
    }    
    for (auto&[sector, clusters] :clustersPerSector) {
        ATH_MSG_VERBOSE("Run segment making on sector "<<sector);
        cache.inputClust = std::move(clusters);
        m_clusterSegMakerNSW->find(ctx, cache);
    }
}

StatusCode MuonSegmentFinderAlg::createSegmentsWithMDTs(const EventContext& ctx,
                                                        const Muon::MuonPatternCombination* patcomb, 
                                                        Trk::SegmentCollection* segs) const {
    
    if (!m_runMdtSegments) {
        ATH_MSG_DEBUG("Do not search segments in the Mdt part of the muon spectrometer");
        return StatusCode::SUCCESS;
    }
   
    bool hasPhiMeasurements = m_patternCalibration->checkForPhiMeasurements(*patcomb);
    Muon::IMuonPatternCalibration::ROTsPerRegion hitsPerRegion{};
    ATH_CHECK(m_patternCalibration->calibrate(ctx, *patcomb, hitsPerRegion));
    using MdtVec = Muon::IMuonPatternCalibration::MdtVec;
    using ROTRegion = Muon::IMuonPatternCalibration::ROTRegion; 
    for (const ROTRegion& region : hitsPerRegion) {
        for (const MdtVec& mdts : region.mdts()) {
            if (mdts.empty()) continue;
            ATH_MSG_VERBOSE("Calling segment finding for sector " << m_idHelperSvc->chamberNameString(mdts.front()->identify()));
            // fit the segments
            if (m_doFullFinder) {
                if (msgLvl(MSG::VERBOSE)){
                    std::vector<const Trk::MeasurementBase*> meas {};
                    meas.insert(meas.end(), mdts.begin(), mdts.end());
                    meas.insert(meas.end(), region.clusters().begin(), region.clusters().end());
                    ATH_MSG_VERBOSE("Call segment maker with "<<Amg::toString(region.regionPos)
                                                             << " "<<Amg::toString(region.regionDir)                
                                                             <<std::endl<<m_printer->print(meas));
                
                }
                m_segmentMaker->find(region.regionPos, region.regionDir, mdts, region.clusters(), hasPhiMeasurements, segs,
                                    region.regionDir.mag());
            } else {
                std::vector<const Trk::RIO_OnTrack*> rios;
                rios.insert(rios.begin(), mdts.begin(), mdts.end());               
                m_segmentMaker->find(rios, segs);
            }
        }  // end loop on hits per region
    }     
    return StatusCode::SUCCESS;    
}
template <class ContType> StatusCode MuonSegmentFinderAlg::loadFromStoreGate(const EventContext& ctx,
                                                           const SG::ReadHandleKey<ContType>& key,
                                                           const ContType* & cont_ptr) const {
    if (key.empty()){
        ATH_MSG_VERBOSE("Empty key given for "<<typeid(ContType).name()<<".");
        cont_ptr = nullptr;
        return StatusCode::SUCCESS;
    }
    SG::ReadHandle<ContType> readHandle{key, ctx};
    if (!readHandle.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve "<<key.fullKey()<<" from store gate");
        return StatusCode::FAILURE;
    }
    cont_ptr = readHandle.cptr();        
    return StatusCode::SUCCESS;
}
