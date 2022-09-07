/*
 Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include "IsolationSelection/IsolationConditionFormula.h"

#include <TF1.h>
#include <TH3.h>
#include <TString.h>

#include <algorithm>
#include <cmath>

namespace CP {
    IsolationConditionFormula::IsolationConditionFormula(std::string name, xAOD::Iso::IsolationType isoType, const std::string& cutFunction,
                                                         bool invertCut) :
        IsolationCondition(name, isoType) {
        m_cutFunction = std::make_unique<TF1>(cutFunction.c_str(), cutFunction.c_str());
        m_invertCut = invertCut;
    }
    IsolationConditionFormula::IsolationConditionFormula(std::string name, std::string isoType, const std::string& cutFunction,
                                                         bool invertCut) :
        IsolationCondition(name, isoType) {
        m_cutFunction = std::make_unique<TF1>(cutFunction.c_str(), cutFunction.c_str());
        m_invertCut = invertCut;
    }
    bool IsolationConditionFormula::accept(const xAOD::IParticle& x) const {
        const float cutVal = m_cutFunction->Eval(x.pt());
        const FloatAccessor& acc = accessor();
        if (!acc.isAvailable(x)) {
            Warning("IsolationConditionFormula", "Accessor %s is not available. Expected when using primary AODs, post-p3793 "
                                                 "derivations (only for *FixedRad or FixedCutPflow*  for electrons), pre-p3517 "
                                                 "derivations (only for FC*), or pre-p3830 derivations (for other electron WPs)",
                                                 SG::AuxTypeRegistry::instance().getName(acc.auxid()).c_str());
            return false;
        }
        if (!m_invertCut) return acc(x) <= cutVal;
        return acc(x) > cutVal;
    }

    bool IsolationConditionFormula::accept(const strObj& x) const {
        const float cutVal = m_cutFunction->Eval(x.pt);
        if (!m_invertCut) return x.isolationValues[type()] <= cutVal;
        return x.isolationValues[type()] > cutVal;
    }

}  // namespace CP
