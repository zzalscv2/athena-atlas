/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtCalibT0/T0CalibrationMT.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "MdtCalibData/MdtTubeFitContainer.h"
#include "MdtCalibT0/ADCMTHistos.h"
#include "MdtCalibT0/HistogramId.h"
#include "MdtCalibT0/MdtRelativeTubeT0.h"
#include "MdtCalibT0/T0CalibrationOutput.h"
#include "MdtCalibT0/T0MTHistos.h"
#include "MdtCalibT0/T0MTSettings.h"
#include "MuonCalibEventBase/MuonCalibSegment.h"
#include "MuonCalibIdentifier/MuonFixedId.h"
#include "MuonCalibStl/DeleteObject.h"
#include "MuonCalibStl/ToString.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1.h"
#include "TH1F.h"
#include "TH2F.h"

namespace MuonCalib {

    T0CalibrationMT::T0CalibrationMT(const std::string& name, const T0MTSettings *settings, const std::vector<int> &sort_by,
                                     const std::vector<int> &adc_sort_by) :
        IMdtCalibration(name),
        m_settings(settings),
        m_converged(false),
        m_name(name),
        m_currentItnum(0),
        m_sort_by(sort_by),
        m_adc_sort_by(adc_sort_by),
        m_delete_settings(false) {
        if (!m_settings) {
            m_settings = new T0MTSettings();
            m_delete_settings = true;
        }

        std::string HistoFileName = "T0MT_" + m_name + ".root";
        m_file = std::make_unique<TFile>(HistoFileName.c_str(), "recreate");
        m_regiondir = m_file->mkdir(m_name.c_str());

        m_histos.resize(sort_by.size());
        m_adc_histos.resize(adc_sort_by.size());

        m_tube_ids.resize(sort_by.size());
        m_adc_tube_ids.resize(adc_sort_by.size());
    }

    T0CalibrationMT::~T0CalibrationMT() {
        m_file->Write();
        m_file->Close();
        if (m_delete_settings) delete m_settings;
    }

    bool T0CalibrationMT::handleSegment(MuonCalibSegment &seg) {
        for (const MuonCalibSegment::MdtHitPtr& hit : seg.mdtHOT()) {
            MuonFixedId id = hit->identify();
            m_nhits_per_tube[id.getIdInt()]++;
            // get the T0 originally subtracted for this hit
            int nML = id.mdtMultilayer();
            int nL = id.mdtTubeLayer();
            int nT = id.mdtTube();
            const MdtTubeFitContainer::SingleTubeCalib *stc(nullptr);
            NtupleStationId sid(id);
            sid.SetMultilayer(0);
            std::map<NtupleStationId, MdtTubeFitContainer *>::const_iterator res_it(m_result.find(sid));
            if (res_it != m_result.end()) {
                const MdtIdHelper& idHelper{res_it->second->idHelperSvc()->mdtIdHelper()};
                const Identifier tubeId = idHelper.channelID(id.stationNameString(), 
                                                             id.eta(), id.phi(), nML, nL, nT);
                stc = res_it->second->getCalib(tubeId);
            }
            double oldT0 = 0;
            if (stc)
                oldT0 = stc->t0;
            else {
                MsgStream log(Athena::getMessageSvc(), "T0CalibrationMT");
                log << MSG::WARNING << "no Single Tube Calib info found for ML=" << nML << " L=" << nL << " T=" << nT << endmsg;
            }

            // get histos
            for (unsigned int i = 0; i < m_sort_by.size(); i++) {
                T0MTHistos *histos = getHistos(id, i);
                histos->GetTSpec()->Fill(hit->driftTime() + oldT0 + hit->tubeT0());
            }
            for (unsigned int i = 0; i < m_adc_sort_by.size(); i++) {
                ADCMTHistos *adc_histos = getADCHistos(id, i);
                adc_histos->GetADCSpec()->Fill(hit->adcCount());
            }
            // relative t0 offsets
            m_rel_tube_t0s[sid].AddHit(*hit);
        }
        return true;
    }

    bool T0CalibrationMT::analyse() {
        MsgStream log(Athena::getMessageSvc(), "T0CalibrationMT");            
        std::map<int, MdtTubeFitContainer::SingleTubeFit> full;
        std::map<int, MdtTubeFitContainer::SingleTubeCalib> st;
        std::map<int, std::string> fit_by;
        if (m_settings->FitTime())
            for (unsigned int i = 0; i < m_sort_by.size(); i++) { analyse_tdc(i, full, st, fit_by); }
        for (unsigned int i = 0; i < m_adc_sort_by.size(); i++) { analyse_adc(i, full, st); }

        for (auto & it : full) {
            if (it.first == 0) continue;
            MuonFixedId fId(it.first);
            NtupleStationId sid(fId);
            sid.SetMultilayer(0);
            MdtTubeFitContainer::SingleTubeFit &fi(it.second);
            MdtTubeFitContainer::SingleTubeCalib &stc(st[it.first]);
            fi.group_by = fit_by[it.first];
            int nML = fId.mdtMultilayer();
            int nL = fId.mdtTubeLayer();
            int nT = fId.mdtTube();
            const MdtIdHelper& idHelper{m_result[sid]->idHelperSvc()->mdtIdHelper()};
            const Identifier tubeId = idHelper.channelID(fId.stationNameString(), 
                                                         fId.eta(), fId.phi(), nML, nL, nT);
                 
            bool setInfo = m_result[sid]->setCalib(std::move(stc), tubeId, log);
            if (!setInfo) {
                log << MSG::WARNING << "T0CalibrationMT::PROBLEM! could not set SingleTubeCalib info" << endmsg;
            }
            fi.n_hits = m_nhits_per_tube[fId.getIdInt()];
            fi.n_hits_above_adc_cut = m_nhits_per_tube[fId.getIdInt()];
            setInfo = m_result[sid]->setFit(std::move(fi), tubeId, log);
            if (!setInfo) {
                MsgStream log(Athena::getMessageSvc(), "T0CalibrationMT");
                log << MSG::WARNING << "T0CalibrationMT::PROBLEM! could not set SingleTubeFit info" << endmsg;
            }
        }

        m_currentItnum++;
        m_file->Write();
        return true;
    }

    bool T0CalibrationMT::analyse_tdc(const int &nr, std::map<int, MdtTubeFitContainer::SingleTubeFit> &full,
                                      std::map<int, MdtTubeFitContainer::SingleTubeCalib> &st, std::map<int, std::string> &fit_by_map) {
        if (m_settings->VerboseLevel() > 1) {
            MsgStream log(Athena::getMessageSvc(), "T0CalibrationMT");
            log << MSG::VERBOSE << "T0CalibrationMT::analyse iteration " << m_currentItnum << endmsg;
        }
        std::string fit_by("UNKNOWN");
        switch (m_sort_by[nr]) {
            case HistogramId::TUBE: fit_by = "TUBE"; break;
            case HistogramId::LAYER: fit_by = "LAYER"; break;
            case HistogramId::MULTILAYER: fit_by = "MULTILAYER"; break;
            case HistogramId::CHAMBER: fit_by = "CHAMBER"; break;
            case HistogramId::MEZZ_CARD: fit_by = "MEZZ_CARD"; break;
        }

        for (std::map<HistogramId, std::unique_ptr<T0MTHistos>>::iterator it = m_histos[nr].begin(); it != m_histos[nr].end(); ++it)
        // loop over m_histos histograms
        {
            doTimeFit(it->second.get(), m_tube_ids[nr][it->first], full, st, fit_by_map, fit_by);
        }
        return true;
    }

    bool T0CalibrationMT::analyse_adc(const int &nr, std::map<int, MdtTubeFitContainer::SingleTubeFit> &full,
                                      std::map<int, MdtTubeFitContainer::SingleTubeCalib> &st) {
        for (std::map<HistogramId, std::unique_ptr<ADCMTHistos>>::iterator it = m_adc_histos[nr].begin(); it != m_adc_histos[nr].end();
             ++it)
            if (m_settings->FitADC()) { doAdcFit(it->second.get(), m_adc_tube_ids[nr][it->first], full, st); }
        return true;
    }

    IMdtCalibration::MdtCalibOutputPtr T0CalibrationMT::analyseSegments(const MuonSegVec &segs) {
        for (const auto & seg : segs) handleSegment(*seg);
        analyse();
        return getResults();
    }

    void T0CalibrationMT::doTimeFit(T0MTHistos *T0h, const std::set<MuonFixedId> &tube_ids,
                                    std::map<int, MdtTubeFitContainer::SingleTubeFit> &fim,
                                    std::map<int, MdtTubeFitContainer::SingleTubeCalib> &stcm, std::map<int, std::string> &fit_by_map,
                                    const std::string &fit_by) {
        TDirectory *cwd = gDirectory;
        m_regiondir->cd();
        // do t0 fit
        MdtRelativeTubeT0::TubeGroup theGroup(MdtRelativeTubeT0::UNKNOWN);
        if (fit_by == "CHAMBER")
            theGroup = MdtRelativeTubeT0::CHAMBER;
        else if (fit_by == "MULTILAYER")
            theGroup = MdtRelativeTubeT0::MULTILAYER;
        else if (fit_by == "LAYER")
            theGroup = MdtRelativeTubeT0::LAYER;
        else if (fit_by == "MEZZ_CARD")
            theGroup = MdtRelativeTubeT0::MEZZ_CARD;
        if (T0h->FitT0() && m_settings->MinEntriesTime() <= T0h->GetTSpec()->GetEntries()) {
            const TF1 *fun(T0h->GetT0Function());
            for (auto tube_id : tube_ids) {
                if (tube_id.getIdInt() == 0) continue;
                NtupleStationId stId(tube_id);
                stId.SetMultilayer(0);
                double rel_t0(0.0);
                if (m_settings->T0Settings()->CorrectRelT0s()) rel_t0 = m_rel_tube_t0s[stId].GetRelativeOffset(tube_id, theGroup);
                MdtTubeFitContainer::SingleTubeFit &fi(fim[tube_id.getIdInt()]);
                MdtTubeFitContainer::SingleTubeCalib &stc(stcm[tube_id.getIdInt()]);
                // store parameters
                fi.statistics = static_cast<int>(T0h->GetTSpec()->GetEntries());
                fi.chi2Tdc = T0h->T0Chi2();
                fi.par[0] = fun->GetParameter(T0MTHistos ::T0_PAR_NR_BACK);
                fi.cov[0] = fun->GetParError(T0MTHistos ::T0_PAR_NR_BACK);
                fi.par[4] = fun->GetParameter(T0MTHistos ::T0_PAR_NR_T0) + rel_t0;
                fi.cov[4] = fun->GetParError(T0MTHistos ::T0_PAR_NR_T0);
                fi.par[6] = fun->GetParameter(T0MTHistos ::T0_PAR_NR_T);
                fi.cov[6] = fun->GetParError(T0MTHistos ::T0_PAR_NR_T);
                stc.t0 = fun->GetParameter(T0MTHistos ::T0_PAR_NR_T0) + rel_t0;
                stc.statusCode = T0h->StatusCode();
                m_converged = true;
                fit_by_map[tube_id.getIdInt()] = fit_by;
            }
        }
        if (T0h->FitTmax() && m_settings->MinEntriesTime() <= T0h->GetTSpec()->GetEntries()) {
            const TF1 *fun(T0h->GetTMaxFunction());
            // store parameters
            for (auto tube_id : tube_ids) {
                if (tube_id.getIdInt() == 0) continue;
                MdtTubeFitContainer::SingleTubeFit &fi(fim[tube_id.getIdInt()]);
                //			MdtTubeFitContainer::SingleTubeCalib &stc(stcm[*it]);
                fi.par[5] = fun->GetParameter(T0MTHistos ::TMAX_PAR_NR_TMAX);
                fi.cov[5] = fun->GetParError(T0MTHistos ::TMAX_PAR_NR_TMAX);
                fi.chi2TdcEnd = fun->GetChisquare() / fun->GetNDF();
            }
        }
        cwd->cd();
    }

    void T0CalibrationMT::doAdcFit(ADCMTHistos *T0h, const std::set<MuonFixedId> &tube_ids,
                                   std::map<int, MdtTubeFitContainer::SingleTubeFit> &fim,
                                   std::map<int, MdtTubeFitContainer::SingleTubeCalib> &stcm) {
        if (T0h->FitAdc() && m_settings->MinEntriesADC() <= T0h->GetADCSpec()->GetEntries()) {
            const TF1 *fun(T0h->GetAdcFunction());
            if (fun == nullptr) return;
            for (auto tube_id : tube_ids) {
                if (tube_id.getIdInt() == 0) continue;
                MdtTubeFitContainer::SingleTubeFit &fi(fim[tube_id.getIdInt()]);
                MdtTubeFitContainer::SingleTubeCalib &stc(stcm[tube_id.getIdInt()]);

                stc.adcCal = fun->GetParameter(1);
                for (int i = 0; (i < fun->GetNpar() && i < 4); i++) {
                    fi.adc_par[i] = fun->GetParameter(i);
                    fi.adc_err[i] = fun->GetParError(i);
                }
                fi.adc_chi2 = fun->GetChisquare() / fun->GetNDF();
            }
        }
    }

    IMdtCalibration::MdtCalibOutputPtr T0CalibrationMT::getResults() const { return std::make_shared<T0CalibrationOutput>(m_result); }

    ADCMTHistos *T0CalibrationMT::getADCHistos(const MuonFixedId &idtube, unsigned int nr) {
        HistogramId id;
        id.Initialize(idtube, m_adc_sort_by[nr]);
        if (!m_adc_histos[nr][id]) {
            TDirectory *cwd = gDirectory;
            m_regiondir->cd();
            m_adc_histos[nr][id] = std::make_unique<ADCMTHistos>(id.getIdInt(), m_settings, id.HistogramName().c_str());
            cwd->cd();
        }
        m_adc_tube_ids[nr][id].insert(idtube);
        return m_adc_histos[nr][id].get();
    }

    T0MTHistos *T0CalibrationMT::getHistos(const MuonFixedId &idtube, unsigned int nr) {
        HistogramId id;
        id.Initialize(idtube, m_sort_by[nr]);
        if (!m_histos[nr][id]) {
            TDirectory *cwd = gDirectory;
            m_regiondir->cd();
            m_histos[nr][id] = std::make_unique<T0MTHistos>(id.getIdInt(), m_settings, id.HistogramName().c_str());
            cwd->cd();
        }
        m_tube_ids[nr][id].insert(idtube);
        return m_histos[nr][id].get();
    }

    void T0CalibrationMT::setInput(const IMdtCalibrationOutput *calib_in) {
        // This method is called both by the event loop and by the tool.
        // Only the call from the tool is relevant for this implementation
        // and should be performed only once.

        if (m_result.size()) return;

        const T0CalibrationOutput *t0Input = dynamic_cast<const T0CalibrationOutput *>(calib_in);
        if (!calib_in || !t0Input) return;
        m_result = t0Input->GetMap();
        for (auto & it : m_result)
            it.second->setImplementation("T0CalibrationMT");
    }

    bool T0CalibrationMT::converged() const { return m_converged; }

}  // namespace MuonCalib
