#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from libpyeformat_helper import SourceIdentifier, SubDetector

def eFexByteStreamToolCfg(flags, name, *, writeBS=False, TOBs=True, xTOBs=False, multiSlice=False, decodeInputs=False):
  acc = ComponentAccumulator()  

  tool = CompFactory.eFexByteStreamTool(name)

  if writeBS:
    # write BS == read xAOD
    # Note: this is currently unsupported!!!
    tool.eEMContainerReadKey   = "L1_eEMxRoI"  if xTOBs else "L1_eEMRoI"
    tool.eTAUContainerReadKey  = "L1_eTauxRoI" if xTOBs else "L1_eTauRoI"
    tool.eEMContainerWriteKey  = ""
    tool.eTAUContainerWriteKey = ""
  else:
    # read BS == write xAOD
    tool.eEMContainerReadKey   = ""
    tool.eTAUContainerReadKey  = ""
    if TOBs or xTOBs or multiSlice:
      efex_roi_moduleids = [0x1000,0x1100]
      tool.ROBIDs = [int(SourceIdentifier(SubDetector.TDAQ_CALO_FEAT_EXTRACT_ROI, moduleid)) for moduleid in efex_roi_moduleids]
    if TOBs:
      tool.eEMContainerWriteKey  = "L1_eEMRoI"
      tool.eTAUContainerWriteKey = "L1_eTauRoI"
    if xTOBs:
      tool.eEMxContainerWriteKey = "L1_eEMxRoI"
      tool.eTAUxContainerWriteKey = "L1_eTauxRoI"
    if multiSlice:
      tool.eEMSliceContainerWriteKey = "L1_eEMxRoIOutOfTime"
      tool.eTAUSliceContainerWriteKey = "L1_eTauxRoIOutOfTime"
    if decodeInputs:
      efex_raw_ids = []
      inputId = int(SourceIdentifier(SubDetector.TDAQ_CALO_FEAT_EXTRACT_DAQ, 0x1000))
      for shelf in range(0,2):
        for module in range(0,12):
          efex_raw_ids += [inputId + shelf*0x100 + module*0x010 ]
      tool.ROBIDs += efex_raw_ids
      tool.eTowerContainerWriteKey   = "L1_eFexDataTowers"

  if flags.Output.HISTFileName != '' or flags.Trigger.doHLT:
    if flags.Trigger.doHLT:
      from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
      monTool = GenericMonitoringTool(flags,'MonTool',HistPath = f'HLTFramework/L1BSConverters/{name}')
      topDir = "EXPERT"
    else:
      # if used in offline reconstruction respect DQ convention (ATR-26371)
      from AthenaMonitoring import AthMonitorCfgHelper
      helper = AthMonitorCfgHelper(flags, 'HLTFramework')
      monTool = helper.addGroup(None, f'{name}MonTool', f'/HLT/HLTFramework/L1BSConverters/{name}')
      topDir = None
      acc.merge(helper.result())
    monTool.defineHistogram('efexDecoderErrorTitle,efexDecoderErrorLocation;errors', path=topDir, type='TH2I',
                            title='Decoder Errors;Title;Location',
                            xbins=1, xmin=0, xmax=1, xlabels=["UNKNOWN"],
                            ybins=1, ymin=0, ymax=1, ylabels=["UNKNOWN"],
                            opt=['kCanRebin'])
    tool.MonTool = monTool

  acc.setPrivateTools(tool)
  return acc


def jFexRoiByteStreamToolCfg(flags, name, *, writeBS=False, xTOBs=False):
  acc = ComponentAccumulator()
  tool = CompFactory.jFexRoiByteStreamTool(name)
  tool.ConvertExtendedTOBs = xTOBs
  jfex_roi_moduleids = [0x2000]
  tool.ROBIDs = [int(SourceIdentifier(SubDetector.TDAQ_CALO_FEAT_EXTRACT_ROI, moduleid)) for moduleid in jfex_roi_moduleids]
  if writeBS:
    # write BS == read xAOD
    tool.jJRoIContainerReadKey   = "L1_jFexSRJetxRoI" if xTOBs else "L1_jFexSRJetRoI"
    tool.jLJRoIContainerReadKey  = "L1_jFexLRJetxRoI" if xTOBs else "L1_jFexLRJetRoI"
    tool.jTauRoIContainerReadKey = "L1_jFexTauxRoI"   if xTOBs else "L1_jFexTauRoI"
    tool.jEMRoIContainerReadKey  = "L1_jFexFwdElxRoI" if xTOBs else "L1_jFexFwdElRoI"
    tool.jTERoIContainerReadKey  = "L1_jFexSumETxRoI" if xTOBs else "L1_jFexSumETRoI"
    tool.jXERoIContainerReadKey  = "L1_jFexMETxRoI"   if xTOBs else "L1_jFexMETRoI"

    tool.jJRoIContainerWriteKey  =""
    tool.jLJRoIContainerWriteKey =""
    tool.jTauRoIContainerWriteKey=""
    tool.jEMRoIContainerWriteKey =""
    tool.jTERoIContainerWriteKey =""
    tool.jXERoIContainerWriteKey =""
  else:
    # read BS == write xAOD
    tool.jJRoIContainerReadKey   =""
    tool.jLJRoIContainerReadKey  =""
    tool.jTauRoIContainerReadKey =""
    tool.jEMRoIContainerReadKey  =""
    tool.jTERoIContainerReadKey  =""
    tool.jXERoIContainerReadKey  =""

    tool.jJRoIContainerWriteKey  = "L1_jFexSRJetxRoI" if xTOBs else "L1_jFexSRJetRoI"
    tool.jLJRoIContainerWriteKey = "L1_jFexLRJetxRoI" if xTOBs else "L1_jFexLRJetRoI"
    tool.jTauRoIContainerWriteKey= "L1_jFexTauxRoI"   if xTOBs else "L1_jFexTauRoI"
    tool.jEMRoIContainerWriteKey = "L1_jFexFwdElxRoI" if xTOBs else "L1_jFexFwdElRoI"
    tool.jTERoIContainerWriteKey = "L1_jFexSumETxRoI" if xTOBs else "L1_jFexSumETRoI"
    tool.jXERoIContainerWriteKey = "L1_jFexMETxRoI"   if xTOBs else "L1_jFexMETRoI"
    
  if flags.Output.HISTFileName != '' or flags.Trigger.doHLT:
    if flags.Trigger.doHLT:
      from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
      monTool = GenericMonitoringTool(flags,'MonTool',HistPath = f'HLTFramework/L1BSConverters/{name}')
      topDir = "EXPERT"
    else:
      # if used in offline reconstruction respect DQ convention (ATR-26371)
      from AthenaMonitoring import AthMonitorCfgHelper
      helper = AthMonitorCfgHelper(flags, 'HLTFramework')
      monTool = helper.addGroup(None, f'{name}MonTool', f'/HLT/HLTFramework/L1BSConverters/{name}')
      topDir = None
      acc.merge(helper.result())

    monTool.defineHistogram('jfexDecoderErrorTitle,jfexDecoderErrorLocation;errors', path=topDir, type='TH2I',
                            title='jFEX TOB Decoder Errors;Type;Location',
                            xbins=1, xmin=0, xmax=1, xlabels=["UNKNOWN"],
                            ybins=1, ymin=0, ymax=1, ylabels=["UNKNOWN"],
                            opt=['kCanRebin'])
    tool.MonTool = monTool

  acc.setPrivateTools(tool)
  return acc

 
def gFexByteStreamToolCfg(flags, name, *, writeBS=False):
  acc = ComponentAccumulator()
  tool = CompFactory.gFexByteStreamTool(name)
  gfex_roi_moduleids = [0x3000]
  tool.ROBIDs = [int(SourceIdentifier(SubDetector.TDAQ_CALO_FEAT_EXTRACT_ROI, moduleid)) for moduleid in gfex_roi_moduleids]
  if writeBS:
    # write BS == read xAOD
    tool.gFexRhoOutputContainerReadKey                  ="L1_gFexRhoRoI"
    tool.gFexSRJetOutputContainerReadKey                ="L1_gFexSRJetRoI"
    tool.gFexLRJetOutputContainerReadKey                ="L1_gFexLRJetRoI"
    tool.gScalarEJwojOutputContainerReadKey             ="L1_gScalarEJwoj"
    tool.gMETComponentsJwojOutputContainerReadKey       ="L1_gMETComponentsJwoj"
    tool.gMHTComponentsJwojOutputContainerReadKey       ="L1_gMHTComponentsJwoj"
    tool.gMSTComponentsJwojOutputContainerReadKey       ="L1_gMSTComponentsJwoj"
    tool.gMETComponentsNoiseCutOutputContainerReadKey   ="L1_gMETComponentsNoiseCut"
    tool.gMETComponentsRmsOutputContainerReadKey        ="L1_gMETComponentsRms"
    tool.gScalarENoiseCutOutputContainerReadKey         ="L1_gScalarENoiseCut"
    tool.gScalarERmsOutputContainerReadKey              ="L1_gScalarERms"
    
    
    tool.gFexRhoOutputContainerWriteKey                 =""
    tool.gFexSRJetOutputContainerWriteKey               =""
    tool.gFexLRJetOutputContainerWriteKey               =""
    tool.gScalarEJwojOutputContainerWriteKey            =""
    tool.gMETComponentsJwojOutputContainerWriteKey      =""
    tool.gMHTComponentsJwojOutputContainerWriteKey      =""
    tool.gMSTComponentsJwojOutputContainerWriteKey      =""
    tool.gMETComponentsNoiseCutOutputContainerWriteKey  =""
    tool.gMETComponentsRmsOutputContainerWriteKey       =""
    tool.gScalarENoiseCutOutputContainerWriteKey        =""
    tool.gScalarERmsOutputContainerWriteKey             =""
  else:
    # read BS == write xAOD
    tool.gFexRhoOutputContainerReadKey                  =""
    tool.gFexSRJetOutputContainerReadKey                =""
    tool.gFexLRJetOutputContainerReadKey                =""
    tool.gScalarEJwojOutputContainerReadKey             =""
    tool.gMETComponentsJwojOutputContainerReadKey       =""
    tool.gMHTComponentsJwojOutputContainerReadKey       =""
    tool.gMSTComponentsJwojOutputContainerReadKey       =""
    tool.gMETComponentsNoiseCutOutputContainerReadKey   =""
    tool.gMETComponentsRmsOutputContainerReadKey        =""
    tool.gScalarENoiseCutOutputContainerReadKey         =""
    tool.gScalarERmsOutputContainerReadKey              =""
    
    
    tool.gFexRhoOutputContainerWriteKey                 ="L1_gFexRhoRoI"
    tool.gFexSRJetOutputContainerWriteKey               ="L1_gFexSRJetRoI"
    tool.gFexLRJetOutputContainerWriteKey               ="L1_gFexLRJetRoI"
    tool.gScalarEJwojOutputContainerWriteKey            ="L1_gScalarEJwoj"
    tool.gMETComponentsJwojOutputContainerWriteKey      ="L1_gMETComponentsJwoj"
    tool.gMHTComponentsJwojOutputContainerWriteKey      ="L1_gMHTComponentsJwoj"
    tool.gMSTComponentsJwojOutputContainerWriteKey      ="L1_gMSTComponentsJwoj"
    tool.gMETComponentsNoiseCutOutputContainerWriteKey  ="L1_gMETComponentsNoiseCut"
    tool.gMETComponentsRmsOutputContainerWriteKey       ="L1_gMETComponentsRms"
    tool.gScalarENoiseCutOutputContainerWriteKey        ="L1_gScalarENoiseCut"
    tool.gScalarERmsOutputContainerWriteKey             ="L1_gScalarERms"


  if flags.Output.HISTFileName != '' or flags.Trigger.doHLT:
    if flags.Trigger.doHLT:
      from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
      monTool = GenericMonitoringTool(flags,'MonTool',HistPath = f'HLTFramework/L1BSConverters/{name}')
      topDir = "EXPERT"
    else:
      # if used in offline reconstruction respect DQ convention (ATR-26371)
      from AthenaMonitoring import AthMonitorCfgHelper
      helper = AthMonitorCfgHelper(flags, 'HLTFramework')
      monTool = helper.addGroup(None, f'{name}MonTool', f'/HLT/HLTFramework/L1BSConverters/{name}')
      topDir = None
      acc.merge(helper.result())

    monTool.defineHistogram('gfexDecoderErrorTitle,gfexDecoderErrorLocation;errors', path=topDir, type='TH2I',
                            title='gFEX TOB Decoder Errors;Type;Location',
                            xbins=1, xmin=0, xmax=1, xlabels=["UNKNOWN"],
                            ybins=1, ymin=0, ymax=1, ylabels=["UNKNOWN"],
                            opt=['kCanRebin'])
    tool.MonTool = monTool    


  acc.setPrivateTools(tool)
  return acc


def jFexInputByteStreamToolCfg(flags, name, *, writeBS=False):
  acc = ComponentAccumulator()
  tool = CompFactory.jFexInputByteStreamTool(name)
  jfex_roi_moduleids = [0x2000,0x2010,0x2020,0x2030,0x2040,0x2050]
  tool.ROBIDs = [int(SourceIdentifier(SubDetector.TDAQ_CALO_FEAT_EXTRACT_DAQ, moduleid)) for moduleid in jfex_roi_moduleids]  
  
  #will be needed in the future for jTower container, still not coded
  if writeBS:
    # write BS == read xAOD
    tool.jTowersReadKey   = "L1_jFexDataTowers" 

    tool.jTowersWriteKey  =""
  else:
    # read BS == write xAOD
    tool.jTowersReadKey   =""

    tool.jTowersWriteKey  = "L1_jFexDataTowers"

  if flags.Output.HISTFileName != '' or flags.Trigger.doHLT:
    if flags.Trigger.doHLT:
      from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
      monTool = GenericMonitoringTool(flags,'MonTool',HistPath = f'HLTFramework/L1BSConverters/{name}')
      topDir = "EXPERT"
    else:
      # if used in offline reconstruction respect DQ convention (ATR-26371)
      from AthenaMonitoring import AthMonitorCfgHelper
      helper = AthMonitorCfgHelper(flags, 'HLTFramework')
      monTool = helper.addGroup(None, f'{name}MonTool', f'/HLT/HLTFramework/L1BSConverters/{name}')
      topDir = None
      acc.merge(helper.result())

    monTool.defineHistogram('jfexDecoderErrorTitle,jfexDecoderErrorLocation;errors', path=topDir, type='TH2I',
                            title='jFEX InputData Decoder Errors;Type;Location',
                            xbins=1, xmin=0, xmax=1, xlabels=["UNKNOWN"],
                            ybins=1, ymin=0, ymax=1, ylabels=["UNKNOWN"],
                            opt=['kCanRebin'])
    tool.MonTool = monTool    


  acc.setPrivateTools(tool)
  return acc


def gFexInputByteStreamToolCfg(flags, name, *, writeBS=False):
  acc = ComponentAccumulator()
  tool = CompFactory.gFexInputByteStreamTool(name)
  gfex_roi_moduleids = [0x3000]
  tool.ROBIDs = [int(SourceIdentifier(SubDetector.TDAQ_CALO_FEAT_EXTRACT_DAQ, moduleid)) for moduleid in gfex_roi_moduleids]  
  print ("[L1CaloFEXByteStreamConfig::gFexInputByteStreamToolCfg]  tool.ROBIDs   ", tool.ROBIDs)

  if writeBS:
    # write BS == read xAOD
    tool.gTowersReadKey   = "L1_gFexDataTowers" 

    tool.gTowersWriteKey  =""
  else:
    # read BS == write xAOD
    tool.gTowersReadKey   =""
  
    tool.gTowersWriteKey  = "L1_gFexDataTowers"
    
  if flags.Output.HISTFileName != '' or flags.Trigger.doHLT:
    if flags.Trigger.doHLT:
      from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
      monTool = GenericMonitoringTool(flags,'MonTool',HistPath = f'HLTFramework/L1BSConverters/{name}')
      topDir = "EXPERT"
    else:
      # if used in offline reconstruction respect DQ convention (ATR-26371)
      from AthenaMonitoring import AthMonitorCfgHelper
      helper = AthMonitorCfgHelper(flags, 'HLTFramework')
      monTool = helper.addGroup(None, f'{name}MonTool', f'/HLT/HLTFramework/L1BSConverters/{name}')
      topDir = None
      acc.merge(helper.result())

    monTool.defineHistogram('gfexDecoderErrorTitle,gfexDecoderErrorLocation;errors', path=topDir, type='TH2I',
                            title='gFEX InputData Decoder Errors;Type;Location',
                            xbins=1, xmin=0, xmax=1, xlabels=["UNKNOWN"],
                            ybins=1, ymin=0, ymax=1, ylabels=["UNKNOWN"],
                            opt=['kCanRebin'])
    tool.MonTool = monTool   

  acc.setPrivateTools(tool)
  return acc
