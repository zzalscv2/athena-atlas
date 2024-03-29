/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ENHANCEDBIASWEIGHTER_ENHANCEDBIASWEIGHTER_H
#define ENHANCEDBIASWEIGHTER_ENHANCEDBIASWEIGHTER_H 1

#include "AsgTools/AsgTool.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/ToolHandle.h"
#include "Gaudi/Accumulators.h"

#include "EnhancedBiasWeighter/IEnhancedBiasWeighter.h"
#include "EnhancedBiasWeighter/ReadLumiBlock.h"

#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "LumiBlockData/BunchCrossingCondData.h"

#include <unordered_map>
#include <mutex>

/**
 * @brief Structure to encompass the data stored in a prescales XML generated by the RuleBook
 */
struct ChainDetail {
  std::string m_name;
  std::string m_lowerName;
  std::string m_comment;
  uint32_t m_counter;
  double m_prescale;
  double m_eventsPassed;
  double m_eventsPassedWeighted;
  double m_rate;
  double m_rateErr;
  double m_passthroughPrescale;
  double m_rerunPrescale;
  double m_expressPrescale;
  double m_efficiency;
  double m_efficiencyErr;
  double m_prescaledEfficiency;
  double m_prescaledEfficiencyErr;
};

/**
 * @brief Tool to calculate data-driven rates using EnhancedBias datasets. 
 */
class EnhancedBiasWeighter: public asg::AsgTool, public virtual IEnhancedBiasWeighter, public virtual DerivationFramework::IAugmentationTool { 
 public: 
   //constructor for athena can be created using special macro
   //Note: if you add a second interface to your tool, you must use: ASG_TOOL_CLASS2( ToolName, Interface1, Interface2) 
   ASG_TOOL_CLASS2( EnhancedBiasWeighter , IEnhancedBiasWeighter, DerivationFramework::IAugmentationTool)
   //add another constructor for non-athena use cases
   EnhancedBiasWeighter( const std::string& name );

   constexpr static double LHC_FREQUENCY = 11245.5; //<! LHC revolution frequency in Hz
   constexpr static uint32_t FULL_RING = 2738; //!< Number of bunches in a full ring

   /// Initialize is required by AsgTool base class
   virtual StatusCode initialize() override; //!< Check if we're MC or data, and if data check if we're to load the weighting info or read it from the input file
   virtual StatusCode finalize() override;
   // 
  
  /**
   * @brief Decorate the AOD with EnhancedBias weighting quantities such that no CVMFS or DB access is required on subsequent passes through the dAOD to perform rates.
   * addBranches is required by DerivationFramework::AugmentationTool
   */
   virtual StatusCode addBranches() const override;

  /**
   * @return The effective number of events this one event represents from online, given EnhancedBias online prescales. For MC, this is the generator weight
   * Retrieved from CVMFS XML, EventInfo (MC) or fetched from TRIG1 dAOD
   */
   virtual double getEBWeight(const xAOD::EventInfo* eventInfo) const override;
   virtual double getEBWeight(const EventContext& context) const override;

  /**
   * @return The amount of online walltime contributed by this event 
   * For data, this is based on the LB length and the number of events in the LB
   * For MC, this is fixed by the sample cross section, the inelastic cross section and the mu of the current event.
   * Retrieved from COOL and CVMFS XML or fetched from TRIG1 dAOD
   */
   virtual double getEBLiveTime(const xAOD::EventInfo* eventInfo) const override;
   virtual double getEBLiveTime(const EventContext& context) const override;

  /**
   * @return The length of the current LB in seconds. Only for data
   * Retrieved from COOL, only for data
   */
   virtual double getLBLength(const xAOD::EventInfo* eventInfo) const override;
   virtual double getLBLength(const EventContext& context) const override;

  /**
   * @return the instantaneous luminosity for the current event in units of cm-2s-1.
   * For data, based on the current LB
   * For MC, based on the event mu 
   * Retrieved from CVMFS XML or fetched from TRIG1 dAOD
   */
   virtual double getLBLumi(const xAOD::EventInfo* eventInfo) const override;
   virtual double getLBLumi(const EventContext& context) const override;

  /**
   * @return The mean luminosity of all events seen
   */
   virtual double getAverageLumi() const override;

  /**
   * @return \<mu\> over all events seen
   */
   virtual double getAverageMu() const override;
  
  /**
   * @return the fractional deadtime in the event. Reported as function per LB if value is available else a fixed number
   * There is no deadtime in MC, so for MC this will always return 1. 
   * Retrieved from CVMFS XML or fetched from TRIG1 dAOD
   */
   virtual double getDeadtime(const int lumiblock = -1) const override;

  /**
   * @return The number of colliding bunches in the Enhanced Bias run being used.
   * For MC this will return the number of bunches which corresponds to a full-ring.
   * Retrieved from CVMFS XML or fetched from TRIG1 dAOD
   */
   virtual uint32_t getPairedBunches() const override;

  /**
   * @return How far into the current train this BCID is.
   * Retrieved from the database using the BunchCrossingCondData or fetched from TRIG1 dAOD
   */
   virtual StatusCode getDistanceIntoTrain(const xAOD::EventInfo* eventInfo, uint32_t& distance) const override;

   /**
   * @return the RunNumber.
   */
   virtual uint32_t getRunNumber() const override;

  /**
   * @return If the current event was selected online by a fully unbiased trigger 
   * for MC we cannot play such games and return True so that every trigger is queried.
   * the user is expected to know that a j0 trigger running on a ttbar MC is going to underestimate
   * the rate of the j0 trigger running on MinBias.
   * Retrieved from CVMFS XML or fetched from TRIG1 dAOD
   */
   virtual bool isUnbiasedEvent(const xAOD::EventInfo* eventInfo) const override; 

  /**
   * @return If the current lumi block should be used for rates or vetoed due to online deadtime 
   * Not a concern for MC
   * Retrieved from CVMFS XML or fetched from TRIG1 dAOD
   */
   virtual bool isGoodLB(const xAOD::EventInfo* eventInfo) const override; 
   virtual bool isGoodLB(const EventContext& context) const override; 

  /**
   * @return If the tool is configured to process MC
   */
   virtual bool isMC() const override;

  /**
   * @brief Parse a presscale XML and return a ordered summary of its content
   * To make most use of the XML parsing available already in this class
   * @param prescaleXML File name of a prescales file as generated by the RuleBook
   * @return Set of L1 or HLT chain names mapped to their desired prescale
   */
   virtual std::unordered_map<std::string, ChainDetail> parsePrescaleXML(const std::string& prescaleXML) const override;

  /**
   * @return The number of BCIDs in each bunch group
   */
   virtual const std::vector<int32_t>& getBunchGroups() const override { return m_bunches; }

 private: 
    StatusCode loadWeights(); //!< Read into memory from XML event weights for this EnhancedBias run.
    StatusCode loadLumi(); //!< Read into memory this EnhancedBias run's XML
    StatusCode trackAverages(const xAOD::EventInfo* eventInfo) const; //!< Internal function to keep track of the mean instantaneous lumi & mean pileup of the EB/MC sample being processed.
    StatusCode trackAverages(const EventContext& context) const;
    /**
     * @return Internal mapping of an event number to a weight ID.
     * Retrieved from CVMFS XML or fetched from TRIG1 dAOD
     */
    int32_t getEventEBID(const xAOD::EventInfo* eventInfo) const; 
    int32_t getEventEBID(const EventContext& context) const; 

    std::string findLocalFile (const std::string& fileName) const;

    SG::ReadCondHandleKey<BunchCrossingCondData> m_bunchCrossingKey{this, "BunchCrossingKey", "BunchCrossingData", "Key BunchCrossing CDO" }; //!< Tool to get distance into bunch train

    Gaudi::Property<uint32_t> m_runNumber{this, "RunNumber", 0, "Run we're processing (if data), needed at initialize to locate and read in extra configuration."};
    Gaudi::Property<bool> m_errorOnMissingEBWeights{this, "ErrorOnMissingEBWeights", false, "If true, Throws error if EB weights are missing."};
    Gaudi::Property<bool> m_enforceEBGRL{this, "EnforceEBGRL", true, "Each Enhanced Bias run has a 'good run list' style veto on some LB. If this flag is true, events in these LB get weight 0"};
    Gaudi::Property<bool> m_useBunchCrossingData{this, "UseBunchCrossingData", true, "BunchCrossing data requires CONDBR2 access. Can be disabled here if this is a problem."};
    Gaudi::Property<bool> m_isMC{this, "IsMC", false,  "MC mode? If so we need a cross section and filter efficiency"};
    Gaudi::Property<double> m_mcCrossSection{this, "MCCrossSection", 0.0, "If running over MC. The process cross section in nb (AMI gives thins in nb)"};
    Gaudi::Property<double> m_mcFilterEfficiency{this, "MCFilterEfficiency", 1.0, "If running over MC. The process filter efficiency (0.0-1.0)"}; 
    Gaudi::Property<double> m_mcKFactor{this, "MCKFactor", 1.0, "If running over MC. The process filter efficiency (0.0-1.0)"}; 
    Gaudi::Property<bool> m_mcIgnoreGeneratorWeights{this, "MCIgnoreGeneratorWeights", false, "If running over MC. Flag to ignore the generator weight."}; 
    Gaudi::Property<double> m_inelasticCrossSection{this, "InelasticCrossSection", 8e-26, "Inelastic cross section in units cm^2. Default 80 mb at 13 TeV."};
    Gaudi::Property<std::string> m_weightsDirectory {this, "EBWeightsDirectory", "", "Path to directory with EB XML weights files, if empty they will be read from calibration area"};  


    double m_deadtime; //!< Online deadtime to correct for in rate prediction. Currently a constant for the EB period
    uint32_t m_pairedBunches; //!< Online number of paired bunches.
    double m_mcModifiedCrossSection; //!< Product of xsec, filter & kfactor. In units of cm
    std::unordered_map<uint64_t, int32_t> m_eventNumberToIdMap; //!< Map event number to a weighting ID

    mutable std::unordered_map<uint32_t, float> m_eventLivetime ATLAS_THREAD_SAFE; //!< Cache of per-event livetime as a function of LB [LB -> effective walltime per event]
    mutable std::mutex m_mutex; //!< Protection for above map
    mutable Gaudi::Accumulators::AveragingCounter<double> m_lumiAverage ATLAS_THREAD_SAFE; //!< The average instantaneous lumionosity over all events.
    mutable Gaudi::Accumulators::AveragingCounter<double> m_muAverage ATLAS_THREAD_SAFE; //!< The average mu over all events.

    std::unordered_map<int32_t, double>    m_idToWeightMap; //!< Map a weighting ID to a Enhanced Bias event weight
    std::unordered_map<int32_t, uint8_t>   m_idToUnbiasedMap; //!< Map a weighting ID to a flag if this weight is from an unbiased (RD) trigger online
    std::unordered_map<uint32_t, uint32_t> m_eventsPerLB; //!< Map of how many EnhancedBias events were recorded per LB 
    std::unordered_map<uint32_t, double>   m_lumiPerLB; //!< Map of instantaneous luminosity per LB 
    std::unordered_map<uint32_t, uint8_t>  m_goodLB;  //!< Like a Good Run List flag for EnhancedBias runs.
    std::unordered_map<uint32_t, double>   m_deadtimePerLB; //!< Map of average deadtime per LB 

    std::vector<int32_t> m_bunches; //!< Number of BCIDs in each bunch group

    ReadLumiBlock m_readLumiBlock; //!< Cache lumi block lengths. Get this from COOL.
}; 

#endif //> !ENHANCEDBIASWEIGHTER_ENHANCEDBIASWEIGHTER_H
