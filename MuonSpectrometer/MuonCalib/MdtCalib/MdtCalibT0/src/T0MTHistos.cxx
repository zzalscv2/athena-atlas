/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtCalibT0/T0MTHistos.h"

#include <cmath>

#include "AthenaKernel/getMessageSvc.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/MsgStream.h"
#include "MdtCalibT0/MTT0PatternRecognition.h"
#include "MdtCalibT0/MTTmaxPatternRecognition.h"
#include "TDirectory.h"
#include "TLine.h"
#include "TRandom3.h"
#include "list"
#include "boost/thread/tss.hpp"

namespace MuonCalib {

    TRandom3* getTLSRandomGen()
    {
        static boost::thread_specific_ptr<TRandom3> rnd ATLAS_THREAD_SAFE;
        TRandom3* random = rnd.get();
        if (!random) {
            random = new TRandom3();
            rnd.reset(random);
        }
        return random;
    }

    /** The fermi function to be fitted at the rising edge of the spectrum */
    inline Double_t mt_t0_fermi(Double_t *x, Double_t *par) {
        // more convenient parameters
        const Double_t &t(x[0]);
        const Double_t &t_0(par[T0MTHistos::T0_PAR_NR_T0]), &T(par[T0MTHistos::T0_PAR_NR_T]), &back(par[T0MTHistos::T0_PAR_NR_BACK]),
            &A(par[T0MTHistos::T0_PAR_NR_A]);
        // the formula
        return (back + A / (1 + std::exp(-(t - t_0) / T)));
    }

    /** The fermi function to be fitted at the trailing slope of the spectrum */
    inline Double_t mt_tmax_fermi(Double_t *x, Double_t *par) {
        // more convenient parameters
        Double_t &t(x[0]);
        Double_t &t_max(par[T0MTHistos::TMAX_PAR_NR_TMAX]), &T(par[T0MTHistos::TMAX_PAR_NR_T]), &back(par[T0MTHistos::TMAX_PAR_NR_BACK]),
            &a(par[T0MTHistos::TMAX_PAR_NR_A]), &b(par[T0MTHistos::TMAX_PAR_NR_B]), &t_0(par[T0MTHistos::TMAX_PAR_NR_T0]);
        // the formula
        return (back + (std::exp(a + b * (t - t_0))) / (1 + std::exp((t - t_max) / T)));
    }

    //////////////////
    // Fill..	//
    //////////////////
    void T0MTHistos::FillT(double t) { m_time->Fill(static_cast<Axis_t>(t)); }

    //////////////////////////////////////////////////////////
    // Initialize(int id, const T0MTSettings & settings)	//
    //////////////////////////////////////////////////////////
    void T0MTHistos::Initialize(int id, const T0MTSettings *settings, const char *hname) {
        m_settings = settings;
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "T0MTHistos");
        if (log.level() <= MSG::VERBOSE) log << MSG::VERBOSE << "T0MTHistos::Initialize: called" << endmsg;
#endif
        char buf[100];
        if (hname == nullptr)
            snprintf(buf, 100, "t_spec_%d", id);
        else
            snprintf(buf, 100, "t_spec_%s", hname);
#ifndef NDEBUG
        if (log.level() <= MSG::VERBOSE) log << MSG::VERBOSE << "directory=" << gDirectory->GetName() << endmsg;
#endif
        m_time = std::make_unique<TH1F>(buf, "", settings->NBinsTime(), settings->TimeMin(), settings->TimeMax());
        m_id = id;
        if (settings->DrawDebugGraphs()) {
#ifndef NDEBUG
            if (log.level() <= MSG::VERBOSE) log << MSG::VERBOSE << "T0MTHistos::Initialize: debug directory created" << endmsg;
#endif
            TDirectory *cwd = gDirectory;
            snprintf(buf, 100, "t0_tmax_dir_%d", id);
            m_dir = gDirectory->mkdir(buf, buf);
            cwd->cd();
        } else {
            m_dir = nullptr;
        }
        m_t0_ok = false;
        m_tmax_ok = false;
    }  // end T0MTHistos::Initialize

    //////////////////////////////////
    // SetTSpec(int id, TH1F *spec)	//
    //////////////////////////////////
    void T0MTHistos::SetTSpec(int id, TH1F *spec, const T0MTSettings *settings, bool copy_spec) {
        m_settings = settings;
        TDirectory *cwd = gDirectory;
        if (copy_spec)
            m_time = std::make_unique<TH1F>(*spec);
        else
            m_time.reset(spec);
        m_id = id;
        if (settings->DrawDebugGraphs()) {
            char buffer[100];
            snprintf(buffer, 100, "t0_tmax_dir_%d", id);
            m_dir = gDirectory->mkdir(buffer, buffer);
            cwd->cd();
        }
    }  // end T0MTHistos::SetTSpec

    //////////////////
    // FitT0()	//
    //////////////////
    bool T0MTHistos::FitT0() {
        if (m_time->GetEntries() < 1000) {
            m_status_code = 2;
            return false;
        }
        if (!NormalFit()) return false;
        if (m_time->GetEntries() < 10000) {
            m_status_code = 0;
            return true;
        }
        if (m_settings->T0Settings()->ScrambleThreshold() > 0 && m_chi2 > m_settings->T0Settings()->ScrambleThreshold()) {
            if (!T0Scramble()) {
                m_status_code = 3;
                return false;
            }
        }
        if (m_settings->T0Settings()->SlicingThreshold() > 0 && m_chi2 > m_settings->T0Settings()->SlicingThreshold()) { TopSlicing(); }
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "T0MTHistos");
        if (log.level() <= MSG::VERBOSE) log << MSG::VERBOSE << m_time->GetName() << " " << m_chi2 << endmsg;
#endif
        m_status_code = 0;
        return true;
    }  // end T0MTHistos::FitT0

    //////////////////
    // FitTmax()	//
    //////////////////
    bool T0MTHistos::FitTmax() {
        TDirectory *cwd = gDirectory;
        if (m_dir != nullptr) m_dir->cd();
        if (!m_time) {
            MsgStream log(Athena::getMessageSvc(), "T0MTHistos");
            log << MSG::WARNING << "T0MTHistos::FitTmax: Class is not initialized!" << endmsg;
            m_tmax_ok = false;
            cwd->cd();
            return false;
        }
        // check if t0-fit was successfull t0 is needed for tmax-pattern recognition
        if (!m_t0_ok || m_t0_fermi == nullptr) {
            MsgStream log(Athena::getMessageSvc(), "T0MTHistos");
            log << MSG::WARNING << "T0MTHistos::FitTmax for tube " << m_id << ": No valid t0-value!" << endmsg;
            m_tmax_ok = false;
            cwd->cd();
            return false;
        }
        // Create pattern Recognition Class
        MTTmaxPatternRecognition rec;
        // perform pattern recognition
        if (!rec.Initialize(m_time.get(), m_t0_fermi->GetParameter(T0_PAR_NR_T0), m_settings)) {
            MsgStream log(Athena::getMessageSvc(), "T0MTHistos");
            log << MSG::WARNING << "T0MTHistos::FitTmax for tube " << m_id << ": Pattern recognition failed!" << endmsg;
            m_tmax_ok = false;
            cwd->cd();
            return false;
        }
        // create function object
        char buffer[100];
        snprintf(buffer, 100, "mt_tmax_fermi");
        if (!m_tmax_fermi) {
            m_tmax_fermi = std::make_unique<TF1>(buffer, mt_tmax_fermi, rec.GetFitRangeMin(), rec.GetFitRangeMax(), N_TMAX_FIT_PAR);
            // set parameter names
            m_tmax_fermi->SetParName(TMAX_PAR_NR_TMAX, "t_{max}");
            m_tmax_fermi->SetParName(TMAX_PAR_NR_T, "T");
            m_tmax_fermi->SetParName(TMAX_PAR_NR_BACK, "r_{b}");
            m_tmax_fermi->SetParName(TMAX_PAR_NR_A, "a");
            m_tmax_fermi->SetParName(TMAX_PAR_NR_B, "b");
            // set fixed values
            m_tmax_fermi->FixParameter(TMAX_PAR_NR_BACK, rec.GetBackground());
            m_tmax_fermi->FixParameter(TMAX_PAR_NR_A, rec.GetA());
            m_tmax_fermi->FixParameter(TMAX_PAR_NR_B, rec.GetB());
            m_tmax_fermi->FixParameter(TMAX_PAR_NR_T0, m_t0_fermi->GetParameter(T0_PAR_NR_T0));
            // set start values
            m_tmax_fermi->SetParameter(TMAX_PAR_NR_TMAX, rec.GetEstimatedTMax());
            m_tmax_fermi->SetParameter(TMAX_PAR_NR_T, 3.0);
        }
        // perform fit
        if (m_dir != nullptr) {
            m_tmax_fermi->SetLineColor(3);
            m_tmax_fermi->Write();
        }
        m_tmax_fermi->SetLineColor(4);
        std::string fitopt("LR");
        if (m_settings->VerboseLevel() == 0) fitopt += "Q";
        if (m_settings->AddFitfun()) {
            fitopt += "+";
        } else {
            fitopt += "0";
        }
        m_time->Fit(m_tmax_fermi.get(), fitopt.c_str());
        m_tmax_ok = true;
        cwd->cd();
        return true;
    }  // end T0MTHistos::FitTmax

    //////////////////
    // FitT0()	//
    //////////////////
    bool T0MTHistos::NormalFit() {
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "T0MTHistos");
        if (log.level() <= MSG::VERBOSE) log << MSG::VERBOSE << "T0MTHistos::FitT0(): called" << endmsg;
#endif
        TDirectory *cwd = gDirectory;
        if (m_dir) m_dir->cd();
        // check if class is initialized
        if (!m_time) {
#ifdef NDEBUG
            MsgStream log(Athena::getMessageSvc(), "T0MTHistos");
#endif
            log << MSG::WARNING << "T0MTHistos::FitT0: Class is not initialized!" << endmsg;
            m_t0_ok = false;
            cwd->cd();
            m_status_code = 3;
            return false;
        }
        // create pattern recognition class
        MTT0PatternRecognition rec;
        // perform pattern recognition
        if (!rec.Initialize(m_time.get(), m_settings)) {
            m_t0_ok = false;
#ifdef NDEBUG
            MsgStream log(Athena::getMessageSvc(), "T0MTHistos");
#endif
            log << MSG::WARNING << "T0MTHistos::FitT0 for tube " << m_id << ": Pattern recognition failed!" << endmsg;
            cwd->cd();
            m_status_code = 3;
            return false;
        }
        m_t0_ok = true;
        // create function class
        char buffer[100];
        //	sprintf(buffer,"mt_t0_fermi_%d", m_id);
        snprintf(buffer, 100, "mt_t0_fermi");
        m_t0_fermi = std::make_unique<TF1>(buffer, mt_t0_fermi, rec.GetFitRangeMin(), rec.GetFitRangeMax(), N_T0_FIT_PAR);
        // set parameter names
        m_t0_fermi->SetParName(T0_PAR_NR_T0, "t_{0}");
        m_t0_fermi->SetParName(T0_PAR_NR_T, "T");
        m_t0_fermi->SetParName(T0_PAR_NR_BACK, "r_{b}");
        m_t0_fermi->SetParName(T0_PAR_NR_A, "A");
        // set fixed values
        m_t0_fermi->FixParameter(T0_PAR_NR_BACK, rec.GetBackground());
        m_t0_fermi->SetParameter(T0_PAR_NR_A, rec.GetHeight() - rec.GetBackground());
        // set estimates as start values
        m_t0_fermi->SetParameter(T0_PAR_NR_T0, rec.GetEstimatedT0());
        // set resonable start value for T
        m_t0_fermi->SetParameter(T0_PAR_NR_T, 3.0);
        // perform fit - NOTE: The return value of the Fit function is not documented!
        if (m_dir != nullptr) {
            std::unique_ptr<TLine> ln = std::make_unique<TLine>(rec.GetFitRangeMin(), 0, rec.GetFitRangeMin(), m_time->GetMaximum());
            ln->Write("t0_range_min");
            ln = std::make_unique<TLine>(rec.GetFitRangeMax(), 0, rec.GetFitRangeMax(), m_time->GetMaximum());
            ln->Write("t0_range_max");
            m_t0_fermi->SetLineColor(3);
            m_t0_fermi->Write();
        }
        m_t0_fermi->SetLineColor(2);
        std::string fitopt("BLR");
        if (m_settings->VerboseLevel() == 0) fitopt += "Q";
        if (m_settings->AddFitfun()) {
            fitopt += "+";
        } else {
            fitopt += "0";
        }
        m_time->Fit(m_t0_fermi.get(), fitopt.c_str(), "", rec.GetFitRangeMin(), rec.GetFitRangeMax());
        if (m_settings->T0Settings()->UseTopChi2())
            TopChi2();
        else
            m_chi2 = m_time->GetFunction("mt_t0_fermi")->GetChisquare() / m_time->GetFunction("mt_t0_fermi")->GetNDF();
        m_t0_ok = true;
        cwd->cd();
        m_status_code = 0;
        return true;
    }  // end T0MTHistos::NormalFit

    //////////////////
    // T0Scramble()	//
    //////////////////
    bool T0MTHistos::T0Scramble() {
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "T0MTHistos");
        if (log.level() <= MSG::VERBOSE) log << MSG::VERBOSE << "Scrambling for " << m_time->GetName() << endmsg;
#endif
        std::string fitopt("BLR");
        if (m_settings->VerboseLevel() == 0) fitopt += "Q";
        if (m_settings->AddFitfun()) {
            fitopt += "+";
        } else {
            fitopt += "0";
        }
        // create scrambled histogram
        char scramhistname[100];
        snprintf(scramhistname, 100, "%s_scram", m_time->GetName());
        std::unique_ptr<TH1F> scramhist = std::make_unique<TH1F>(scramhistname, "scrambled histogram", m_time->GetSize() - 2,
                                                                 m_time->GetXaxis()->GetXmin(), m_time->GetXaxis()->GetXmax());

        for (int binnr = 0; binnr < m_time->GetSize(); binnr++) {
            scramhist->SetBinContent(binnr, m_time->GetBinContent(binnr) + getTLSRandomGen()->Gaus(0, m_time->GetBinError(binnr)));
            scramhist->SetBinError(binnr, m_time->GetBinError(binnr) * 1.41421356);
            if (scramhist->GetBinContent(binnr) < 0) scramhist->SetBinContent(binnr, 0);
        }
        TDirectory *cwd = gDirectory;
        if (m_dir) m_dir->cd();
        MTT0PatternRecognition scramrec;
        // perform pattern recognition
        if (!scramrec.Initialize(scramhist.get(), m_settings)) {
#ifdef NDEBUG
            MsgStream log(Athena::getMessageSvc(), "T0MTHistos");
#endif
            log << MSG::WARNING << "T0MTHistos::FitT0 for tube " << m_id << ": Scrambed pattern recognition failed!" << endmsg;
            cwd->cd();
            return false;
        }
        char scrambuffer[100];
        snprintf(scrambuffer, 100, "scrammt_t0_fermi");
        std::unique_ptr<TF1> scramm_t0_fermi = std::make_unique<TF1>();
        m_t0_fermi->Copy(*scramm_t0_fermi);
        scramm_t0_fermi->SetName(scrambuffer);
        scramm_t0_fermi->SetRange(scramrec.GetFitRangeMin(), scramrec.GetFitRangeMax());
        // set parameter names
        scramm_t0_fermi->SetParName(T0_PAR_NR_T0, "t_{0}");
        scramm_t0_fermi->SetParName(T0_PAR_NR_T, "T");
        scramm_t0_fermi->SetParName(T0_PAR_NR_BACK, "r_{b}");
        scramm_t0_fermi->SetParName(T0_PAR_NR_A, "A");
        // set fixed values
        scramm_t0_fermi->FixParameter(T0_PAR_NR_BACK, scramrec.GetBackground());
        scramm_t0_fermi->SetParameter(T0_PAR_NR_A, scramrec.GetHeight() - scramrec.GetBackground());
        // set estimates as start values
        scramm_t0_fermi->SetParameter(T0_PAR_NR_T0, scramrec.GetEstimatedT0());
        // set resonable start value for T
        scramm_t0_fermi->SetParameter(T0_PAR_NR_T, 3.0);
        // perform fit - NOTE: The return value of the Fit function is not documented!
        scramhist->Fit(scrambuffer, fitopt.c_str(), "", scramrec.GetFitRangeMin(), scramrec.GetFitRangeMax());
        // set parameter for the new fit of the original histogram
        m_time->GetListOfFunctions()->Clear();
        // set fixed values
        m_t0_fermi->FixParameter(T0_PAR_NR_BACK, scramm_t0_fermi->GetParameter(T0_PAR_NR_BACK));
        m_t0_fermi->SetParameter(T0_PAR_NR_A, scramm_t0_fermi->GetParameter(T0_PAR_NR_A));
        // set estimates as start values
        m_t0_fermi->SetParameter(T0_PAR_NR_T0, scramm_t0_fermi->GetParameter(T0_PAR_NR_T0));
        // set resonable start value for T
        m_t0_fermi->SetParameter(T0_PAR_NR_T, scramm_t0_fermi->GetParameter(T0_PAR_NR_T));
        m_time->GetListOfFunctions()->Clear();
        m_time->Fit("mt_t0_fermi", fitopt.c_str(), "", scramrec.GetFitRangeMin(), scramrec.GetFitRangeMax());
        if (m_settings->T0Settings()->UseTopChi2())
            TopChi2();
        else
            m_chi2 = m_time->GetFunction("mt_t0_fermi")->GetChisquare() / m_time->GetFunction("mt_t0_fermi")->GetNDF();
        cwd->cd();
        return true;
    }  // end T0MTHistos::T0Scramble

    void T0MTHistos::TopChi2() {
        // calculate topchi2
        m_chi2 = 0;
        int topndf = 0;
        TF1 *t0_fermi = m_time->GetFunction("mt_t0_fermi");
        Double_t min, max;
        t0_fermi->GetRange(min, max);
        int startbin = m_time->FindBin(min) + 1;
        int endbin = m_time->FindBin(max) - 1;
        for (int bin = startbin; bin < endbin; bin++) {
            float measval = m_time->GetBinContent(bin);
            float funcval = t0_fermi->Eval(m_time->GetBinCenter(bin));
            float errval = m_time->GetBinError(bin);
            // take only chi2 from top part or if the  bin content is min 10 or if the function >10
            if (measval < 10 && funcval < 10 &&
                m_time->GetBinCenter(bin) < t0_fermi->GetParameter(T0_PAR_NR_T0) + 2 * t0_fermi->GetParameter(T0_PAR_NR_T))
                continue;
            if (errval == 0)
                m_chi2 += (measval - funcval) * (measval - funcval);
            else
                m_chi2 += (measval - funcval) * (measval - funcval) / (errval * errval);
            topndf++;
        }
        if (topndf != 0) m_chi2 = m_chi2 / topndf;
    }  // end T0MTHistos::TopChi2

    void T0MTHistos::TopSlicing() {
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "T0MTHistos");
        if (log.level() <= MSG::VERBOSE) log << MSG::VERBOSE << "Slicing for " << m_time->GetName() << endmsg;
#endif
        // vector with slice chi2
        std::list<Slice> slice_chi2;
        Slice current;
        current.chi_2 = 0.0;
        current.n_bins = 0;
        TF1 *t0_fermi = m_time->GetFunction("mt_t0_fermi");
        current.min_bin = m_time->FindBin(t0_fermi->GetParameter(T0_PAR_NR_T0) + 2 * t0_fermi->GetParameter(T0_PAR_NR_T));
        Double_t min, max;
        t0_fermi->GetRange(min, max);
#ifndef NDEBUG
        if (log.level() <= MSG::VERBOSE) log << MSG::VERBOSE << current.min_bin << " " << max << endmsg;
#endif
        for (int bin = m_time->FindBin(t0_fermi->GetParameter(T0_PAR_NR_T0) + 2 * t0_fermi->GetParameter(T0_PAR_NR_T));
             bin < m_time->FindBin(max) - 1; bin++) {
            if (current.n_bins == 10) {
#ifndef NDEBUG
                if (log.level() <= MSG::VERBOSE) log << MSG::VERBOSE << current.chi_2 / current.n_bins << endmsg;
#endif
                current.max_bin = bin;
                slice_chi2.push_back(current);
                current.chi_2 = 0.0;
                current.min_bin = bin;
                current.n_bins = 0;
            }
            double measval = m_time->GetBinContent(bin);
            double funcval = t0_fermi->Eval(m_time->GetBinCenter(bin));
            double errval = m_time->GetBinError(bin);
            if (errval == 0)
                current.chi_2 += std::pow(measval - funcval, 2.0);
            else
                current.chi_2 += std::pow(measval - funcval, 2.0) / std::pow(errval, 2);
            current.n_bins++;
        }
#ifndef NDEBUG
        if (log.level() <= MSG::VERBOSE) log << MSG::VERBOSE << "number of slices: " << slice_chi2.size() << endmsg;
#endif
        std::list<Slice>::iterator it = slice_chi2.end();
        do {
            --it;
            if (it == slice_chi2.begin()) {
#ifndef NDEBUG
                if (log.level() <= MSG::VERBOSE) log << MSG::VERBOSE << "No gain in slicing!" << endmsg;
#endif
                return;
            }
        } while (it->chi_2 / static_cast<double>(it->n_bins) > 3);
        max = m_time->GetBinCenter(it->min_bin);
        m_time->GetListOfFunctions()->Clear();
        std::string fitopt("BLR");
        if (m_settings->VerboseLevel() == 0) fitopt += "Q";
        if (m_settings->AddFitfun()) {
            fitopt += "+";
        } else {
            fitopt += "0";
        }
        m_time->Fit("mt_t0_fermi", fitopt.c_str(), "", min, max);
        if (m_settings->T0Settings()->UseTopChi2())
            TopChi2();
        else
            m_chi2 = m_time->GetFunction("mt_t0_fermi")->GetChisquare() / m_time->GetFunction("mt_t0_fermi")->GetNDF();
    }  // end T0MTHistos::TopSlicing

}  // namespace MuonCalib
