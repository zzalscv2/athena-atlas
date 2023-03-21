/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARMONITORING_LARDIGITALTRIGGMON_H
#define LARMONITORING_LARDIGITALTRIGGMON_H

//inheritance:
#include "AthenaMonitoring/AthMonitorAlgorithm.h"

//LAr services:
#include "LArElecCalib/ILArPedestal.h"

//STL:
#include <string>

#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "GaudiKernel/ToolHandle.h"

//Events infos:
#include "xAODEventInfo/EventInfo.h"
#include "LumiBlockComps/ILumiBlockMuTool.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArRawEvent/LArRawSCContainer.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LumiBlockData/LuminosityCondData.h"
#include <mutex>

class LArEM_ID;
class LArOnlineID;
class HWIdentifier;
class LArOnlineIDStrHelper;
class LArOnOffIdMapping;
class CaloCell_SuperCell_ID;

class LArDigitalTriggMonAlg: public AthMonitorAlgorithm
{
  
  
public:
  LArDigitalTriggMonAlg(const std::string& name, ISvcLocator* pSvcLocator);
  
  /** @brief Default destructor */
  virtual ~LArDigitalTriggMonAlg();
  
  virtual StatusCode initialize() override;
  
  virtual StatusCode fillHistograms(const EventContext& ctx) const override;
  
private:  
  /**declaration variables used in joboptions*/
  Gaudi::Property<std::string> m_MonGroupName  {this, "LArDigitTriggMonGroupName", "LArDigitTriggMonGroup"};
  /**Range to check for the max sample. If min and max=0, the range is set dynamically	*/
  Gaudi::Property<int>         m_AskedSampleRangeLow{this, "SampleRangeLow", -1};
  Gaudi::Property<int>         m_AskedSampleRangeUp {this, "SampleRangeUp", -1};
  /**default saturation cuts*/
  Gaudi::Property<int>         m_ADCsatureCut  {this, "ADCSatureCut", 4095};
  /**default cut to select events*/
  Gaudi::Property<int>         m_SigmaCut      {this, "SigmaCut", 5};
  /** Use the SampleMax expected and the SampleNumber from DB*/
  Gaudi::Property<int>         m_ExpectedSampleMax {this, "ExpectedSampleMax", 0};
  Gaudi::Property<int>         m_SampleNumberFromDB{this, "SampleNumberFromDB", 0};
  /** Switch to online/offline mode*/
  Gaudi::Property<bool>        m_IsOnline      {this, "IsOnline", false}; 
  Gaudi::Property<int>         m_NLatomeBins{this, "NLatomeBins", 117};


  Gaudi::Property<std::string>        m_EtName{this, "EtName", "Dummy"};
  Gaudi::Property<std::string>        m_AdcName{this, "AdcName", "Dummy"};


  //Added for Stream aware:
  /** Give the name of the streams you want to monitor:*/
  Gaudi::Property<std::vector<std::string> >  m_streams {this, "Streams", {""}};
  
  //Histogram group names
  Gaudi::Property<std::string> m_scMonGroupName {this, "SCMonGroup", "SC"};

  //\** Handle to cabling *\/ 
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this, "CablingSCKey","LArOnOffIdMapSC","SG Key of LArOnOffIdMapping object"}; 

  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this,"EventInfo","EventInfo","SG Key of EventInfo object"};

  /** Handle to digits */
  SG::ReadHandleKey<LArDigitContainer> m_digitContainerKey{this,"LArDigitContainerKey","dummy","SG key of LArDigitContainer read from Bytestream"}; //raw ADC 12 bits - ADC axis up to 4096
  SG::ReadHandleKey<LArRawSCContainer> m_rawSCContainerKey{this,"LArRawSCContainerKey","dummy","SG key of LArRawSCContainer read from Bytestream"};

  SG::ReadHandleKey<LArRawSCContainer> m_rawSCEtRecoContainerKey{this,"LArRawSCEtRecoContainerKey","SC_ET_RECO","SG key of LArRawSCContainer read from Bytestream"};
  
  /** Handle to pedestal */
  SG::ReadCondHandleKey<ILArPedestal>    m_keyPedestalSC{this,"LArPedestalKeySC","LArPedestalSC","SG key of LArPedestal CDO"};

  /** Handle to Super Cell DD Manager */
  SG::ReadCondHandleKey<CaloSuperCellDetDescrManager> m_caloSuperCellMgrKey{this
      ,"CaloSuperCellDetDescrManager"
      ,"CaloSuperCellDetDescrManager"
      ,"SG key of the resulting CaloSuperCellDetDescrManager"};

  SG::ReadDecorHandleKey<xAOD::EventInfo> m_actualMuKey {this, "actualInteractionsPerCrossing",
           "EventInfo.actualInteractionsPerCrossing","Decoration for Actual Number of Interactions Per Crossing"};
  // SC_ET_ID cuts on taus selection, SC_ET just takes everything

  /* Id helpers */
  const LArOnlineID* m_LArOnlineIDHelper;
  const LArEM_ID* m_LArEM_IDHelper;
  const CaloCell_SuperCell_ID*  m_SCID_helper;


  int WhatPartition(HWIdentifier id, int side) const; 
  int getXbinFromSourceID(int sourceID) const;
  int getXbinPerSideFromSourceID(int sourceID) const;

  //Enumerate layer-types, ignoring sides. Useful for configuration that is per-definition symmetric 
  enum LayerEnumNoSides{EMBPNS=0, EMB1NS, EMB2NS, EMB3NS, HEC0NS, HEC1NS, HEC2NS, HEC3NS,
                        EMECPNS,EMEC1NS,EMEC2NS,EMEC3NS,FCAL1NS,FCAL2NS,FCAL3NS,MAXLYRNS};

  //Mapping of CaloCell nomencature to CaloCellMonitoring nomencature
  const std::map<unsigned,LayerEnumNoSides> m_caloSamplingToLyrNS{ 
    {CaloSampling::PreSamplerB, EMBPNS},{CaloSampling::EMB1,EMB1NS},{CaloSampling::EMB2,EMB2NS},{CaloSampling::EMB3,EMB3NS},         //LAr Barrel
    {CaloSampling::PreSamplerE, EMECPNS},{CaloSampling::EME1,EMEC1NS}, {CaloSampling::EME2,EMEC2NS}, {CaloSampling::EME3,EMEC3NS},   //LAr Endcap 		
    {CaloSampling::HEC0,HEC0NS}, {CaloSampling::HEC1,HEC1NS}, {CaloSampling::HEC2,HEC2NS}, {CaloSampling::HEC3,HEC3NS},              //Hadronic endcap
    {CaloSampling::FCAL0,FCAL1NS}, {CaloSampling::FCAL1,FCAL2NS}, {CaloSampling::FCAL2,FCAL3NS}                                      //FCAL
  };
  
  StringArrayProperty m_layerNames{this, "LayerNames", {"EMBPA", "EMBPC", "EMB1A", "EMB1C", "EMB2A", "EMB2C", "EMB3A", "EMB3C",
	"HEC0A", "HEC0C", "HEC1A", "HEC1C", "HEC2A", "HEC2C", "HEC3A", "HEC3C",
	"EMECPA", "EMECPC", "EMEC1A", "EMEC1C", "EMEC2A", "EMEC2C", "EMEC3A", "EMEC3C", 
	"FCAL1A", "FCAL1C", "FCAL2A", "FCAL2C", "FCAL3A", "FCAL3C"},
        "Names of individual layers to monitor"};


  //From: https://gitlab.cern.ch/atlas-lar-online/atlas-moncfg/-/blob/master/tools/LATOMEMonitoring.py#L82
  std::set<std::string> m_badSCs;

  std::map <std::string, std::pair<std::string, int> > m_LatomeDetBinMapping = {
    {"0x48",{"FCALC",1}},
    {"0x4c",{"EMEC/HECC",3}},
  {"0x44",{"EMECC",11}},
    {"0x4a",{"EMB/EMECC",27}},
    {"0x42",{"EMBC",43}},
    {"0x41",{"EMBA",59}},
    {"0x49",{"EMB/EMECA",75}},
    {"0x43",{"EMECA",91}},
    {"0x4b",{"EMEC/HECA",107}},
    {"0x47",{"FCALA",115}}
  };
  std::map <std::string, std::pair<std::string, int> > m_LatomeDetBinMappingPerSide = { 
    {"0x48",{"FCAL",57}},
    {"0x4c",{"EMEC_HEC",49}},
    {"0x44",{"EMEC",33}},
    {"0x4a",{"EMB_EMEC",17}},
    {"0x42",{"EMB",1}},
    {"0x41",{"EMB",1}},
    {"0x49",{"EMB_EMEC",17}},
    {"0x43",{"EMEC",33}},
    {"0x4b",{"EMEC_HEC",49}},
    {"0x47",{"FCAL",57}}
  };				     




};
#endif
