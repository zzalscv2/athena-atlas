//Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


/**
 * @file  LArRampValidatonAlg.h
 * @author Jessica Leveque <jessica.leveque @cern.ch>
 * @date March 2008
 * @brief Uses the LArCalibValidationAlg to validate ramp runs.
 */




#ifndef LARRAMPVALIDATIONALG_H
#define LARRAMPVALIDATIONALG_H
 
#include <vector>
#include <string>
 
#include "LArRawConditions/LArRampComplete.h"
#include "LArRawConditions/LArRawRampContainer.h"
#include "LArCalibDataQuality/LArCalibValidationAlg.h"
#include "CaloIdentifier/CaloCellGroup.h"

typedef LArCalibValidationAlg<LArRampComplete,ILArRamp> LArRampValidationBase;


/** 
  * @class LArCalibValidationAlg
  * @brief Algorithm to validate LAr Ramp
  * Algorithm to validate LAr Ramp runs by comparing them channel-by-channel with a reference run.
  * This is the first implementation, more inteded as test-case for the base-class. Needs certainly some
  * refinement.
  */
                                                   
class LArRampValidationAlg: public LArRampValidationBase {
 
 public:
  /** 
   * @brief Regular algorithm constructor
   */
  LArRampValidationAlg (const std::string& name, ISvcLocator* pSvcLocator);
  
 private:
  /** @brief Method to validate the ramps single readout channels
   */
  bool validateChannel(const LArCondObj& ref, const LArCondObj& val, const HWIdentifier chid, const int gain, const LArOnOffIdMapping *cabling, const LArBadChannelCont *bcCont) override final;

  LArCondObj getRefObj(const HWIdentifier chid, const int gain) const override final;
     
  /** @brief Summary method executed after the loop over all channels */
  virtual StatusCode summary(const LArOnOffIdMapping *cabling, const LArBadChannelCont *bcCont) override;
  
  /** @brief Executed before the loop over all channels to reset global sums */
  virtual StatusCode preLoop() override;

  /** To store Gain Keys for Raw Ramps (job-Property)*/
  std::vector<std::string> m_contKey;
  /** Raw Ramp time tolerance (in ns) (job-Property)*/
  float m_rawrampTimeTolerance;
  /** Raw Ramp ADC (job-Property)*/
  float m_rawrampTimeADC;

  /** Ramp tolerance as init string for CellGroupObject (job-Property)*/
  std::vector<std::string> m_toleranceInit;

  /** Ramp tolerance as CaloCellGroup object */
  CaloCellGroupList m_tolerance;

  /** Ramp tolerance (FEB average) as CaloCellGroup object (job-Property)*/
  std::vector<std::string> m_toleranceInitFEB;

  /** Ramp tolerance (FEB average) as initializer string CaloCellGroup object */
  CaloCellGroupList m_toleranceFEB;

  /** To check if Raw Ramps are found */
  bool m_hasRawRampContainer;

  //The following is for keeping track of entire FEBs
  /** @brief Method to compare FEB averages */
  bool febSummary(const LArOnOffIdMapping *cabling, const LArBadChannelCont *bcCont);

  //The following is used to look for channels deviating from average
  bool deviateFromAvg(const LArCondObj& val, const HWIdentifier chid, const int gain, const LArOnOffIdMapping *cabling, const LArBadChannelCont *bcCont);

  class DataPerRegion {
  public:
    double       rampVal=.0;
    double       rmsVal=.0;
    double       rampRef=.0;
    double       rmsRef=.0;
    unsigned     nEntries=0;
  };
  

  std::unordered_map<HWIdentifier,DataPerRegion> m_vDataPerFEB;
  std::unordered_map<Identifier,DataPerRegion> m_vDataPerSector;

  LArRawRampContainer* m_rawRampContainer;

  //The following is for keeping track of the global average
  double m_rampGlobalVal,m_rmsGlobalVal;
  double m_rampGlobalRef,m_rmsGlobalRef;
  unsigned m_nEntriesGlobal;
};


#endif

