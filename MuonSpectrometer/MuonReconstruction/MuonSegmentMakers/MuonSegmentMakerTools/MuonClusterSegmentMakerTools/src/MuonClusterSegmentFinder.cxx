/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonClusterSegmentFinder.h"

#include "AtlasHepMC/GenEvent.h"
#include "GaudiKernel/ConcurrencyFlags.h"
#include "MuonLinearSegmentMakerUtilities/ClusterAnalysis.h"
#include "MuonPrepRawData/MdtPrepDataCollection.h"
#include "MuonRIO_OnTrack/MdtDriftCircleOnTrack.h"
#include "MuonRIO_OnTrack/MuonClusterOnTrack.h"
#include "TFile.h"
#include "TTree.h"

namespace {
    bool sortfunctionMB(const Trk::MeasurementBase* i, const Trk::MeasurementBase* j) {
        return (std::abs(i->globalPosition().z()) < std::abs(j->globalPosition().z()));
    }

}  // namespace
namespace Muon {

    MuonClusterSegmentFinder::MuonClusterSegmentFinder(const std::string& type, const std::string& name, const IInterface* parent) :
        AthAlgTool(type, name, parent) {
        declareInterface<IMuonClusterSegmentFinder>(this);
        declareProperty("DoNtuple", m_doNtuple = false);
    }

    StatusCode MuonClusterSegmentFinder::finalize() {
        if (m_doNtuple) {
            TDirectory* cdir = gDirectory;
            m_tree->m_file->cd();
            m_tree->m_tree->Write();
            m_tree->m_file->Write();
            m_tree->m_file->Close();
            delete m_tree->m_ntuple;
            gDirectory = cdir;
        }
        return StatusCode::SUCCESS;
    }

    StatusCode MuonClusterSegmentFinder::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_printer.retrieve());
        ATH_CHECK(m_layerHashProvider.retrieve());
        ATH_CHECK(m_muonPRDSelectionTool.retrieve());
        ATH_CHECK(m_segmentMaker.retrieve());
        ATH_CHECK(m_clusterTool.retrieve());
        ATH_CHECK(m_clusterCreator.retrieve());
        ATH_CHECK(m_trackToSegmentTool.retrieve());
        ATH_CHECK(m_slTrackFitter.retrieve());
        ATH_CHECK(m_ambiguityProcessor.retrieve());
        ATH_CHECK(m_edmHelperSvc.retrieve());
        ATH_CHECK(m_trackCleaner.retrieve());
        ATH_CHECK(m_segmentOverlapRemovalTool.retrieve());

        /// Ensure that the n-tuple is only written in the single thread mode
        m_doNtuple &= Gaudi::Concurrency::ConcurrencyFlags::numThreads() <= 1;
        if (m_doNtuple) {
            TDirectory* cdir = gDirectory;
            m_tree = std::make_unique<Tree>();
            m_tree->m_file = new TFile("ClusterNtuple.root", "RECREATE");
            m_tree->m_tree = new TTree("clusters", "clusters");
            m_tree->m_ntuple = new ClusterSeg::ClusterNtuple();
            m_tree->m_ntuple->initForWrite(*m_tree->m_tree);
            gDirectory = cdir;
        }

        return StatusCode::SUCCESS;
    }

    bool MuonClusterSegmentFinder::matchTruth(const PRD_MultiTruthCollection& truthCol, const Identifier& id, int& bcode) const {
        typedef PRD_MultiTruthCollection::const_iterator iprdt;
        std::pair<iprdt, iprdt> range = truthCol.equal_range(id);
        // Loop over particles contributing to this cluster
        for (iprdt i = range.first; i != range.second; ++i) {
            if (!i->second.isValid()) {
                ATH_MSG_INFO("Unexpected invalid HepMcParticleLink in PRD_MultiTruthCollection");
            } else {
                const HepMcParticleLink& link = i->second;
                if (link.cptr()) {
                    if (std::abs(link.cptr()->pdg_id()) == 13) {
                        bcode = link.barcode();
                        return true;
                    }
                }
            }
        }
        return false;
    }

    void MuonClusterSegmentFinder::getClusterSegments(const Muon::MdtPrepDataContainer* mdtPrdCont,
                                                      std::vector<const Muon::TgcPrepDataCollection*>* tgcCols,
                                                      std::vector<const Muon::RpcPrepDataCollection*>* rpcCols,
                                                      const PRD_MultiTruthCollection* tgcTruthColl,
                                                      const PRD_MultiTruthCollection* rpcTruthColl, Trk::SegmentCollection* segColl) const {
        if (tgcCols) {
            Muon::TgcPrepDataContainer* clusterPRD = new Muon::TgcPrepDataContainer(m_idHelperSvc->tgcIdHelper().module_hash_max());
            for (const auto *const tgcCol : *tgcCols) {
                Muon::TgcPrepDataCollection* clusteredCol = m_clusterTool->cluster(*tgcCol);
                if (clusteredCol) clusterPRD->addCollection(clusteredCol, tgcCol->identifyHash()).ignore();
            }
            std::vector<const Muon::TgcPrepDataCollection*> theTGCs;
            std::transform(clusterPRD->begin(), clusterPRD->end(), std::back_inserter(theTGCs), [](auto* c){return c;});
            
            findSegments(theTGCs, mdtPrdCont, segColl, tgcTruthColl);
        }  // end if TGC

        if (rpcCols) {
            Muon::RpcPrepDataContainer* clusterPRD = new Muon::RpcPrepDataContainer(m_idHelperSvc->rpcIdHelper().module_hash_max());
            for (const auto *const rpcCol : *rpcCols) {
                Muon::RpcPrepDataCollection* clusteredCol = m_clusterTool->cluster(*rpcCol);
                if (clusteredCol) clusterPRD->addCollection(clusteredCol, rpcCol->identifyHash()).ignore();
            }
            std::vector<const Muon::RpcPrepDataCollection*> theRPCs;
            std::transform(clusterPRD->begin(), clusterPRD->end(), std::back_inserter(theRPCs), [](auto* c){return c;});
            
            findSegments(theRPCs, mdtPrdCont, segColl, rpcTruthColl);
        }  // end if RPC
    }

    void MuonClusterSegmentFinder::getClusterSegments(const Muon::MdtPrepDataContainer* mdtPrdCont,
                                                      const Muon::RpcPrepDataContainer* rpcPrdCont,
                                                      const Muon::TgcPrepDataContainer* tgcPrdCont,
                                                      const PRD_MultiTruthCollection* tgcTruthColl,
                                                      const PRD_MultiTruthCollection* rpcTruthColl, Trk::SegmentCollection* segColl) const {
        if (tgcPrdCont) {
            Muon::TgcPrepDataContainer* clusterPRD = m_clusterTool->cluster(*tgcPrdCont);
            std::vector<const Muon::TgcPrepDataCollection*> theTGCs;
            std::transform(clusterPRD->begin(), clusterPRD->end(), std::back_inserter(theTGCs), [](auto* c){return c;});
            findSegments(theTGCs, mdtPrdCont, segColl, tgcTruthColl);
        }  // end if TGC

        if (rpcPrdCont) {
            Muon::RpcPrepDataContainer* clusterPRD = m_clusterTool->cluster(*rpcPrdCont);
            std::vector<const Muon::RpcPrepDataCollection*> theRPCs;
            std::transform(clusterPRD->begin(), clusterPRD->end(), std::back_inserter(theRPCs), [](auto* c){return c;});
            

            findSegments(theRPCs, mdtPrdCont, segColl, rpcTruthColl);
        }  // end if rpc
    }

    void MuonClusterSegmentFinder::findSegments(std::vector<const TgcPrepDataCollection*>& tgcCols,
                                                const Muon::MdtPrepDataContainer* mdtPrdCont, Trk::SegmentCollection* segColl,
                                                const PRD_MultiTruthCollection* tgcTruthColl) const {
        ATH_MSG_INFO("Executing " << name() << "...");
        ATH_MSG_DEBUG("start with " << segColl->size() << " segments");

        candEvent* thisEvent = new candEvent;

        makeClusterVecs(tgcTruthColl, tgcCols, thisEvent);

        ClusterSeg::ClusterAnalysis theAnalysis;
        ATH_MSG_DEBUG("the size of Clust is " << thisEvent->Clust().size());
        std::vector<std::vector<ClusterSeg::SpacePoint>> sPoints = theAnalysis.analyse(thisEvent->Clust());

        processSpacePoints(thisEvent, sPoints);
        if (!thisEvent->segTrkColl()->empty())
            ATH_MSG_DEBUG("it made at least one track ");
        else
            ATH_MSG_DEBUG("processSpacePoints didn't make anything");

        std::map<int, bool> themap;
        findOverlap(themap, thisEvent);
        resolveCollections(themap, thisEvent);

        getSegments(thisEvent, mdtPrdCont, segColl);

        ATH_MSG_DEBUG("now have " << segColl->size() << " segments");

        if (m_doNtuple) {
            Tree& t = getTree();
            t.m_ntuple->fill(thisEvent->Clust());
            t.m_tree->Fill();
        }
        delete thisEvent;
    }

    void MuonClusterSegmentFinder::findSegments(std::vector<const RpcPrepDataCollection*>& rpcCols,
                                                const Muon::MdtPrepDataContainer* mdtPrdCont, Trk::SegmentCollection* segColl,
                                                const PRD_MultiTruthCollection* rpcTruthColl) const {
        ATH_MSG_INFO("Executing " << name() << "...");
        ATH_MSG_DEBUG("start with " << segColl->size() << " segments");

        candEvent* thisEvent = new candEvent;

        makeClusterVecs(rpcTruthColl, rpcCols, thisEvent);

        ClusterSeg::ClusterAnalysis theAnalysis;
        ATH_MSG_DEBUG("the size of Clust is " << thisEvent->Clust().size());
        std::vector<std::vector<ClusterSeg::SpacePoint>> sPoints = theAnalysis.analyse(thisEvent->Clust());

        processSpacePoints(thisEvent, sPoints);
        if (!thisEvent->segTrkColl()->empty())
            ATH_MSG_DEBUG("it made at least one track ");
        else
            ATH_MSG_DEBUG("processSpacePoints didn't make anything");

        std::map<int, bool> themap;
        findOverlap(themap, thisEvent);
        resolveCollections(themap, thisEvent);

        getSegments(thisEvent, mdtPrdCont, segColl);

        ATH_MSG_DEBUG("now have " << segColl->size() << " segments");

        if (m_doNtuple) {
            Tree& t = getTree();
            t.m_ntuple->fill(thisEvent->Clust());
            t.m_tree->Fill();
        }
        delete thisEvent;
    }

    void MuonClusterSegmentFinder::findOverlap(std::map<int, bool>& themap, candEvent* theEvent) {
        CompareMuonSegmentKeys compareKeys{};
        if (theEvent->keyVector().size() > 1) {
            for (unsigned int i = 0; i < theEvent->keyVector().size(); i++) { themap.insert(std::pair<int, bool>(i, true)); }
            for (unsigned int i = 0; i < theEvent->keyVector().size() - 1; i++) {
                for (unsigned int j = i + 1; j < theEvent->keyVector().size(); j++) {
                    CompareMuonSegmentKeys::OverlapResult overlap = compareKeys(theEvent->keyVector()[i], theEvent->keyVector()[j]);
                    if (overlap == 0 || overlap == 2) themap[j] = false;
                    if (overlap == 1) themap[i] = false;
                }
            }
        }
    }

    Trk::Track* MuonClusterSegmentFinder::fit(const std::vector<const Trk::MeasurementBase*>& vec2,
                                              const Trk::TrackParameters& startpar) const {
        const EventContext& ctx = Gaudi::Hive::currentContext();
        std::unique_ptr<Trk::Track> segtrack{m_slTrackFitter->fit(ctx, vec2, startpar, false, Trk::nonInteracting)};

        if (segtrack) {
            ATH_MSG_DEBUG("segment fit succeeded");

            std::unique_ptr<Trk::Track> cleanedTrack = m_trackCleaner->clean(*segtrack, ctx);
            if (cleanedTrack && !(*cleanedTrack->perigeeParameters() == *segtrack->perigeeParameters())) {
                // using release until the entire code can be migrated to use smart pointers
                segtrack.swap(cleanedTrack);
            } else {
                ATH_MSG_DEBUG("track remains unchanged");
            }

            if (!m_edmHelperSvc->goodTrack(*segtrack, 30) && vec2.size() > 4) {
                ATH_MSG_DEBUG("bad segment fit:");
                if (segtrack->fitQuality())
                    ATH_MSG_DEBUG("with chi^2/nDoF = " << segtrack->fitQuality()->chiSquared() << "/"
                                                       << segtrack->fitQuality()->numberDoF());

                return nullptr;
            }
        }
        return segtrack.release();
    }

    void MuonClusterSegmentFinder::makeClusterVecs(const std::vector<const Muon::MuonClusterOnTrack*>& clustCol,
                                                   candEvent* theEvent) const {
        for (const MuonClusterOnTrack* clust : clustCol) {
            MuonStationIndex::PhiIndex pIndex = m_idHelperSvc->phiIndex(clust->identify());
            bool tmatch(false);
            int barcode(0);
            if (m_idHelperSvc->measuresPhi(clust->identify())) {
                theEvent->clusters().push_back(clust);
                ClusterSeg::Cluster* cluster =
                    new ClusterSeg::Cluster(clust->globalPosition().x(), clust->globalPosition().y(), clust->globalPosition().z(), true,
                                            MuonStationIndex::TechnologyIndex::TGC, pIndex, tmatch, barcode);
                theEvent->Clust().push_back(cluster);
            } else {
                theEvent->clusters().push_back(clust);
                ClusterSeg::Cluster* cluster =
                    new ClusterSeg::Cluster(clust->globalPosition().x(), clust->globalPosition().y(), clust->globalPosition().z(), false,
                                            MuonStationIndex::TechnologyIndex::TGC, pIndex, tmatch, barcode);
                theEvent->Clust().push_back(cluster);
            }
        }
    }

    void MuonClusterSegmentFinder::makeClusterVecs(const PRD_MultiTruthCollection* truthCollectionTGC,
                                                   const std::vector<const TgcPrepDataCollection*>& tgcCols, candEvent* theEvent) const {
        for (std::vector<const TgcPrepDataCollection*>::const_iterator colIt = tgcCols.begin(); colIt != tgcCols.end(); ++colIt) {
            TgcPrepDataCollection::const_iterator pit = (*colIt)->begin();
            for (; pit != (*colIt)->end(); ++pit) {
                const MuonCluster* cl = *pit;
                if (!cl) continue;
                bool tmatch = false;
                int barcode = 0;
                if (truthCollectionTGC) {
                    const Identifier& id = (*pit)->identify();
                    tmatch = matchTruth(*truthCollectionTGC, id, barcode);
                }
                const MuonClusterOnTrack* clust = m_clusterCreator->createRIO_OnTrack(*cl, cl->globalPosition());
                MuonStationIndex::PhiIndex pIndex = m_idHelperSvc->phiIndex(clust->identify());
                if (m_idHelperSvc->measuresPhi(clust->identify())) {
                    theEvent->clusters().push_back(clust);
                    ClusterSeg::Cluster* cluster =
                        new ClusterSeg::Cluster(clust->globalPosition().x(), clust->globalPosition().y(), clust->globalPosition().z(), true,
                                                MuonStationIndex::TechnologyIndex::TGC, pIndex, tmatch, barcode);
                    theEvent->Clust().push_back(cluster);
                } else {
                    theEvent->clusters().push_back(clust);
                    ClusterSeg::Cluster* cluster =
                        new ClusterSeg::Cluster(clust->globalPosition().x(), clust->globalPosition().y(), clust->globalPosition().z(),
                                                false, MuonStationIndex::TechnologyIndex::TGC, pIndex, tmatch, barcode);
                    theEvent->Clust().push_back(cluster);
                }
            }
        }
    }

    void MuonClusterSegmentFinder::makeClusterVecs(const PRD_MultiTruthCollection* truthCollectionRPC,
                                                   const std::vector<const RpcPrepDataCollection*>& rpcCols, candEvent* theEvent) const {
        for (std::vector<const RpcPrepDataCollection*>::const_iterator colIt = rpcCols.begin(); colIt != rpcCols.end(); ++colIt) {
            RpcPrepDataCollection::const_iterator pit = (*colIt)->begin();
            for (; pit != (*colIt)->end(); ++pit) {
                const MuonCluster* cl = *pit;
                if (!cl) continue;
                bool tmatch = false;
                int barcode = 0;
                if (truthCollectionRPC) {
                    const Identifier& id = (*pit)->identify();
                    tmatch = matchTruth(*truthCollectionRPC, id, barcode);
                }
                const MuonClusterOnTrack* clust = m_clusterCreator->createRIO_OnTrack(*cl, cl->globalPosition());
                MuonStationIndex::PhiIndex pIndex = m_idHelperSvc->phiIndex(clust->identify());
                if (m_idHelperSvc->measuresPhi(clust->identify())) {
                    theEvent->clusters().push_back(clust);
                    ClusterSeg::Cluster* cluster =
                        new ClusterSeg::Cluster(clust->globalPosition().x(), clust->globalPosition().y(), clust->globalPosition().z(), true,
                                                MuonStationIndex::TechnologyIndex::RPC, pIndex, tmatch, barcode);
                    theEvent->Clust().push_back(cluster);
                } else {
                    theEvent->clusters().push_back(clust);
                    ClusterSeg::Cluster* cluster =
                        new ClusterSeg::Cluster(clust->globalPosition().x(), clust->globalPosition().y(), clust->globalPosition().z(),
                                                false, MuonStationIndex::TechnologyIndex::RPC, pIndex, tmatch, barcode);
                    theEvent->Clust().push_back(cluster);
                }
            }
        }
    }

    void MuonClusterSegmentFinder::processSpacePoints(candEvent* theEvent,
                                                      std::vector<std::vector<ClusterSeg::SpacePoint>>& sPoints) const {
        bool truthSeed(true);
        int fakeCounter(0);
        unsigned int barcodeCounter(0);
        int barcodeVal(0);

        for (const std::vector<ClusterSeg::SpacePoint>& sit : sPoints) {
            if (sit.size() < 2) continue;
            std::vector<const MuonClusterOnTrack*> vec1;
            std::vector<const Trk::MeasurementBase*> vec2;

            for (unsigned int i = 0; i < sit.size(); i++) {
                int spEit = sit[i].eit();
                vec1.push_back(theEvent->clusters()[spEit]);
                vec2.push_back(theEvent->clusters()[spEit]);
                int spPit = sit[i].pit();
                vec1.push_back(theEvent->clusters()[spPit]);
                vec2.push_back(theEvent->clusters()[spPit]);
                if (!(sit[i].isMatch()))
                    fakeCounter++;
                else if (barcodeCounter == 0) {
                    barcodeVal = sit[i].barcode();
                    barcodeCounter++;
                } else if (barcodeVal == sit[i].barcode())
                    barcodeCounter++;
            }

            if (fakeCounter != 0) truthSeed = false;
            std::sort(vec2.begin(), vec2.end(), sortfunctionMB);
            Trk::TrackParameters* startpar = nullptr;
            Amg::Vector3D gp(sit.front().x(), sit.front().y(), sit.front().z());
            Amg::Vector3D gd(sit.back().x() - sit.front().x(), sit.back().y() - sit.front().y(), sit.back().z() - sit.front().z());
            Amg::Vector3D perpos = gp + -10 * (gd.unit());
            if (perpos.dot(gd) < 0) gd = -1 * gd;
            startpar = new Trk::Perigee(perpos, gd, 0, perpos);
            ATH_MSG_DEBUG("It is starting a fit with " << vec2.size() << "Measurement Base elements and " << startpar);
            Trk::Track* segtrack = fit(vec2, *startpar);
            delete startpar;
            if (segtrack) {
                MuonSegmentKey keyEntry = MuonSegmentKey(vec2);
                theEvent->keyVector().push_back(keyEntry);
                double chi2 = segtrack->fitQuality()->chiSquared();
                double dof = segtrack->fitQuality()->doubleNumberDoF();
                if (m_tree) {
                    Tree& t = getTree();
                    if (truthSeed) t.m_ntuple->fill(chi2 / dof, ClusterSeg::FillType::chi2T);
                    t.m_ntuple->fill(chi2 / dof, ClusterSeg::FillType::chi2);
                }
                ATH_MSG_DEBUG("the chi2 is " << chi2 << "the dof are " << dof << " and the chi2/dof is " << chi2 / dof);
                theEvent->segTrkColl()->push_back(segtrack);
                theEvent->trackSeeds().emplace_back(gp, gd);
                theEvent->hits().push_back(vec1);
            } else {
                ATH_MSG_DEBUG("segment fit failed");
            }
        }
    }

    void MuonClusterSegmentFinder::resolveCollections(const std::map<int, bool>& themap, candEvent* theEvent) const {
        for (unsigned int i = 0; i < theEvent->keyVector().size(); i++) {
            if (themap.at(i)) {
                theEvent->resolvedTrackSeeds().emplace_back(theEvent->trackSeeds()[i].first, theEvent->trackSeeds()[i].second);
                theEvent->resolvedhits().push_back(theEvent->hits()[i]);
                theEvent->resolvedTracks()->push_back(new Trk::Track(*(theEvent->segTrkColl()->at(i))));
            }
        }
        if (theEvent->keyVector().size() == 1) {
            theEvent->resolvedTrackSeeds().emplace_back(theEvent->trackSeeds()[0].first, theEvent->trackSeeds()[0].second);
            theEvent->resolvedhits().push_back(theEvent->hits()[0]);
            theEvent->resolvedTracks()->push_back(new Trk::Track(*(theEvent->segTrkColl()->at(0))));
        }
        ATH_MSG_DEBUG("Resolved track candidates: old size " << theEvent->segTrkColl()->size() << " new size "
                                                             << theEvent->resolvedTracks()->size());
    }

    void MuonClusterSegmentFinder::getSegments(candEvent* theEvent, const Muon::MdtPrepDataContainer* mdtPrdCont,
                                               Trk::SegmentCollection* segColl) const {
        std::vector<const Muon::MuonSegment*> appendSegments;

        const std::vector<const Muon::MuonClusterOnTrack*> MCOTs;
        for (unsigned int i = 0; i < theEvent->resolvedTracks()->size(); i++) {
            const DataVector<const Trk::TrackParameters>* tpVec = theEvent->resolvedTracks()->at(i)->trackParameters();
            if (!tpVec || tpVec->empty() || !tpVec->front()) continue;
            const Trk::TrackParameters& startPars = *tpVec->front();

            const std::vector<const MuonClusterOnTrack*>& MCOTs = theEvent->resolvedhits()[i];
            if (MCOTs.empty()) continue;

            const Identifier& id = MCOTs.front()->identify();
            MuonStationIndex::DetectorRegionIndex regionIndex = m_idHelperSvc->regionIndex(id);
            MuonStationIndex::LayerIndex layerIndex = m_idHelperSvc->layerIndex(id);

            MuonLayerSurface::SurfacePtr surfacePtr(startPars.associatedSurface().clone());
            std::shared_ptr<const Trk::TrackParameters> parsPtr(startPars.clone());

            // get sectors and loop over them
            std::vector<int> sectors;
            theEvent->sectorMapping().getSectors(startPars.position().phi(), sectors);
            for (auto sector : sectors) {
                MuonLayerSurface layerSurface(surfacePtr, sector, regionIndex, layerIndex);
                MuonSystemExtension::Intersection intersection(parsPtr, layerSurface);

                std::vector<const MdtPrepDataCollection*> mdtCols;
                if (!getLayerData(sector, regionIndex, layerIndex, mdtPrdCont, mdtCols)) {
                    ATH_MSG_DEBUG("Failed to get MDT PRD collections ");
                    continue;
                }
                std::vector<const Muon::MdtDriftCircleOnTrack*> MDTs;
                for (const auto *mdtCol : mdtCols) {
                    if (!m_muonPRDSelectionTool->calibrateAndSelectMdt(intersection, *mdtCol, MDTs)) {
                        ATH_MSG_DEBUG("Failed to calibrate MDT PRD collection ");
                        continue;
                    }
                }
                ATH_MSG_DEBUG("Running mdt segment finding: MDTs " << MDTs.size() << " MCOTs " << MCOTs.size());
                m_segmentMaker->find(theEvent->resolvedTrackSeeds()[i].first, theEvent->resolvedTrackSeeds()[i].second, MDTs, MCOTs, false,
                                     segColl);
                ATH_MSG_DEBUG("the size of the segment collection is " << segColl->size());
                for (unsigned int j = 0; j < segColl->size(); j++) {
                    Trk::Segment* tseg = segColl->at(j);
                    ATH_MSG_DEBUG("the " << j << "th segment contains " << (dynamic_cast<MuonSegment*>(tseg))->numberOfContainedROTs()
                                         << " ROTs ");
                }
            }
        }
    }

    bool MuonClusterSegmentFinder::getLayerData(int sector, MuonStationIndex::DetectorRegionIndex regionIndex,
                                                MuonStationIndex::LayerIndex layerIndex, const Muon::MdtPrepDataContainer* input,
                                                std::vector<const MdtPrepDataCollection*>& output) const {
        // get technologies in the given layer
        unsigned int sectorLayerHash = MuonStationIndex::sectorLayerHash(regionIndex, layerIndex);

        // get hashes
        const MuonLayerHashProviderTool::HashVec& hashes =
            m_layerHashProvider->getHashes(sector, MuonStationIndex::TechnologyIndex::MDT, sectorLayerHash);

        // skip empty inputs
        if (hashes.empty()) return true;

        // loop over hashes
        for (MuonLayerHashProviderTool::HashVec::const_iterator it = hashes.begin(); it != hashes.end(); ++it) {
            // skip if not found
            const auto *col = input->indexFindPtr(*it);
            if (!col) {
                // ATH_MSG_WARNING("Cannot find hash " << *it << " in container at " << location);
                continue;
            }
            ATH_MSG_VERBOSE("  adding " << m_idHelperSvc->toStringChamber(col->identify()) << " size " << col->size());
            // else add
            output.push_back(col);
        }
        return true;
    }

    MuonClusterSegmentFinder::Tree& MuonClusterSegmentFinder::getTree() const
    {
      // We earlier checked that no more than one thread is being used.
      Tree* t ATLAS_THREAD_SAFE = m_tree.get();
      return *t;
    }


}  // namespace Muon
