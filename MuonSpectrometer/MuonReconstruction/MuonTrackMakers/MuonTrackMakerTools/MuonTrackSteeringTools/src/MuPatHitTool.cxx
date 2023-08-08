/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MuPatHitTool.h"

#include "MuPatPrimitives/SortMuPatHits.h"
#include "MuonCompetingRIOsOnTrack/CompetingMuonClustersOnTrack.h"
#include "MuonPrepRawData/RpcPrepData.h"
#include "MuonRIO_OnTrack/CscClusterOnTrack.h"
#include "MuonRIO_OnTrack/MdtDriftCircleOnTrack.h"
#include "MuonRIO_OnTrack/RpcClusterOnTrack.h"
#include "MuonRecHelperTools/IMuonEDMHelperSvc.h"
#include "MuonRecHelperTools/MuonEDMPrinterTool.h"
#include "MuonSegment/MuonSegment.h"
#include "MuonTrackMakerUtils/MuonTrackMakerStlTools.h"
#include "TrkGeometry/MagneticFieldProperties.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack.h"
#include "TrkTrack/Track.h"
#include "TrkTrack/TrackStateOnSurface.h"

namespace Muon {

    MuPatHitTool::MuPatHitTool(const std::string& t, const std::string& n, const IInterface* p) : AthAlgTool(t, n, p) {
        declareInterface<MuPatHitTool>(this);
    }

    MuPatHitTool::~MuPatHitTool() = default;

    StatusCode MuPatHitTool::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_mdtRotCreator.retrieve());
        ATH_CHECK(m_cscRotCreator.retrieve(DisableTool{m_cscRotCreator.empty()})); 
        ATH_CHECK(m_edmHelperSvc.retrieve());
        ATH_CHECK(m_printer.retrieve());
        ATH_CHECK(m_pullCalculator.retrieve());
        ATH_CHECK(m_propagator.retrieve());

        return StatusCode::SUCCESS;
    }

    bool MuPatHitTool::create(const EventContext& ctx, const MuonSegment& seg, MuPatHitList& hitList) const {
        ATH_MSG_DEBUG(" creating hit list from segment " << std::endl << m_printer->print(seg));

        // create parameters with very large momentum and no charge
        double momentum{1.e8}, charge{0.};
        std::unique_ptr<const Trk::TrackParameters> pars{m_edmHelperSvc->createTrackParameters(seg, momentum, charge)};
        if (!pars) {
            ATH_MSG_WARNING(" could not create track parameters for segment ");
            return false;
        }

        return create(ctx, *pars, seg.containedMeasurements(), hitList);
    }

    bool MuPatHitTool::create(const EventContext& ctx, const Trk::TrackParameters& pars,
                              const std::vector<const Trk::MeasurementBase*>& measVec, MuPatHitList& hitList) const {
        // loop over hits
        for (const Trk::MeasurementBase* meas : measVec) {
            // create hit info
            MuPatHit::Info hitInfo = getHitInfo(*meas);

            const Identifier& id = hitInfo.id;
            if (hitInfo.type == MuPatHit::UnknownType) {
                ATH_MSG_WARNING(" unknown hit type " << m_idHelperSvc->toString(id));
                continue;
            }

            // create broad measurement
            std::unique_ptr<const Trk::MeasurementBase> broadMeas = createBroadMeasurement(*meas, hitInfo);
            if (!broadMeas) {
                ATH_MSG_WARNING(" could not create broad measurement " << m_idHelperSvc->toString(id));
                continue;
            }
            // extrapolate
            std::unique_ptr<const Trk::TrackParameters> exPars;
            if (pars.associatedSurface() == broadMeas->associatedSurface()) {
                exPars = pars.uniqueClone();
                ATH_MSG_VERBOSE(" start parameters and measurement expressed at same surface, cloning parameters ");
            } else {                
                exPars = m_propagator->propagate(ctx, pars, broadMeas->associatedSurface(), Trk::anyDirection, false, m_magFieldProperties);
                if (!exPars) { continue; }  // !exPars
            }

            // create hit and insert it into list
            ATH_MSG_VERBOSE(" inserting hit " << m_idHelperSvc->toString(id) << " " << m_printer->print(*exPars));
            std::unique_ptr<MuPatHit> hit = std::make_unique<MuPatHit>(std::move(exPars), meas->uniqueClone(), std::move(broadMeas), std::move(hitInfo));
            hitList.push_back(std::move(hit));           
        }

        const SortByDirectionMuPatHits isLargerCal{pars};
        std::stable_sort(hitList.begin(), hitList.end(), isLargerCal);      
        return true;
    }

    bool MuPatHitTool::create(const Trk::Track& track, MuPatHitList& hitList) const {
        // loop over hits
        for (const Trk::TrackStateOnSurface* tsit : *track.trackStateOnSurfaces()) {
            // do not take into account scatteres and holes for now
            if (tsit->type(Trk::TrackStateOnSurface::Scatterer) ||
                tsit->type(Trk::TrackStateOnSurface::Hole)) continue;

            const Trk::MeasurementBase* meas = tsit->measurementOnTrack();
            if (!meas) continue;

            const Trk::TrackParameters* pars = tsit->trackParameters();
            if (!pars) continue;

            // create hit info
            MuPatHit::Info hitInfo = getHitInfo(*meas);

            if (tsit->type(Trk::TrackStateOnSurface::Outlier)) hitInfo.status = MuPatHit::Outlier;

            const Identifier& id = hitInfo.id;

            if (!m_idHelperSvc->isMuon(id)) continue;

            if (hitInfo.type == MuPatHit::UnknownType) {
                ATH_MSG_WARNING(" unknown hit type " << m_idHelperSvc->toString(id));
                continue;
            }

            // create broad measurement
            std::unique_ptr<const Trk::MeasurementBase> broadMeas = createBroadMeasurement(*meas, hitInfo);
            if (!broadMeas) {
                ATH_MSG_WARNING(" could not create broad measurement " << m_idHelperSvc->toString(id));
                continue;
            }

            // create hit and insert it into list
            std::unique_ptr<MuPatHit> hit = std::make_unique<MuPatHit>(pars->uniqueClone(), meas->uniqueClone(), std::move(broadMeas), hitInfo);
            ATH_MSG_VERBOSE(" inserting hit " << m_printer->print(*meas) << (hitInfo.status == MuPatHit::Outlier ? " Outlier" : ""));
            double residual{0.}, pull{0.};
            calculateResiduals(tsit, Trk::ResidualPull::Unbiased, residual, pull);
            hit->setResidual(residual,pull);
            hitList.push_back(std::move(hit));         
        }

        const Trk::TrackParameters* pars = track.perigeeParameters();
        const SortByDirectionMuPatHits isLargerCal{*pars};
        std::stable_sort(hitList.begin(), hitList.end(), isLargerCal);      

        return true;
    }

    MuPatHitList MuPatHitTool::merge(const MuPatHitList& hitList1, const MuPatHitList& hitList2) const {
        // copy first list into outList
        MuPatHitList tmpList{};
        tmpList.reserve(hitList1.size() + hitList2.size());

        if (!hitList1.empty()) {
            const Trk::TrackParameters& pars{hitList1.front()->parameters()};
            const SortByDirectionMuPatHits isLargerCal{pars};            
            std::merge(hitList1.begin(), hitList1.end(), hitList2.begin(), hitList2.end(), 
                            std::back_inserter(tmpList), isLargerCal);
        } else {
            return hitList2;
        }
        MuPatHitList outList{};
        outList.reserve(tmpList.size());
        std::set<Identifier> used_hits{};
        /// Loop another time to ensure that duplicate hits are removed
        std::copy_if(std::make_move_iterator(tmpList.begin()), std::make_move_iterator(tmpList.end()), std::back_inserter(outList),
                     [&used_hits](const MuPatHitPtr& pathit) { return used_hits.insert(pathit->info().id).second; });
        
        return outList;
    }

    bool MuPatHitTool::extract(const MuPatHitList& hitList, std::vector<const Trk::MeasurementBase*>& measVec, bool usePreciseHits,
                               bool /*getReducedTrack*/) const {
        // make sure the vector is sufficiently large
        measVec.reserve(hitList.size());

        // loop over hit list
        for (const MuPatHitPtr& hit : hitList) {
            if (hit->info().status != MuPatHit::OnTrack) { continue; }
            const Trk::MeasurementBase* meas = usePreciseHits ? &hit->preciseMeasurement() : &hit->broadMeasurement();
            measVec.push_back(meas);
        }
        return true;
    }

    bool MuPatHitTool::remove(const Identifier& id, MuPatHitList& hitList) const {
        // loop over hit list
        MuPatHitIt lit = hitList.begin(), lit_end = hitList.end();
        for (; lit != lit_end; ++lit) {
            const MuPatHit& hit = **lit;
            if (hit.info().id == id) {
                hitList.erase(lit);
                return true;
            }
        }
        // if we get here the hit was not found
        return false;
    }

    bool MuPatHitTool::remove(const Trk::MeasurementBase& meas, MuPatHitList& hitList) const {
        // loop over hit list
        const Identifier meas_id = m_edmHelperSvc->getIdentifier(meas);
        MuPatHitIt lit = hitList.begin(), lit_end = hitList.end();
        for (; lit != lit_end; ++lit) {
            const MuPatHit& hit = **lit;
            if (m_edmHelperSvc->getIdentifier(hit.preciseMeasurement()) == meas_id || 
                m_edmHelperSvc->getIdentifier(hit.broadMeasurement()) == meas_id) {
                hitList.erase(lit);
                return true;
            }
        }
        // if we get here the hit was not found
        return false;
    }

    MuPatHit::Type MuPatHitTool::getHitType(const Identifier& id) const {
        if (m_idHelperSvc->isMdt(id))
            return MuPatHit::MDT;
        else if (m_idHelperSvc->isTgc(id))
            return MuPatHit::TGC;
        else if (m_idHelperSvc->isCsc(id))
            return MuPatHit::CSC;
        else if (m_idHelperSvc->isRpc(id))
            return MuPatHit::RPC;
        else if (m_idHelperSvc->isMM(id))
            return MuPatHit::MM;
        else if (m_idHelperSvc->issTgc(id))
            return MuPatHit::sTGC;
        else if (m_idHelperSvc->isMuon(id))
            return MuPatHit::PREC;
        return MuPatHit::UnknownType;
    }

    MuPatHit::Info MuPatHitTool::getHitInfo(const Trk::MeasurementBase& meas) const {
        MuPatHit::Info hitInfo{};
        hitInfo.id = m_edmHelperSvc->getIdentifier(meas);
        // for clusters store layer id instead of channel id
        hitInfo.measuresPhi = true;  // assume that all PseudoMeasurements measure phi!!
        hitInfo.type = MuPatHit::Pseudo;
        hitInfo.status = MuPatHit::OnTrack;
        if (hitInfo.id.is_valid() && m_idHelperSvc->isMuon(hitInfo.id)) {
            hitInfo.type = getHitType(hitInfo.id);
            hitInfo.measuresPhi = m_idHelperSvc->measuresPhi(hitInfo.id);
            hitInfo.isSmall = m_idHelperSvc->isSmallChamber(hitInfo.id);
            hitInfo.stIdx = m_idHelperSvc->stationIndex(hitInfo.id);
            hitInfo.isEndcap = m_idHelperSvc->isEndcap(hitInfo.id);
            if (hitInfo.type != MuPatHit::MDT && hitInfo.type != MuPatHit::MM) hitInfo.id = m_idHelperSvc->layerId(hitInfo.id);
        }
        return hitInfo;
    }

    std::unique_ptr<const Trk::MeasurementBase> MuPatHitTool::createBroadMeasurement(const Trk::MeasurementBase& meas,
                                                                                     const MuPatHit::Info& hitInfo) const {
        // don't change errors for Pseudo measurements
        if (hitInfo.type == MuPatHit::MDT) {
            const MdtDriftCircleOnTrack* mdt = dynamic_cast<const MdtDriftCircleOnTrack*>(&meas);
            if (!mdt) {
                ATH_MSG_WARNING(" found hit with a MDT Identifier that is not a MdtDriftCircleOnTrack "
                                << m_idHelperSvc->toString(hitInfo.id));
                return nullptr;
            }
            ATH_MSG_DEBUG(" creating broad MdtDriftCircleOnTrack ");

            return std::unique_ptr<const Trk::MeasurementBase>(m_mdtRotCreator->updateError(*mdt));

        } else if (hitInfo.type == MuPatHit::CSC && !hitInfo.measuresPhi) {
            if (m_cscRotCreator.empty()) {
                // Configured to not use CSC's
                return nullptr;
            }
            const CscClusterOnTrack* csc = dynamic_cast<const CscClusterOnTrack*>(&meas);
            if (!csc) {
                ATH_MSG_WARNING(" found hit with CSC identifier that is not a CscClusterOnTrack " << m_idHelperSvc->toString(hitInfo.id));
                return nullptr;
            }
            ATH_MSG_DEBUG(" creating broad CscClusterOnTrack ");

            return std::unique_ptr<const Trk::MeasurementBase>(
                m_cscRotCreator->createRIO_OnTrack(*csc->prepRawData(), csc->globalPosition()));
        }

        // don't change errors for CSC phi hits, TGC, RPC and Pseudo measurements
        return meas.uniqueClone();
    }

    bool MuPatHitTool::update(const Trk::Track& track, MuPatHitList& hitList) const {
        const DataVector<const Trk::MeasurementBase>* measurements = track.measurementsOnTrack();
        if (!measurements) return false;

        std::set<Identifier> ids;

        DataVector<const Trk::MeasurementBase>::const_iterator mit = measurements->begin();
        DataVector<const Trk::MeasurementBase>::const_iterator mit_end = measurements->end();
        for (; mit != mit_end; ++mit) {
            Identifier id = m_edmHelperSvc->getIdentifier(**mit);
            if (!id.is_valid()) continue;

            if (!m_idHelperSvc->isMdt(id)) id = m_idHelperSvc->layerId(id);

            ids.insert(id);
        }

        // loop over hit list
        MuPatHitIt lit = hitList.begin(), lit_end = hitList.end();
        for (; lit != lit_end; ++lit) {
            MuPatHit& hit = **lit;
            if (!ids.count(hit.info().id)) {
                hit.info().status = MuPatHit::Outlier;
                continue;
            }
        }
        return true;
    }

    std::string MuPatHitTool::print(const MuPatHitList& hitList, bool printPos, bool printDir, bool printMom) const {
        std::ostringstream sout;
        DistanceAlongParameters distCal{};

        // for nicely aligned printout, get max width of Id printout
        std::vector<std::string> idStrings;
        std::vector<std::string> dataStrings;
        idStrings.reserve(hitList.size());
        unsigned int idWidth = 0;
        std::string result = "first  ";
        bool isLarger = true;
        double distance = 0;
        MuPatHitCit it = hitList.begin();
        MuPatHitCit it_end = hitList.end();
        MuPatHitCit itNext = hitList.begin();

        const Trk::TrackParameters& pars{hitList.front()->parameters()};
        const SortByDirectionMuPatHits isLargerCal{pars};

        if (itNext != it_end) ++itNext;
        for (; it != it_end; ++it, ++itNext) {

            Identifier id = m_edmHelperSvc->getIdentifier((*it)->measurement());
            std::string idStr = id.is_valid() ? m_idHelperSvc->toString(id) : "pseudo-measurement";
            idStrings.push_back(idStr);
            if (idStr.length() > idWidth) idWidth = idStr.length();
            const Trk::TrackParameters& pars = (*it)->parameters();
            std::ostringstream dataOss;
            if (printPos) {
                dataOss << "r " << std::fixed << std::setprecision(0) << std::setw(5) << pars.position().perp() << " z " << std::fixed
                        << std::setprecision(0) << std::setw(6) << pars.position().z();
            }
            if (printDir) {
                dataOss << " theta " << std::fixed << std::setprecision(5) << std::setw(7) << pars.momentum().theta() << " phi "
                        << std::fixed << std::setprecision(3) << std::setw(6) << pars.momentum().phi();
            }
            if (printMom) {
                dataOss << " q*p(GeV) " << std::scientific << std::setprecision(3) << std::setw(10)
                        << pars.momentum().mag() * pars.charge() / 1000.;
            }


            dataOss << "  " << result << " dist " << distance;
            dataStrings.push_back(dataOss.str());
            if (itNext != it_end) {
                isLarger = isLargerCal(*it, *itNext);

                distance = distCal(*it, *itNext);
                result = isLarger ? "larger " : "smaller";

                if (isLarger == isLargerCal(*itNext, *it)) {
                    result = "duplicate";
                } else if (!isLarger) {
                    result += "   sorting problem ";
                }
            }
        }

        // second loop to print out aligned strings
        unsigned int n = idStrings.size();
        for (unsigned int i = 0; i < n; ++i) {
            sout << "  " << std::left << std::setw(idWidth) << idStrings[i] << std::right << " " << dataStrings[i];
            if (i != n - 1) sout << std::endl;
        }

        return sout.str();
    }
    bool MuPatHitTool::isSorted(const MuPatHitList& hitList) const {
        MuPatHitCit it = hitList.begin();
        MuPatHitCit it_end = hitList.end();
        MuPatHitCit itNext = it;
        if (itNext != it_end) ++itNext;
        bool isLarger = true;
        const Trk::TrackParameters& pars{hitList.front()->parameters()};
        const SortByDirectionMuPatHits isLargerCal{pars};
        for (; itNext != it_end; ++it, ++itNext) {
            isLarger = isLargerCal(*it, *itNext);
            bool sameSurface = (isLarger == isLargerCal(*it, *itNext));  // same surface
            if (!isLarger && !sameSurface) return false;
            if (sameSurface) return false;
        }
        return true;
    }
    inline void MuPatHitTool::calculateResiduals(const Trk::TrackStateOnSurface* tsos, Trk::ResidualPull::ResidualType type, double& residual,
                                            double& residualPull) const {
        std::unique_ptr<const Trk::ResidualPull> resPull(
            m_pullCalculator->residualPull(tsos->measurementOnTrack(), tsos->trackParameters(), type));
        if (resPull) {
            residual = resPull->residual().front();
            residualPull = resPull->pull().front();
        } else {
            residual = residualPull = -999;
        }
    }
}  // namespace Muon
