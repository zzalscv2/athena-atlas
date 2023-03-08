/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#ifndef PMGTOOLS_IPMGTRUTHWEIGHTTOOL_H
#define PMGTOOLS_IPMGTRUTHWEIGHTTOOL_H

// STL include(s):
#include <string>
#include <vector>

// EDM include(s):
#include "PATInterfaces/IReentrantSystematicsTool.h"
#include <xAODEventInfo/EventInfo.h>

namespace PMGTools
{
  /// Interface for xAOD Truth Weight Tool which retrieves
  /// Meta Data from a truth record to interface the event
  /// weights
  ///
  /// @author James Robinson <james.robinson@cern.ch>
  ///
  class IPMGTruthWeightTool: public virtual CP::IReentrantSystematicsTool
  {
    /// Declare the interface that the class provides
    ASG_TOOL_INTERFACE(xAOD::IPMGTruthWeightTool)

  public:
    /// Return vector of weight names (descriptions) from meta data
    virtual const std::vector<std::string>& getWeightNames() const = 0;

    /// Return the weight corresponding to weightName for this event
    virtual float getWeight(const xAOD::EventInfo* evtInfo, const std::string& weightName) const = 0;

    /// Check if a weight called weightName exists
    virtual bool hasWeight(const std::string& weightName) const = 0;

    /// Return the weight corresponding to the current systematics
    virtual float getSysWeight(const xAOD::EventInfo* evtInfo, const CP::SystematicSet& sys) const = 0;

    /// Return the weight index corresponding to the current systematics
    virtual size_t getSysWeightIndex(const CP::SystematicSet& sys) const = 0;

  }; // class IPMGTruthWeightTool

} // namespace xAOD

#endif // PMGTOOLS_IPMGTRUTHWEIGHTTOOL_H
