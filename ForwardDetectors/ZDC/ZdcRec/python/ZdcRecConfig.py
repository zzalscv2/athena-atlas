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

from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg

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


def ZdcAnalysisToolCfg(flags, run, config="PbPb2015", DoCalib=False, DoTimeCalib=False, DoTrigEff=False):
    acc = ComponentAccumulator()

    acc.setPrivateTools(CompFactory.ZDC.ZdcAnalysisTool(
        Configuration = config,
        DoCalib = DoCalib,
        DoTimeCalib = DoTimeCalib,
        DoTrigEff = DoTrigEff, 
        LHCRun = run ))
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

    acc.addEventAlgo(CompFactory.ZdcByteStreamLucrodData())
    anaTool = acc.popToolsAndMerge(ZdcAnalysisToolCfg(flags,3,config,doCalib,doTimeCalib,doTrigEff))
    acc.addEventAlgo(CompFactory.ZdcRecRun3("ZdcRecRun3",ZdcAnalysisTool=anaTool))

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
            acc.merge(ZdcRecRun2Cfg(flags))
        elif (run == LHCPeriod.Run3):
            acc.merge(ZdcRecRun3Cfg(flags))
        
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

    flags.Detector.GeometryZDC=True
    flags.Detector.GeometryAFP=False
    flags.Detector.GeometryALFA=False
    flags.Detector.GeometryLucid=False
    flags.Detector.GeometryMDT=False
    flags.Detector.GeometryMM=False
    flags.Detector.GeometryMuon=False
    flags.Trigger.DecodeHLT=False
    flags.Trigger.enableL1MuonPhase1=False
    flags.Trigger.L1.doMuon=False
    flags.Trigger.L1.doCalo=False
    flags.Trigger.L1.doTopo=False

    # This does not work in this context
    # run = flags.GeoModel.Run
    # The EDM Version should be auto configured, but is not working at the moment, so is set by hand

    year = 22 # unfortunately not yet able to autoconfigure, since project name not available.

    if (year < 20):
        flags.Trigger.EDMVersion=2
        flags.GeoModel.Run = LHCPeriod.Run2
    elif (year > 20):
        flags.Trigger.EDMVersion=3
        flags.GeoModel.Run = LHCPeriod.Run3

    flags.Output.AODFileName="calibAOD.pool.root"
    flags.Output.doWriteAOD=True
    flags.fillFromArgs()

    flags.lock()


    acc=MainServicesCfg(flags)

    
    from TriggerJobOpts.TriggerRecoConfig import TriggerRecoCfgData
    acc.merge(TriggerRecoCfgData(flags))

    from AtlasGeoModel.ForDetGeoModelConfig import ForDetGeometryCfg
    acc.merge(ForDetGeometryCfg(flags))

    acc.merge(ZdcRecCfg(flags))

    acc.printConfig(withDetails=True)

    with open("config.pkl", "wb") as f:
        acc.store(f)
    status = acc.run()
    if status.isFailure():
        import sys
        sys.exit(-1)

