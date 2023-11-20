/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////////
////  Authors: Pierre-Francois Giraud, Camilla Maiani
////  Contacts: pierre-francois.giraud@cern.ch, cmaiani@cern.ch
////////////////////////////////////////////////////////////////////

#include "AlignmentErrorTool.h"

#include <boost/functional/hash.hpp>
#include <fstream>
#include <sstream>

#include "MuonAlignErrorBase/AlignmentRotationDeviation.h"
#include "MuonAlignErrorBase/AlignmentTranslationDeviation.h"
#include "TrkCompetingRIOsOnTrack/CompetingRIOsOnTrack.h"
#include "TrkPrepRawData/PrepRawData.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkTrack/Track.h"

namespace MuonAlign {

AlignmentErrorTool::AlignmentErrorTool(const std::string& t, const std::string& n, const IInterface* p) : AthAlgTool(t, n, p) {
    declareInterface<Trk::ITrkAlignmentDeviationTool>(this);
}

StatusCode AlignmentErrorTool::initialize() {
    ATH_MSG_INFO("*****************************************");
    ATH_MSG_INFO("AlignmentErrorTool::initialize()");

    ATH_CHECK(m_idTool.retrieve());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_readKey.initialize());

    return StatusCode::SUCCESS;
}

void AlignmentErrorTool::makeAlignmentDeviations(const Trk::Track& track, std::vector<Trk::AlignmentDeviation*>& deviations) const {
    ATH_MSG_DEBUG("AlignmentErrorTool::makeAlignmentDeviations()");

    SG::ReadCondHandle<MuonAlignmentErrorData> readHandle{m_readKey};
    const MuonAlignmentErrorData* readCdo{*readHandle};
    if (!readCdo) {
        ATH_MSG_ERROR("nullptr to the read conditions object");
        return;
    }
    const auto& deviationVec = readCdo->getDeviations();
    std::vector<deviationSummary_t> devSumVec;
    devSumVec.reserve(deviationVec.size());
    deviationSummary_t aDevSumm;
    for (auto & i : deviationVec) {
        aDevSumm.translation = i.translation;
        aDevSumm.rotation = i.rotation;
        aDevSumm.stationName = i.stationName;
        aDevSumm.multilayer = i.multilayer;
        devSumVec.emplace_back(std::move(aDevSumm));
    }

    typedef Trk::TrackStates tsosc_t;
    const tsosc_t* tsosc = track.trackStateOnSurfaces();

    // LOOP ON HITS ON TRACK //
    unsigned int nPrecisionHits = 0;
    for (const auto *tsos : *tsosc) {
        if (!tsos->type(Trk::TrackStateOnSurface::Measurement)) {
            continue;
        }
        const Trk::MeasurementBase* meas = tsos->measurementOnTrack();
        const auto* rot = dynamic_cast<const Trk::RIO_OnTrack*>(meas);

        if (!rot) {
            const auto* crot = dynamic_cast<const Trk::CompetingRIOsOnTrack*>(meas);
            if (crot) {
                unsigned int index = crot->indexOfMaxAssignProb();
                rot = &(crot->rioOnTrack(index));
            }
        }
        if (!rot) continue;

        Identifier channelId = rot->identify();
        if (!m_idHelperSvc->isMuon(channelId)) {
            // the RIO_OnTrack Identifiers could also come from ID or Calo, but this tool is only interested in MS hits
            ATH_MSG_VERBOSE("Given Identifier " << channelId.get_compact() << " is not a muon Identifier, continuing");
            continue;
        }

        // Keep only the precision coordinate hits
        if (m_idHelperSvc->isRpc(channelId)
            || m_idHelperSvc->isTgc(channelId)
            || (m_idHelperSvc->isCsc(channelId) && m_idHelperSvc->cscIdHelper().measuresPhi(channelId) == 1)
            || (m_idHelperSvc->issTgc(channelId) && m_idHelperSvc->stgcIdHelper().channelType(channelId) != sTgcIdHelper::sTgcChannelTypes::Strip)) {
            continue;
        }

        // To maintain backward compatibility with the old error CLOBs, activate
        // the NSW hits only if specified in the CLOB, else disccard them
        if (!readCdo->hasNswHits()
            && (m_idHelperSvc->issTgc(channelId) || m_idHelperSvc->isMM(channelId))) {
            continue;
        }

        MuonCalib::MuonFixedLongId calibId = m_idTool->idToFixedLongId(channelId);
        if (!calibId.isValid()) continue;

        // GATHERING INFORMATION TO PUT TOGETHER THE STATION NAME //
        std::string alignStationName = hardwareName(calibId);
        int multilayer = 1;
        if (calibId.is_mdt()) {
            multilayer = calibId.mdtMultilayer();
        } else if (calibId.is_mmg()) {
            multilayer = calibId.mmgMultilayer();
        } else if (calibId.is_stg()) {
            multilayer = calibId.stgMultilayer();
        }
        std::string multilayerName = std::to_string(multilayer);

        ATH_MSG_DEBUG("Hit is in station " << alignStationName << " multilayer " << multilayerName);
        ++nPrecisionHits;

        // FOR CROSS-CHECK
        bool is_matched = false;

        // LOOP ON STATION DEVIATIONS EXTRACTED FROM INPUT FILE //
        for (auto & iDev : devSumVec) {
            // try to regexp-match the station name and the multilayer name
            if (!boost::regex_match(alignStationName, iDev.stationName)) {
                continue;
            }
            if (!boost::regex_match(multilayerName, iDev.multilayer)) {
                continue;
            }

            // ASSOCIATE EACH NUISANCE TO A LIST OF HITS
            iDev.hits.push_back(rot);

            // COMPUTE RELEVANT NUMBERS
            const Trk::PrepRawData* prd = rot->prepRawData();
            const Trk::Surface& sur = prd->detectorElement()->surface(prd->identify());

            double w2 = 1.0 / (rot->localCovariance()(Trk::loc1, Trk::loc1));
            iDev.sumW2 += w2;
            iDev.sumP += w2 * tsos->trackParameters()->position();
            iDev.sumU += w2 * tsos->trackParameters()->momentum().unit();

            // CHECK 1 //
            Amg::Vector3D zATLAS(0., 0., 1.);
            Amg::Vector3D v1 = (tsos->trackParameters()->position()).cross(zATLAS);
            v1 /= v1.mag();
            Amg::Vector3D v2 = sur.transform().rotation().col(2) / (sur.transform().rotation().col(2)).mag();
            double sign = (v1.dot(v2) > 0.) ? 1. : -1.;

            // ARTIFICIALLY ORIENTATE EVERYTHING TOWARDS THE SAME DIRECTION
            iDev.sumV += sign * w2 * sur.transform().rotation().col(2);

            // FOR CROSS-CHECK
            is_matched = true;


        }  // LOOP ON DEVIATIONS

        if (!is_matched) {
            ATH_MSG_WARNING("The hits in the station " << alignStationName << ", multilayer " << multilayerName
                            << " couldn't be matched to any deviation regexp in the list.");
        }

    }  // LOOP ON TSOS

    // Nuisance parameters covering the complete track are not wanted. (MS/ID
    // error treated differently for now). Removing the deviations covering the
    // full track in further processing.
    for (auto& dev : devSumVec) {
        if (dev.hits.size() == nPrecisionHits) {
            dev.hits.clear();
        }
    }

    // GET RID OF PERFECT OVERLAPS BY COMBINING ERRORS
    std::vector<const Trk::RIO_OnTrack*> v1, v2;
    for (auto iti = devSumVec.begin(); iti != devSumVec.end(); ++iti) {

        v1 = iti->hits;
        if (v1.empty()) {
            continue;
        }

        std::stable_sort(v1.begin(), v1.end());

        for (auto itj = iti+1; itj != devSumVec.end(); ++itj) {

            if (iti->hits.size() != itj->hits.size()) {
                continue;
            }

            v2 = itj->hits;
            std::stable_sort(v2.begin(), v2.end());

            if (v1 == v2) {
                auto iDev = std::distance(devSumVec.begin(), iti);
                auto jDev = std::distance(devSumVec.begin(), itj);
                ATH_MSG_DEBUG("Found deviations " << iDev << " and " << jDev << " related to the same list of hits. Merging...");
                ATH_MSG_DEBUG("old (translation, rotation) systematic uncertainties for "
                              << iDev << ": " << iti->translation << ", " << iti->rotation);
                ATH_MSG_DEBUG("old (translation, rotation) systematic uncertainties for "
                              << jDev << ": " << itj->translation << ", " << itj->rotation);

                // MERGE THE TWO DEVIATIONS ASSOCIATED TO THE SAME LIST OF HITS //
                double new_translation = std::hypot(iti->translation, itj->translation);
                double new_rotation = std::hypot(iti->rotation, itj->rotation);

                // NOW PREPARE TO ERASE ONE OF THE TWO COPIES //
                itj->hits.clear();

                // ASSIGN NEW TRASLATION/ROTATION TO THE REMAINING COPY //
                iti->translation = new_translation;
                iti->rotation = new_rotation;
                ATH_MSG_DEBUG("New combined (translation, rotation) systematic uncertainties: " << new_translation << ", " << new_rotation);

            }  // FIND AN OVERLAP IN THE HITS LISTS
        }      // SECOND LOOP ON DEVIATIONS
    }          // FIRST LOOP ON DEVIATIONS


    // NOW BUILD THE DEVIATIONS
    deviations.clear();
    ATH_MSG_DEBUG("************************************");
    ATH_MSG_DEBUG("FINAL LIST OF DEVIATIONS");
    for (const auto & iDev : devSumVec) {
        if (iDev.hits.empty()) {
            continue;
        }

        double rotation = iDev.rotation;
        double translation = iDev.translation;

        Amg::Vector3D sumP = iDev.sumP;
        Amg::Vector3D sumU = iDev.sumU;
        Amg::Vector3D sumV = iDev.sumV;
        double sumW2 = iDev.sumW2;

        sumP *= (1. / sumW2);
        sumU *= (1. / sumW2);
        sumV *= (1. / sumW2);

        if (translation >= 0.001 * Gaudi::Units::mm) {
            std::size_t hitshash = 0;
            for (const auto *it : iDev.hits) {
                boost::hash_combine(hitshash, (it->identify()).get_compact());
            }
            deviations.push_back(
                new AlignmentTranslationDeviation(sumU.cross(sumV), translation * Gaudi::Units::mm, iDev.hits));
            deviations.back()->setHashOfHits(hitshash);

            ATH_MSG_DEBUG("A translation along ("
                          << sumU.x() << ", " << sumU.y() << ", " << sumU.z() << ") with sigma=" << translation * Gaudi::Units::mm
                          << " mm was applied to " << iDev.hits.size()
                          << " hits matching the station: " << iDev.stationName.str() << " and the multilayer "
                          << iDev.multilayer.str());
        }
        if (rotation >= 0.000001 * Gaudi::Units::rad) {
            std::size_t hitshash = 0;
            for (const auto *it : iDev.hits) {
                boost::hash_combine(hitshash, (it->identify()).get_compact());
            }
            deviations.push_back(new AlignmentRotationDeviation(sumP, sumV, rotation * Gaudi::Units::rad, iDev.hits));
            deviations.back()->setHashOfHits(hitshash);

            ATH_MSG_DEBUG("A rotation around the center = (" << sumP.x() << ", " << sumP.y() << ", " << sumP.z() << ") and axis = ("
                                                             << sumV.x() << ", " << sumV.y() << ", " << sumV.z()
                                                             << ") with sigma=" << rotation / Gaudi::Units::mrad << " mrad was applied to "
                                                             << iDev.hits.size() << " hits matching the station "
                                                             << iDev.stationName.str() << " and the multilayer "
                                                             << iDev.multilayer.str());
        }

    }  // LOOP ON NUISANCES

    ATH_MSG_DEBUG("******************************");
    ATH_MSG_DEBUG("FINAL CHECKUP");
    ATH_MSG_DEBUG("Found " << deviations.size() << " nuisances after duplicates merging");
    ATH_MSG_DEBUG("******************************");

    return;
}

////////////////////////////
// RECOGNIZE STATION NAME //
////////////////////////////

inline std::string AlignmentErrorTool::hardwareName(MuonCalib::MuonFixedLongId calibId) const {
    using StationName = MuonCalib::MuonFixedLongId::StationName;

    // The only exception that cannot be caught by hardwareEta() above
    if (sector(calibId)==13) {
        if (calibId.eta()== 7 && calibId.stationName()==StationName::BOL) return "BOE1A13"; // BOE1A13 not BOL7A13
        if (calibId.eta()==-7 && calibId.stationName()==StationName::BOL) return "BOE1C13"; // BOE1C13 not BOL7C13
        if (calibId.eta()== 8 && calibId.stationName()==StationName::BOL) return "BOE2A13"; // BOE2A13 not BOL8A13
        if (calibId.eta()==-8 && calibId.stationName()==StationName::BOL) return "BOE2C13"; // BOE2C13 not BOL8C13
    }

    std::string ret { calibId.stationNameString() };
    ret.push_back(static_cast<char>('0'+std::abs(hardwareEta(calibId))));
    ret.append(side(calibId)).append(sectorString(calibId));

    return ret;
}

inline std::string_view AlignmentErrorTool::side(MuonCalib::MuonFixedLongId calibId) const {
    return calibId.eta()>0 ? "A" : calibId.eta()<0 ? "C" : "B";
}

inline std::string AlignmentErrorTool::sectorString(MuonCalib::MuonFixedLongId calibId) const {
    int sec = sector(calibId);
    if (sec<0 || sec > 99) {
        throw std::runtime_error("Unhandled sector number");
    }
    std::string ret = "00";
    ret[0] += (sec/10);
    ret[1] += (sec%10);
    return ret;
}

inline int AlignmentErrorTool::sector(MuonCalib::MuonFixedLongId calibId) const {
    if (calibId.is_tgc()) {
        // TGC sector convention is special
        return calibId.phi();
    } else {
        return isSmallSector(calibId) ? 2*calibId.phi() : 2*calibId.phi()-1;
    }
}

inline bool AlignmentErrorTool::isSmallSector(MuonCalib::MuonFixedLongId calibId) const {
    using StationName = MuonCalib::MuonFixedLongId::StationName;
    switch (calibId.stationName()) {
        case StationName::BIS:
        case StationName::BMS:
        case StationName::BOS:
        case StationName::BEE:
        case StationName::BMF:
        case StationName::BOF:
        case StationName::BOG:
        case StationName::EES:
        case StationName::EMS:
        case StationName::EOS:
        case StationName::EIS:
        case StationName::CSS:
        case StationName::BMG:
        case StationName::MMS:
        case StationName::STS:
            return true;
        default:
            return false;
    }
}

inline int AlignmentErrorTool::hardwareEta(MuonCalib::MuonFixedLongId calibId) const {
    using StationName = MuonCalib::MuonFixedLongId::StationName;
    switch (calibId.stationName()) {
        case StationName::BML:
            {
                if (sector(calibId)==13) {
                    switch (calibId.eta()) {
                        case 4: return 5;
                        case 5: return 6;
                        case 6: return 7;
                        case -4: return -5;
                        case -5: return -6;
                        case -6: return -7;
                    }
                }
                return calibId.eta();
            }
        case StationName::BOL:
            {
                if (sector(calibId)==13) {
                    if (calibId.eta()== 7) return 1; // BOE1A13 not BOL7A13
                    if (calibId.eta()==-7) return -1; // BOE1C13 not BOL7C13
                }
                return calibId.eta();
            }
        case StationName::BOF:
            return calibId.eta()>0 ? calibId.eta()*2-1 : calibId.eta()*2+1;
        case StationName::BOG:
            return calibId.eta()*2;
        case StationName::EIL:
            {
                if ((sector(calibId) == 1) || (sector(calibId) == 9)) {
                    switch (calibId.eta()) {
                        case 4: return 5;
                        case 5: return 4;
                        case -4: return -5;
                        case -5: return -4;
                    }
                }
                return calibId.eta();
            }
        case StationName::EEL:
            {
                if ((sector(calibId) == 5) && (calibId.eta() == 1)) return 2;
                if ((sector(calibId) == 5) && (calibId.eta() == -1)) return -2;
                return calibId.eta();
            }
        default: return calibId.eta();
    }
}

}  // namespace MuonAlign
