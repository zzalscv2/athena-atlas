/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "jFexFwdElRoIThresholdsTool.h"

uint64_t jFexFwdElRoIThresholdsTool::getPattern(const xAOD::jFexFwdElRoI& roi,
                                                const RoIThresholdsTool::ThrVec& menuThresholds,
                                                const TrigConf::L1ThrExtraInfoBase& /*menuExtraInfo*/) const {
    
    // Get RoI properties (once, rather than for every threshold in the menu)
    unsigned int et     = roi.et();
    int          ieta   = roi.menuEta();
    uint64_t     thresholdMask = 0;

    // Iterate through thresholds and see which ones are passed
    for (const std::shared_ptr<TrigConf::L1Threshold>& thrBase : menuThresholds) {
        std::shared_ptr<TrigConf::L1Threshold_jEM> thr = std::static_pointer_cast<TrigConf::L1Threshold_jEM>(thrBase);
        
        //Checking et thresholds
        if (et > thr->thrValueMeV(ieta)) {
            thresholdMask |= (1<<thr->mapping());
        }
        
        ATH_MSG_DEBUG("jFEX FwdEl HLT seeding for ("<< thr->name() <<"): et=" << et << " > "<<thr->thrValueMeV(ieta));
    }
    return thresholdMask;        

}
