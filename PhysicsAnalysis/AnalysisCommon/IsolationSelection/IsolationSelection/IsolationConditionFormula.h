// Dear emacs, this is -*- c++ -*-

/*
 Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
 */

#ifndef ISOLATIONSELECTION_ISOLATIONCONDITIONFORMULA_H
#define ISOLATIONSELECTION_ISOLATIONCONDITIONFORMULA_H

#include <memory>

#include "IsolationSelection/IsolationCondition.h"

// Forward Declaration(s)
class TF1;
class TH3F;

namespace CP {
    class IsolationConditionFormula : public IsolationCondition {
    public:
        IsolationConditionFormula(std::string name, xAOD::Iso::IsolationType isoType, const std::string& cutFunction,
                                  bool invertCut = false);
        IsolationConditionFormula(std::string name, std::string isoType, const std::string& cutFunction, bool invertCut = false);
        virtual ~IsolationConditionFormula() = default;

        virtual bool accept(const xAOD::IParticle& x) const override;
        virtual bool accept(const strObj& x) const override;

    private:
        std::shared_ptr<TF1> m_cutFunction{nullptr};
        bool m_invertCut{false};
    };
}  // namespace CP
#endif
