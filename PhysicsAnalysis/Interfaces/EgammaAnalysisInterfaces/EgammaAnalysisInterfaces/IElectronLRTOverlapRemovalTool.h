/*
 Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef IELECTRONLRTOVERLAPREMOVALTOOL__H
#define IELECTRONLRTOVERLAPREMOVALTOOL__H

// C++ include(s):
#include <set>

// Framework include(s):
#include "AsgTools/IAsgTool.h"

// EDM include(s):
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/ElectronContainer.h"

// Local include(s):

/**
 * @mainpage ElectronLRTOverlapRemovalTool
 *
 */
namespace CP
{

    class IElectronLRTOverlapRemovalTool : public virtual asg::IAsgTool
    {
        ///
        /// @class IElectronLRTOverlapRemovalTool
        /// @brief Interface class.
        /// Abstract interface class. Provides the user interface for the
        /// ElectronLRTOverlapRemovalTool class.

        ASG_TOOL_INTERFACE(CP::IElectronLRTOverlapRemovalTool)

    public:
        /// Allow to specify a number of supported overlap removal strategies.
        // default strategy: Discard the electron with the looser ID in the case of a shared
        //                   cluster. in the case of a 'tie', choose the standard electron.
        // prompt strategy: Require electrons to pass VeryLooseNoPix ID WP. For those passing,
        //                   discard those from the LRT collection that share a cluster
        //                   with the standard collection 
        // removeFailing strategy: Remove electrons failing ID, but don't do overlap removal
        //                   on electrons that share clusters. NOT FOR ANALYSIS, for validation
        //                   purposes only                   
        typedef enum
        {
            defaultStrategy = 0,
            promptStrategy = 1,
            passThrough = 2
        } overlapStrategy;

        /// Check the overlap between the prompt and LRT electron collections.
        /// Saves a set of points to electrons to be removed.
        virtual void checkOverlap(const xAOD::ElectronContainer &promptCollection,
                                  const xAOD::ElectronContainer &lrtCollection,
                                  std::set<const xAOD::Electron *> &ElectronsToRemove) const = 0;
    };
    // class IElectronLRTOverlapRemovalTool

} // namespace CP

#endif /* IELECTRONLRTOVERLAPREMOVALTOOL__H */
