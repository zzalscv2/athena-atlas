/*
 Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
 */

#ifndef ISOLATIONSELECTION_ISOLATIONCONDITION_H
#define ISOLATIONSELECTION_ISOLATIONCONDITION_H

#include <IsolationSelection/Defs.h>
#include <xAODBase/IParticle.h>
#include <xAODPrimitives/IsolationType.h>
#include <xAODPrimitives/tools/getIsolationAccessor.h>

#include <map>
#include <memory>
#include <vector>

#include "AthContainers/AuxElement.h"

struct strObj {
    float pt{0.f};
    float eta{0.f};
    std::vector<float> isolationValues;
    xAOD::Type::ObjectType type{xAOD::Type::ObjectType::EventInfo};
};

namespace CP {
    class IsolationCondition {
    public:
        IsolationCondition(const std::string& name, xAOD::Iso::IsolationType isoType);
        IsolationCondition(const std::string& name, const std::vector<xAOD::Iso::IsolationType>& isoTypes);
        IsolationCondition(const std::string& name, std::string& isoType);
        IsolationCondition(const std::string& name, const std::vector<std::string>& isoTypes);

        IsolationCondition(const IsolationCondition& rhs) = delete;
        IsolationCondition& operator=(const IsolationCondition& rhs) = delete;
        virtual ~IsolationCondition() = default;

        std::string name() const;

        unsigned int num_types() const;
        xAOD::Iso::IsolationType type(unsigned int n = 0) const;
        const FloatAccessor& accessor(unsigned int n = 0) const;

        virtual bool accept(const xAOD::IParticle& x) const = 0;
        virtual bool accept(const strObj& x) const = 0;

    private:
        std::string m_name;
        std::vector<xAOD::Iso::IsolationType> m_isolationType;
        std::vector<FloatAccessor> m_acc;
    };
}  // namespace CP
#endif
