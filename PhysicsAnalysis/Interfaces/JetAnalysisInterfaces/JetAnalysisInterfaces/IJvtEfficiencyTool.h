/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETANALYSISINTERFACES_IJVTEFFICIENCYTOOL_H
#define JETANALYSISINTERFACES_IJVTEFFICIENCYTOOL_H

#include "PATInterfaces/CorrectionCode.h"
#include "PATInterfaces/ISystematicsTool.h"
#include "xAODJet/JetContainer.h"

// TODO: Right now the systematics are defined in the old IJetJvtEfficiency header

namespace CP {

    class IJvtEfficiencyTool : public virtual CP::ISystematicsTool {
        ASG_TOOL_INTERFACE(CP::IJvtEfficiencyTool)
    public:
        virtual ~IJvtEfficiencyTool() = default;

        /**
         * @brief Calculate the efficiency scale factor for the provided jet
         * @param jet The jet whose efficiency should be calculated
         * @param[out] sf The calculated scale factor
         * @return A code signifying the status of the returned scale factor
         */
        virtual CorrectionCode getEfficiencyScaleFactor(const xAOD::Jet &jet, float &sf) const = 0;

        /**
         * @brief Calculate the inefficiency scale factor for the provided jet
         * @param jet The jet whose inefficiency should be calculated
         * @param[out] sf The calculated scale factor
         * @return A code signifying the status of the returned scale factor
         */
        virtual CorrectionCode
        getInefficiencyScaleFactor(const xAOD::Jet &jet, float &sf) const = 0;
    };
} // namespace CP

#endif //> !JETANALYSISINTERFACES_IJVTEFFICIENCYTOOL_H