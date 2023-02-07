/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PMGTOOLS_PMGTRUTHWEIGHTTOOL_H
#define PMGTOOLS_PMGTRUTHWEIGHTTOOL_H

// EDM include(s):
#include "AsgTools/AsgMetadataTool.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthMetaData.h"
#include "xAODTruth/TruthMetaDataContainer.h"

// Interface include(s):
#include "PMGAnalysisInterfaces/IPMGTruthWeightTool.h"
#include "PATInterfaces/SystematicsCache.h"


namespace PMGTools
{
  /// Implementation for the xAOD truth meta data weight tool
  ///
  /// @author James Robinson <james.robinson@cern.ch>
  ///
  class PMGTruthWeightTool : public virtual IPMGTruthWeightTool, public asg::AsgMetadataTool
  {
    /// Create a proper constructor for Athena
    ASG_TOOL_CLASS2(PMGTruthWeightTool, PMGTools::IPMGTruthWeightTool, IReentrantSystematicsTool)

  public:
    /// Create a constructor for standalone usage
    PMGTruthWeightTool(const std::string& name);

    /// @name Function(s) implementing the asg::IAsgTool interface
    /// @{

    /// Function initialising the tool
    virtual StatusCode initialize() override;

    /// @}

    /// @name Function(s) implementing the IPMGTruthWeightTool interface
    /// @{

    /// Implements interface from IPMGTruthWeightTool
    virtual const std::vector<std::string>& getWeightNames() const override;

    /// Implements interface from IPMGTruthWeightTool
    virtual float getWeight(const xAOD::EventInfo* evtInfo, const std::string& weightName) const override;

    /// Implements interface from IPMGTruthWeightTool
    virtual bool hasWeight(const std::string& weightName) const override;

    /// Implements interface from IPMGTruthWeightTool
    virtual float getSysWeight(const xAOD::EventInfo* evtInfo, const CP::SystematicSet& sys) const override;

    /// Implements interface from IPMGTruthWeightTool
    virtual size_t getSysWeightIndex(const CP::SystematicSet& sys) const override;

    /// @}

    /// @name Function(s) implementing the ISystematicsTool interface
    /// @{

    /// Implements interface from ISystematicsTool
    virtual CP::SystematicSet affectingSystematics() const override;

    /// Implements interface from ISystematicsTool
    virtual CP::SystematicSet recommendedSystematics() const override;

    /// @}

  protected:
    /// @name Callback function(s) from AsgMetadataTool
    /// @{

    /// Function called when a new input file is opened
    virtual StatusCode beginInputFile() override;

    /// @}

    /// Helper function for retrieving MC channel number from file metadata
    StatusCode getMCChannelNumber(uint32_t &mcChannelNumber);

    /// Loads weight information from xAOD::TruthMetaDataContainer
    StatusCode loadMetaData();

    /// Loads weight information from POOL using HepMCWeightNames
    StatusCode loadPOOLMetaData();

    /// Validate weight caches
    StatusCode validateWeightLocationCaches();

    /// Clear caches
    void clearWeightLocationCaches();

    /// Stores the meta data record name
    std::string m_metaName;

    /// Current MC channel number
    uint32_t m_mcChannelNumber{};

    /// Ptr to the meta data container for this file
    const xAOD::TruthMetaDataContainer* m_metaDataContainer{nullptr};

    /// Flag to indicate whether the xAOD::TruthMetaData objects have incorrect McChannelNumber
    bool m_useChannelZeroInMetaData{false};

    /// Available weight names for this file
    std::vector<std::string> m_weightNames;

    /// Weight names to indices of available weights in this file
    std::unordered_map<std::string, size_t> m_weightIndices;

    /// Systematics to indices of available weights
    CP::SystematicsCache<std::size_t> m_calibCache {this};
  };
} // namespace PMGTools

#endif // PMGTOOLS_PMGTRUTHWEIGHTTOOL_H
