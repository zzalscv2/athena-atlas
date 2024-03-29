/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  LArCaliWaveValidatonAlg.h
 * @author Ilija Vukotic <ilijav @cern.ch>
 * @date Feb 2008
 * @brief Uses the LArCalibValidationAlg to validate delay runs.
 */



#ifndef LARCALIWAVEVALIDATIONALG_H
#define LARCALIWAVEVALIDATIONALG_H
 
#include <vector>
#include <string>
 
#include "LArRawConditions/LArCaliWaveContainer.h"
#include "LArCalibDataQuality/LArCalibValidationAlg.h"
#include "LArRawConditions/LArWaveHelper.h"
#include "CaloIdentifier/CaloCellGroup.h"

typedef LArCalibValidationAlg<LArCaliWaveContainer,LArCaliWaveContainer> LArCaliWaveValidationBase;


/** 
  * @class LArCalibValidationAlg
  * @brief Algorithm to validate LAr CaliWaves
  * Algorithm to validate LAr Delay runs by comparing them channel-by-channel with a reference run.
  * This is the first implementation, more inteded as test-case for the base-class. Needs certainly some
  * refinement.
  */
                                                   
class LArCaliWaveValidationAlg: public LArCaliWaveValidationBase {
 
 public:
  /** 
   * @brief Regular algorithm constructor
   */
  LArCaliWaveValidationAlg (const std::string& name, ISvcLocator* pSvcLocator);
  
 private:
  /** @brief Method to validate the pedestal single readout channels
   */
  bool validateChannel(const LArCondObj& ref, const LArCondObj& val, const HWIdentifier chid, const int gain, const LArOnOffIdMapping *cabling, const LArBadChannelCont *bcCont) override final;

  LArCondObj getRefObj(const HWIdentifier chid, const int gain) const override final{
      return m_reference->get(chid,gain);
  }


  /** @brief Summary method executed after the loop over all channels */
  virtual StatusCode summary(const LArOnOffIdMapping *cabling, const LArBadChannelCont *bcCont) override;
  
  /** @brief Executed before the loop over all channels to reset global sums */
  virtual StatusCode preLoop() override;


  /** Amplitude tolerance (in permills) (job-Property)*/
  std::vector<std::string> m_ampToleranceInit;
  CaloCellGroupList m_ampTolerance;

  /** Tolerance for the average amplitude of one FEB (in permills) (job-Property) */
  std::vector<std::string> m_ampToleranceFEBInit;
  CaloCellGroupList m_ampToleranceFEB;

  /** FWHM tolerance (in permills) (job-Property) */  
  std::vector<std::string> m_fwhmToleranceInit;
  CaloCellGroupList m_fwhmTolerance;

  /** Tolerance for the average FWHM  of one FEB (in permills) (job-Property) */  
  std::vector<std::string> m_fwhmToleranceFEBInit;
  CaloCellGroupList m_fwhmToleranceFEB;

  bool m_timeShift;

  //The following is for keeping track of entire FEBs
  /** @brief Method to compare FEB averages */
  bool febSummary(const LArOnOffIdMapping *cabling, const LArBadChannelCont *bcCont);

  class DataPerFEB {
  public:
    DataPerFEB () : chid(0), febid(0), gain(0),
		   ampVal(0.),fwhmVal(0.),ampRef(0.),fwhmRef(0.), nEntries(0) {}
    DataPerFEB (const HWIdentifier cid, const HWIdentifier fid, int g) :  
      chid(cid), febid(fid), gain(g), ampVal(0.),fwhmVal(0.),ampRef(0.),fwhmRef(0.),nEntries(0) {}
    HWIdentifier chid;
    HWIdentifier febid;
    int          gain;
    double       ampVal;
    double       fwhmVal;
    double       ampRef;
    double       fwhmRef;
    unsigned     nEntries;
  };
  
  std::vector<DataPerFEB> m_vDataPerFEB;

  //The following is for keeping track of the global average
  double m_ampGlobalVal=0;
  double m_fwhmGlobalVal=0;
  double m_ampGlobalRef=0;
  double m_fwhmGlobalRef=0;
  unsigned m_nEntriesGlobal=0;

  LArWaveHelper m_waveHelper;

};


#endif

