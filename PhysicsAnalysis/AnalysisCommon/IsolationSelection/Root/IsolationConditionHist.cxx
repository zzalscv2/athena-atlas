/*
 Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

// $Id: IsolationConditionHist.cxx 678002 2015-06-24 15:39:36Z morrisj $
#include "IsolationSelection/IsolationConditionHist.h"
// #include <xAODPrimitives/tools/getIsolationAccessor.h>
#include <TF1.h>
#include <TH3.h>

#include <algorithm>
#include <cmath>

#include "IsolationSelection/Interp3D.h"

namespace CP {
    constexpr float MeVtoGeV = 1.e-3;
    IsolationConditionHist::IsolationConditionHist(std::string name, xAOD::Iso::IsolationType isoType, const std::string& isolationFunction,
                                                   std::unique_ptr<TH3F> efficiencyHisto3D) :
        IsolationCondition(name, isoType), m_efficiencyHisto3D(std::move(efficiencyHisto3D)) {
        m_isolationFunction = std::make_unique<TF1>(isolationFunction.c_str(), isolationFunction.c_str(), 0.0, 1000.0);

        /// check if pt is using GeV as unit
        std::string xtitle(m_efficiencyHisto3D->GetXaxis()->GetTitle());
        m_ptGeV = (xtitle.find("GeV") != std::string::npos);
    }
    bool IsolationConditionHist::accept(const xAOD::IParticle& x) const {
        const float cutValue = getCutValue(x.pt(), x.eta());
        return accessor()(x) <= cutValue * x.pt();
    }

    bool IsolationConditionHist::accept(const strObj& x) const {
        const float cutValue = getCutValue(x.pt, x.eta);
        return x.isolationValues[type()] <= cutValue * x.pt;
    }

    float IsolationConditionHist::getCutValue(const float pt, const float eta) const {
        float xpt = pt * MeVtoGeV;  // convert to GeV
        if (!m_interp)
            return m_efficiencyHisto3D->GetBinContent(m_efficiencyHisto3D->GetXaxis()->FindBin((m_ptGeV ? xpt : pt)),
                                                      m_efficiencyHisto3D->GetYaxis()->FindBin(eta),
                                                      std::min(int(m_isolationFunction->Eval(xpt)), 99));
        return m_interp->Interpol3d(m_ptGeV ? xpt : pt, eta, std::min(m_isolationFunction->Eval(xpt), 99.), m_efficiencyHisto3D);
    }
}  // namespace CP
