/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonSegmentTruthAssociationAlg.h"

#include "MuonSegment/MuonSegment.h"
#include "StoreGate/WriteDecorHandle.h"
#include "xAODMuon/MuonSegment.h"
#include "xAODMuon/MuonSegmentAuxContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "AtlasHepMC/MagicNumbers.h"

namespace Muon {

    // Constructor with parameters:
    MuonSegmentTruthAssociationAlg::MuonSegmentTruthAssociationAlg(const std::string& name, ISvcLocator* pSvcLocator) :
        AthReentrantAlgorithm(name, pSvcLocator) {}

    // Initialize method:
    StatusCode MuonSegmentTruthAssociationAlg::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_printer.retrieve());
        ATH_CHECK(m_muonTrackTruthTool.retrieve());
        m_muonTruthSegmentContainerName = m_muonTruthSegmentContainerName.key() + ".recoSegmentLink";
        m_muonSegmentCollectionName = m_muonSegmentCollectionName.key() + ".truthSegmentLink";
        ATH_CHECK(m_muonTruthSegmentContainerName.initialize());
        ATH_CHECK(m_muonSegmentCollectionName.initialize());
        ATH_CHECK(m_mcEventColl.initialize());
        if (!(m_idHelperSvc->hasSTGC() && m_idHelperSvc->hasMM())) m_muonSimData = {"MDT_SDO", "RPC_SDO", "TGC_SDO"};
        ATH_CHECK(m_muonSimData.initialize());
        ATH_CHECK(m_cscSimData.initialize(m_idHelperSvc->hasCSC()));
        ATH_CHECK(m_trackRecord.initialize());
        return StatusCode::SUCCESS;
    }

    // Execute method:
    StatusCode MuonSegmentTruthAssociationAlg::execute(const EventContext& ctx) const {
        // skip if no input data found
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, ElementLink<xAOD::MuonSegmentContainer> > muonTruthSegments(
            m_muonTruthSegmentContainerName, ctx);
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, ElementLink<xAOD::MuonSegmentContainer> > segments(m_muonSegmentCollectionName,
                                                                                                            ctx);
        if (!muonTruthSegments.isPresent()) {
            ATH_MSG_DEBUG("No muon truth segments");
            return StatusCode::SUCCESS;
        }
        if (!segments.isPresent()) {
            ATH_MSG_DEBUG("No muon segments");
            return StatusCode::SUCCESS;
        }
        if (!muonTruthSegments.isValid()) {
            ATH_MSG_ERROR("Muon truth segments not valid");
            return StatusCode::FAILURE;
        }
        if (!segments.isValid()) {
            ATH_MSG_ERROR("Muon segments not valid");
            return StatusCode::FAILURE;
        }
        SG::ReadHandle<TrackRecordCollection> truthTrackCol(m_trackRecord, ctx);
        if (!truthTrackCol.isValid()) {
            ATH_MSG_ERROR("Track collection " << m_trackRecord.key() << " is not present");
            return StatusCode::FAILURE;
        }
        if (truthTrackCol.cptr()->empty()) {
            ATH_MSG_DEBUG("Track collection " << m_trackRecord.key() << " is empty. Skip the rest of the alg");
            return StatusCode::SUCCESS;
        }

        std::string truthSegmentContainerName (m_muonTruthSegmentContainerName.key());
        auto ppos = truthSegmentContainerName.find('.');
        truthSegmentContainerName.resize(std::min(ppos,truthSegmentContainerName.size()));
        std::string segmentCollectionName( m_muonSegmentCollectionName.key());
        ppos = segmentCollectionName.find('.');
        segmentCollectionName.resize(std::min(ppos,segmentCollectionName.size()));

        std::vector<const Muon::MuonSegment*> muonSegments;
        typedef std::map<const Muon::MuonSegment*, ElementLink<xAOD::MuonSegmentContainer> > MuonSegmentLinkMap;
        MuonSegmentLinkMap muonSegmentLinkMap;
        unsigned int segIndex = 0;
        muonSegments.reserve(segments->size());
        for (const auto seg : *segments) {
            segments(*seg) = ElementLink<xAOD::MuonSegmentContainer>();
            if (seg->muonSegment().isValid()) {
                const Muon::MuonSegment* mseg = dynamic_cast<const Muon::MuonSegment*>(*seg->muonSegment());
                if (mseg) {
                    ATH_MSG_DEBUG(" Reco segment " << m_printer->print(*mseg));
                    muonSegments.push_back(mseg);
                    muonSegmentLinkMap[mseg] = ElementLink<xAOD::MuonSegmentContainer>(segmentCollectionName, segIndex);
                }
            }
            ++segIndex;
        }

        SG::ReadHandle<McEventCollection> mcEventCollection(m_mcEventColl, ctx);
        std::vector<const MuonSimDataCollection*> muonSimData;
        for (SG::ReadHandle<MuonSimDataCollection>& simDataMap : m_muonSimData.makeHandles(ctx)) {
            if (!simDataMap.isValid()) {
                ATH_MSG_WARNING(simDataMap.key() << " not valid");
                continue;
            }
            if (!simDataMap.isPresent()) continue;
            muonSimData.push_back(simDataMap.cptr());
        }
        IMuonTrackTruthTool::TruthTree truth_tree;
        if (m_idHelperSvc->hasCSC()) {
            SG::ReadHandle<CscSimDataCollection> cscSimDataMap(m_cscSimData, ctx);
            if (!cscSimDataMap.isValid()) {
                ATH_MSG_WARNING(cscSimDataMap.key() << " not valid");
                truth_tree = m_muonTrackTruthTool->createTruthTree(truthTrackCol.cptr(), mcEventCollection.cptr(), muonSimData, nullptr);
            } else {
                truth_tree = m_muonTrackTruthTool->createTruthTree(truthTrackCol.cptr(), mcEventCollection.cptr(), muonSimData,
                                                                   cscSimDataMap.cptr());
            }
        } else {
            truth_tree = m_muonTrackTruthTool->createTruthTree(truthTrackCol.cptr(), mcEventCollection.cptr(), muonSimData, nullptr);
        }
        ATH_MSG_DEBUG("Matching reconstructed segments " << muonSegments.size());
        IMuonTrackTruthTool::SegmentResultVec segmentMatchResult = m_muonTrackTruthTool->match(truth_tree, muonSegments);

        // create a map of chamber index onto the truth segments
        std::map<Muon::MuonStationIndex::ChIndex, std::vector<ElementLink<xAOD::MuonSegmentContainer> > > chamberTruthSegmentLinks;
        segIndex = 0;
        for (const auto truthSegment : *muonTruthSegments) {
            muonTruthSegments(*truthSegment) = ElementLink<xAOD::MuonSegmentContainer>();
            std::vector<ElementLink<xAOD::MuonSegmentContainer> >& linkVec = chamberTruthSegmentLinks[truthSegment->chamberIndex()];
            linkVec.push_back(ElementLink<xAOD::MuonSegmentContainer>(truthSegmentContainerName, segIndex));
            ATH_MSG_DEBUG("New truth segment: index " << segIndex << " " << Muon::MuonStationIndex::chName(truthSegment->chamberIndex())
                                                      << " nlinks " << linkVec.size() << " link " << *linkVec.back());
            ++segIndex;
        }

        // extract segment belonging to this muon
        IMuonTrackTruthTool::SegmentResultVec segmentResultVec;
        std::set<Muon::MuonStationIndex::ChIndex> chambers;
        std::vector<std::pair<Muon::MuonStationIndex::ChIndex, const TrackRecord*> > chamberTruthVec;
        for (const auto& result : segmentMatchResult) {
            // only use segment that are matched
            ATH_MSG_DEBUG("Match reco segment " << m_printer->print(*(result.first)) << " truth " << result.second.truthTrack);
            if (!result.second.truthTrack) continue;
            int barcode = result.second.truthTrack->GetBarCode();

            // get chamber Identifier
            Identifier id;
            if (!result.second.mdts.matchedHits.empty()) id = *result.second.mdts.matchedHits.begin();
            if (!result.second.cscs.matchedHits.empty()) id = *result.second.cscs.matchedHits.begin();
            if (!result.second.stgcs.matchedHits.empty()) id = *result.second.stgcs.matchedHits.begin();
            if (!result.second.mms.matchedHits.empty()) id = *result.second.mms.matchedHits.begin();
            if (!id.is_valid()) continue;

            Muon::MuonStationIndex::ChIndex chIndex = m_idHelperSvc->chamberIndex(id);
            auto pos = chambers.insert(chIndex);

            if (!pos.second) {
                bool skip = false;
                // select the first segment per chamber
                for (const auto& index : chamberTruthVec) {
                    // check that it is associated to same truthTrack and skip in that case
                    if (index.first == chIndex && index.second == result.second.truthTrack) {
                        ATH_MSG_DEBUG("Skip this reco segment ");
                        skip = true;
                        break;
                    }
                }
                if (skip) continue;
            }

            std::pair<Muon::MuonStationIndex::ChIndex, const TrackRecord*> chPair;
            chPair.first = chIndex;
            chPair.second = result.second.truthTrack;
            chamberTruthVec.push_back(chPair);

            // look-up segment element link
            auto segPos = muonSegmentLinkMap.find(result.first);
            if (segPos == muonSegmentLinkMap.end()) {
                ATH_MSG_WARNING("Could not find segment in container");
                continue;
            }
            ElementLink<xAOD::MuonSegmentContainer> recoLink = segPos->second;
            const xAOD::MuonSegment* recoSegment = *recoLink;
            if (!recoLink) {
                ATH_MSG_WARNING("Reco segment link invalid");
                continue;
            }
            ATH_MSG_DEBUG("recoLink " << recoLink << " chamber index: " << Muon::MuonStationIndex::chName(chIndex));
            // look-up truth segments in the chamber layer
            auto truthPos = chamberTruthSegmentLinks.find(chIndex);
            if (truthPos == chamberTruthSegmentLinks.end()) continue;
            ATH_MSG_DEBUG("Found matching chamber index: " << Muon::MuonStationIndex::chName(chIndex) << " links "
                                                           << truthPos->second.size());

            for (auto& truthSegLink : truthPos->second) {
                // get truth particle for the segment
                truthSegLink.toPersistent();
                const xAOD::MuonSegment* truthSegment = *truthSegLink;
                if (!truthSegment) {
                    ATH_MSG_WARNING("Invalid truth segment link " << truthSegLink);
                    continue;
                }
                ATH_MSG_DEBUG("truthSegLink " << truthSegLink);
                if (!truthSegment->isAvailable<ElementLink<xAOD::TruthParticleContainer> >("truthParticleLink")) {
                    ATH_MSG_WARNING("truthSegment without truthParticleLink ");
                    continue;
                }
                ElementLink<xAOD::TruthParticleContainer> truthLink =
                    truthSegment->auxdata<ElementLink<xAOD::TruthParticleContainer> >("truthParticleLink");
                const xAOD::TruthParticle* truthParticle = *truthLink;
                if (!truthParticle) {
                    ATH_MSG_WARNING("Invalid truth link " << truthLink);
                    continue;
                }
                // match barcodes
                if (barcode % HepMC::SIM_REGENERATION_INCREMENT == truthParticle->barcode()) {
                    ATH_MSG_DEBUG("Matched reconstructed segment: barcode " << barcode << " layer "
                                                                            << Muon::MuonStationIndex::chName(chIndex));
                    recoLink.toPersistent();
                    truthLink.toPersistent();
                    muonTruthSegments(*truthSegment) = recoLink;
                    segments(*recoSegment) = truthSegLink;
                } else {
                    ATH_MSG_DEBUG("barcode mismatch " << barcode << " truthParticle->barcode " << truthParticle->barcode());
                }
            }
        }
        return StatusCode::SUCCESS;
    }

}  // namespace Muon
