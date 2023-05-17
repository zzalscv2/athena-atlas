/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @class LArNoiseCorrelationMonAlg
 * @author Margherita Spalla 
 * base on LArNoiseCorrelationMon
 *
 */

#ifndef LARMONITORING_LARCORRMON_H
#define LARMONITORING_LARCORRMON_H

//inheritance:
#include "AthenaMonitoring/AthMonitorAlgorithm.h"


//LAr services:
#include "LArElecCalib/ILArPedestal.h"
#include "LArRecConditions/LArBadChannelMask.h"
#include "LArRecConditions/LArBadChannelCont.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "LArCabling/LArOnOffIdMapping.h"


//STL:
#include <string>



class LArOnlineID;
class HWIdentifier;

class LArNoiseCorrelationMonAlg final: public AthMonitorAlgorithm
{
  
  
public:
   //Delegate constructor
  using AthMonitorAlgorithm::AthMonitorAlgorithm;
   
  /** @brief Default destructor */
  virtual ~LArNoiseCorrelationMonAlg();
  
    
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms(const EventContext& ctx) const override;

  
private:
  
  /** services  */
  const LArOnlineID* m_LArOnlineIDHelper=nullptr;

  /** Handle to bad-channel mask */
  LArBadChannelMask m_bcMask;
  SG::ReadCondHandleKey<LArBadChannelCont> m_bcContKey {this, "BadChanKey", "LArBadChannel", "SG key for LArBadChan object"};
  Gaudi::Property<std::vector<std::string> > m_problemsToMask{this,"ProblemsToMask",{}, "Bad-Channel categories to mask"};

  /** Handle to cabling */
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
  
  /** Handle to pedestal */
  SG::ReadCondHandleKey<ILArPedestal> m_keyPedestal{this,"LArPedestalKey","LArPedestal","SG key of LArPedestal CDO"};

  /** list of FEBs to monitor. FEB names are expected to be strings of the form used in 'febString' method, e.g.  'BarrelCFT00Slot02'  */
  Gaudi::Property<std::vector<std::string> > m_FEBlist {this, "FEBlist", {}};

  Gaudi::Property<bool> m_plotsOFF {this, "PlotsOFF", false}; //to avoid plotting everything when online or in case the wrong custom list is passed

  /** to avoid asking for triggers in case of a calibration run*/
  Gaudi::Property<bool> m_isCalibrationRun {this, "IsCalibrationRun", false};
  
  /**bool use to mask the bad channels*/
  Gaudi::Property<bool>        m_ignoreKnownBadChannels{this, "IgnoreBadChannels", false}; 

  /** Handle to digits */
  SG::ReadHandleKey<LArDigitContainer> m_LArDigitContainerKey{this,"LArDigitContainerKey","FREE","SG key of LArDigitContainer read from Bytestream"};
  
  /** the group array **/
  Gaudi::Property<std::string> m_noiseCorrGroupName {this, "NoiseCorrGroupName", "NoiseCorr"};
  std::map<std::string,int> m_noiseCorrGroups;


  bool isGoodChannel(const HWIdentifier id,const float ped,const LArOnOffIdMapping *cabling,const LArBadChannelCont* bc) const;

  bool m_checkAbortGap=false;
  const std::string m_abortGapTrig{"HLT_noalg_cosmiccalo_L1RD1_EMPTY"};

  /** Internally used data structure*/
  struct perFeb_t {
    perFeb_t(const std::string& n) : m_febName(n) {};
    std::string m_febName;
    std::vector<std::pair<const LArDigit*,double> > m_digitsAndPed;
    std::vector<std::pair<int,double> > m_meanSum;
    std::vector<std::pair<std::pair<int,int>,double> > m_partSum;
    void sumSamples(const LArOnlineID* lArOnlineIDHelper);
  };

  std::map<HWIdentifier,perFeb_t> m_febMapModel; //Pre-fill with elements known at initialize

};

#endif
