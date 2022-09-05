/*
 Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include "IsolationSelection/IsolationCondition.h"

#include "AthContainers/AuxElement.h"
#include "xAODPrimitives/IsolationCorrection.h"
namespace CP {

    IsolationCondition::IsolationCondition(const std::string& name, const std::vector<xAOD::Iso::IsolationType>& isoTypes) :
        m_name(name), m_isolationType(isoTypes) {
        for (const xAOD::Iso::IsolationType& iso_type : m_isolationType) { m_acc.emplace_back(toCString(iso_type)); }
    }
    IsolationCondition::IsolationCondition(const std::string& name, const std::vector<std::string>& isoTypes) : m_name(name) {
        for (const std::string& iso_type : isoTypes) {
            m_isolationType.push_back(xAOD::Iso::IsolationType(0));
            m_acc.emplace_back(iso_type);
        }
    }
    IsolationCondition::IsolationCondition(const std::string& name, xAOD::Iso::IsolationType isoType) :
        IsolationCondition(name, std::vector<xAOD::Iso::IsolationType>{isoType}) {}
    IsolationCondition::IsolationCondition(const std::string& name, std::string& isoType) :
        IsolationCondition(name, std::vector<std::string>{isoType}) {}

    unsigned int IsolationCondition::num_types() const { return m_isolationType.size(); }
    std::string IsolationCondition::name() const { return m_name; }
    xAOD::Iso::IsolationType IsolationCondition::type(unsigned int n) const { return m_isolationType[n]; }
    const SG::AuxElement::ConstAccessor<float>& IsolationCondition::accessor(unsigned int n) const { return m_acc.at(n); }
}  // namespace CP
