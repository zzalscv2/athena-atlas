/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @class CaloSuperCellBuilderDriver
 * @author Denis Oliveira Damazio Denis.Oliveira.Damazio@cern.ch
 * @author Walter Hopkins
 * @brief Raw channel builder adapted for trigger use
 *
 * The conversion LArDigit -> energy is done on demand
 * Slight modification added in 2017 to add Tile trigger towers to the supercell container
 *   */


#ifndef CALOREC_CALOSUPERCELLBUILDERDRIVER_H
#define CALOREC_CALOSUPERCELLBUILDERDRIVER_H

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/StoreGateSvc.h"
#include "AthenaBaseComps/AthAlgorithm.h"

#include "LArCabling/LArSuperCellCablingTool.h"
#include "CaloIdentifier/CaloGain.h"

#include "LArIdentifier/LArOnline_SuperCellID.h"

#include "CaloRec/ITTToSCTool.h"
#include "LArROD/LArRawChannelBuilderParams.h"
#include "LArROD/ILArRawChannelBuilderToolBase.h"
#include "LArROD/ILArRawChannelBuilderADC2EToolBase.h"
#include "LArROD/ILArRawChannelBuilderPedestalToolBase.h"

#include <vector>
#include <string>

class LArDigit;
class LArRawChannelContainer;

class HWIdentifier;
class Identifier;

class CaloSuperCellBuilderDriver : public AthAlgorithm
{
 public:
  
  CaloSuperCellBuilderDriver (const std::string& name,
                       ISvcLocator* pSvcLocator);
  virtual ~CaloSuperCellBuilderDriver();
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();
  void initEventTools();
  bool buildLArCell(const LArDigit* digit, int& energy, int& time, CaloGain::CaloGain& gain, int& prov,
		    MsgStream* pLog = NULL);
  void finishEventTools();
  
  template<typename T>
    StatusCode retrieveStoreGate(const T*& object, const std::string& key);
  
  template<typename T>
    StatusCode retrieveDetectorStore(const T*& object, const std::string& key);

  template<typename T>
    StatusCode regHandleDetectorStore(const T& object, const std::string& key);
  
  template<typename T>
    StatusCode retrieveTool(T*& object, const std::string& key);
  
 private:
  
  float pedestal(MsgStream* pLog);
  
  void ADC2energy(MsgStream* pLog);

  LArRawChannelBuilderParams* m_params;
  
  typedef ToolHandleArray<ILArRawChannelBuilderToolBase>          builderToolVector;
  typedef ToolHandleArray<ILArRawChannelBuilderADC2EToolBase>     adc2eToolVector;
  typedef ToolHandleArray<ILArRawChannelBuilderPedestalToolBase>  pedestalToolVector;
  
  //Services & Tools 
  StoreGateSvc*               m_storeGateSvc;
  StoreGateSvc*               m_detStore;
  IToolSvc*                   m_toolSvc;
  
  const LArOnline_SuperCellID*          m_onlineHelper;
  
  std::string m_DataLocation, m_ChannelContainerName;
  
  builderToolVector           m_buildTools;
  
  adc2eToolVector             m_adc2eTools;
  
  pedestalToolVector          m_pedestalTools;
  
  float                       m_oldPedestal;
  HWIdentifier                m_oldIdentifier;
  
  int                         m_checkSamples;
  
  ToolHandle<LArSuperCellCablingTool> m_larCablingSvc;
  ToolHandle<ITTToSCTool> m_ttToSCTool;
  
  std::vector<float>          m_ramps;
  
  bool m_buildDiscChannel ; 
  bool m_addTileTriggerTowers ; 
  int	 m_defaultShiftTimeSamples;
  int	 m_bcs;
  int	 m_counter;
};

#include "CaloRec/CaloSuperCellBuilderDriver.icc"

#endif
