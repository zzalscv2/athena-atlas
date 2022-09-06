/*
 Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include "IsolationSelection/IsolationConditionCombined.h"

#include <TF2.h>
#include <TH3.h>

#include <algorithm>
#include <cmath>

namespace CP {
    IsolationConditionCombined::IsolationConditionCombined(const std::string& name, const std::vector<xAOD::Iso::IsolationType>& isoType,
                                                           std::unique_ptr<TF1> isoFunction, const std::string& cutFunction) :
        IsolationCondition(name, isoType) {
        m_cutFunction = std::make_unique<TF1>(cutFunction.c_str(), cutFunction.c_str());
        m_isoFunction = std::move(isoFunction);
    }
    bool IsolationConditionCombined::accept(const xAOD::IParticle& x) const {
        const float cutValue = m_cutFunction->Eval(x.pt());
        std::vector<double> isoVars(num_types(), 0);
        for (unsigned int acc = 0; acc < num_types(); ++acc) {
            const FloatAccessor& acc_ele = accessor(acc);

            if (!acc_ele.isAvailable(x)) {
                Warning("IsolationConditionFormula", Form("Accessor %s is not available. Expected when using primary AODs, post-p3793 "
                                                          "derivations (only for *FixedRad or FixedCutPflow* for electrons), pre-p3517 "
                                                          "derivations (only for FC*), or pre-p3830 derivations (for other electron WPs)",
                                                          SG::AuxTypeRegistry::instance().getName(acc_ele.auxid()).c_str()));
                isoVars.push_back(FLT_MAX);
            } else
                isoVars.push_back(acc_ele(x));
        }
        const float isoValue = m_isoFunction->EvalPar(isoVars.data());
        return isoValue <= cutValue;
    }

    bool IsolationConditionCombined::accept(const strObj& x) const {
        const float cutValue = m_cutFunction->Eval(x.pt);
        std::vector<double> isoVars;
        for (unsigned int itype = 0; itype < num_types(); ++itype) isoVars.push_back(x.isolationValues[type(itype)]);
        const float isoValue = m_isoFunction->EvalPar(isoVars.data());
        return isoValue <= cutValue;
    }
}  // namespace CP
