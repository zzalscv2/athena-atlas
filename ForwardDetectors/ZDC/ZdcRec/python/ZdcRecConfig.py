#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import Format
from AthenaConfiguration.Enums import LHCPeriod

from GeoModelSvc.GeoModelSvcConf import GeoModelSvc

from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
from OutputStreamAthenaPool.OutputStreamConfig import addToESD

from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg

from TrigConfigSvc.TriggerConfigAccess import getL1MenuAccess
from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg

def ZdcRecOutputCfg(flags):

    acc = ComponentAccumulator()

    ZDC_ItemList=[]
    if flags.Input.Format is Format.BS:
        ZDC_ItemList.append("xAOD::ZdcModuleContainer#ZdcModules")
        ZDC_ItemList.append("xAOD::ZdcModuleAuxContainer#ZdcModulesAux.")
        ZDC_ItemList.append("xAOD::ZdcModuleContainer#ZdcSums")
        ZDC_ItemList.append("xAOD::ZdcModuleAuxContainer#ZdcSumsAux.")

    acc.merge(addToESD(flags,ZDC_ItemList))
    acc.merge(addToAOD(flags,ZDC_ItemList))

    return acc


def ZdcAnalysisToolCfg(flags, run, config="LHCf2022", DoCalib=False, DoTimeCalib=False, DoTrigEff=False):
    acc = ComponentAccumulator()

    print('ZdcAnalysisToolCfg: setting up ZdcAnalysisTool with config='+config)

    acc.setPrivateTools(CompFactory.ZDC.ZdcAnalysisTool(
        name = 'ZdcAnalysisTool'+config, 
        Configuration = config,
        DoCalib = DoCalib,
        DoTimeCalib = DoTimeCalib,
        DoTrigEff = DoTrigEff, 
        LHCRun = run ))
    return acc

def ZdcLEDAnalysisToolCfg(flags, config = 'PbPb2023'):
    acc = ComponentAccumulator()

    print('ZdcAnalysisToolCfg: setting up ZdcAnalysisTool with config='+config)
    acc.setPrivateTools(CompFactory.ZDC.ZdcLEDAnalysisTool(name = 'ZdcLEDAnalysisTool'+config, 
                                                           Configuration = config))
    return acc


def ZdcTrigValToolCfg(flags, config = 'LHCf2022'):
    acc = ComponentAccumulator()
    
    acc.merge(TrigDecisionToolCfg(flags))
    
    trigValTool = CompFactory.ZDC.ZdcTrigValidTool(
        name = 'ZdcTrigValTool',
        WriteAux = True,
        AuxSuffix = "",
        filepath_LUT = "TrigT1ZDC/zdcRun3T1LUT_v1_30_05_2023.json")
        
    trigValTool.TrigDecisionTool = acc.getPublicTool('TrigDecisionTool')
    
    trigValTool.triggerList = [c for c in getL1MenuAccess(flags) if 'L1_ZDC_BIT' in c]
    
    acc.setPrivateTools(trigValTool)
      
    return acc

def ZdcRecRun2Cfg(flags):        
    acc = ComponentAccumulator()
    config = "default"
    doCalib = False
    doTimeCalib = False
    doTrigEff = False

    if flags.Input.ProjectName == "data15_hi":
        config = "PbPb2015"
        doCalib = True
        doTimeCalib = True
        doTrigEff = True
    elif flags.Input.ProjectName == "data17_13TeV":
        config = "PbPb2015"
        doCalib = False
        doTimeCalib = False
        doTrigEff = False
    elif flags.Input.ProjectName == "data16_hip":
        config = "pPb2016"
        doCalib = True
        doTimeCalib = False
        doTrigEff = False
    elif flags.Input.ProjectName == "data18_hi":
        config = "PbPb2018"
        doCalib = True
        doTimeCalib = False
        doTrigEff = False

    acc.merge(ByteStreamReadCfg(flags, type_names=['xAOD::TriggerTowerContainer/ZdcTriggerTowers',
                                         'xAOD::TriggerTowerAuxContainer/ZdcTriggerTowersAux.']))

    acc.addEventAlgo(CompFactory.ZdcByteStreamRawDataV2())
    acc.addEventAlgo(CompFactory.ZdcRecV3Decode())

    anaTool = acc.popToolsAndMerge(ZdcAnalysisToolCfg(flags,2,config,doCalib,doTimeCalib,doTrigEff))

    acc.addEventAlgo(CompFactory.ZdcRecV3("ZdcRecV3",ZdcAnalysisTool=anaTool))

    return acc

def ZdcRecRun3Cfg(flags):

    acc = ComponentAccumulator()
    config = "LHCf2022"
    doCalib = False
    doTimeCalib = False
    doTrigEff = False
    
    if flags.Input.ProjectName == "data22_13p6TeV":
        config = "LHCf2022"
    elif flags.Input.ProjectName == "data23_hi":
        config = "PbPb2023"
    elif flags.Input.ProjectName == "data22_hi":
        config = "PbPb2023"

    acc.addEventAlgo(CompFactory.ZdcByteStreamLucrodData())
    acc.addEventAlgo(CompFactory.ZdcRecRun3Decode())
    
    anaTool = acc.popToolsAndMerge(ZdcAnalysisToolCfg(flags,3,config,doCalib,doTimeCalib,doTrigEff))
    trigTool = acc.popToolsAndMerge(ZdcTrigValToolCfg(flags,config))    
    zdcTools = [anaTool,trigTool] # expand list as needed
    #zdcTools = [anaTool] # add trigTool after deocration migration
    
    zdcAlg = CompFactory.ZdcRecRun3("ZdcRecRun3",ZdcAnalysisTools=zdcTools)
    acc.addEventAlgo(zdcAlg, primary=True)

    return acc
    
def ZdcLEDRecCfg(flags):

    acc = ComponentAccumulator()
    
    if flags.Input.Format is Format.BS:
        run = flags.GeoModel.Run
        
        # debugging message since the metadata isn't working for calibration files yet
        print ("ZdcRecConfig.py: run = "+run.name)
        
        config = 'ppPbPb2023'

        acc.addEventAlgo(CompFactory.ZdcByteStreamLucrodData())
        acc.addEventAlgo(CompFactory.ZdcRecRun3Decode())
    
        anaTool = acc.popToolsAndMerge(ZdcLEDAnalysisToolCfg(flags, config)) #anatool for zdcLED calibration  
    
        zdcTools = []
        zdcTools += [anaTool] # add trigTool after deocration migration
    
        zdcAlg = CompFactory.ZdcRecRun3("ZdcRecRun3",ZdcAnalysisTools=zdcTools)
        acc.addEventAlgo(zdcAlg, primary=True)

    if flags.Output.doWriteESD or flags.Output.doWriteAOD:
        acc.merge(ZdcRecOutputCfg(flags))
        
    return acc

def ZdcRecCfg(flags):    
    """Configure Zdc analysis alg
    Additional arguments are useful in calibration runs
    """

    acc = ComponentAccumulator()
 
    if flags.Input.Format is Format.BS:
        run = flags.GeoModel.Run

        # debugging message since the metadata isn't working for calibration files yet
        print ("ZdcRecConfig.py: run = "+run.name)

        if (run == LHCPeriod.Run2):
            print ('ZdcRecConfig.py: setting up Run 2!')
            acc.merge(ZdcRecRun2Cfg(flags))
        elif (run == LHCPeriod.Run3):
            print ('ZdcRecConfig.py: setting up Run 3!')
            acc.merge(ZdcRecRun3Cfg(flags))
        else:
            print ('ZdcRecConfig.py: setting up nothing (problem)!')

    if flags.Output.doWriteESD or flags.Output.doWriteAOD:
        acc.merge(ZdcRecOutputCfg(flags))

    return acc

if __name__ == '__main__':

    """ This is selftest & Zdc analysis transform at the same time"""
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    # This appears to be needed for calibration data (standalone), but is not working at present
    GeoModelSvc.AtlasVersion = "ATLAS-R3S-2021-03-00-00"

    flags = initConfigFlags()
    flags.Scheduler.CheckDependencies = True
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.ShowDataFlow = True
    flags.Scheduler.ShowControlFlow = True
    flags.Scheduler.EnableVerboseViews = True

    flags.Detector.GeometryZDC=True
    flags.Detector.GeometryAFP=False
    flags.Detector.GeometryALFA=False
    flags.Detector.GeometryLucid=False
    flags.Detector.GeometryMDT=False
    flags.Detector.GeometryMM=False
    flags.Detector.GeometryMuon=False
    flags.Trigger.decodeHLT=False
    flags.Trigger.enableL1MuonPhase1=False
    flags.Trigger.L1.doMuon=False
    flags.Trigger.L1.doCalo=False
    flags.Trigger.L1.doTopo=False
    #flags.Reco.EnableTrigger = False

    # This does not work in this context
    # run = flags.GeoModel.Run
    # The EDM Version should be auto configured, but is not working at the moment, so is set by hand

    flags.Output.AODFileName="calibAOD.pool.root"
    flags.Output.doWriteAOD=True

    flags.fillFromArgs()
    
    isLED = False
    
    pn = flags.Input.ProjectName
    
    if not isLED:
        year = int(pn.split('_')[0].split('data')[1])
        if (year < 20):
            flags.Trigger.EDMVersion=2
            flags.GeoModel.Run = LHCPeriod.Run2
        elif (year > 20):
            flags.Trigger.EDMVersion=3
            flags.GeoModel.Run = LHCPeriod.Run3
    else:
        flags.Trigger.EDMVersion=3
        flags.GeoModel.Run = LHCPeriod.Run3

    flags.lock()


    acc=MainServicesCfg(flags)

    if not isLED:
        from TriggerJobOpts.TriggerRecoConfig import TriggerRecoCfgData
        acc.merge(TriggerRecoCfgData(flags))

    
    from AtlasGeoModel.ForDetGeoModelConfig import ForDetGeometryCfg
    acc.merge(ForDetGeometryCfg(flags))

    if not isLED: 
        acc.merge(ZdcRecCfg(flags))
    else:
        acc.merge(ZdcLEDRecCfg(flags))
        
    acc.printConfig(withDetails=True)

    with open("config.pkl", "wb") as f:
        acc.store(f)
    status = acc.run()
    if status.isFailure():
        import sys
        sys.exit(-1)

