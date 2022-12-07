# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

import sys
import os

from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from LArCalibProcessing.LArSC2NtupleConfig import LArSC2NtupleCfg
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

def L1CaloMenuCfg(flags):


    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

    acc=ComponentAccumulator()
    from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg, HLTConfigSvcCfg, L1PrescaleCondAlgCfg, HLTPrescaleCondAlgCfg
    from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import L1TriggerByteStreamDecoderCfg

    acc.merge( L1TriggerByteStreamDecoderCfg(flags) )
    acc.merge( L1ConfigSvcCfg(flags) )
    acc.merge( HLTConfigSvcCfg(flags) )
    acc.merge( L1PrescaleCondAlgCfg(flags) )
    acc.merge( HLTPrescaleCondAlgCfg(flags) )


    from TrigConfigSvc.TrigConfigSvcCfg import BunchGroupCondAlgCfg
    acc.merge( BunchGroupCondAlgCfg( flags ) )

    from AthenaConfiguration.ComponentFactory import CompFactory
    tdm = CompFactory.getComp('TrigDec::TrigDecisionMakerMT')()
    tdm.doL1 = True
    tdm.doHLT = False
    acc.addEventAlgo( tdm, 'AthAlgSeq' )

    return acc


def fromRunArgs(runArgs):

    from AthenaCommon.Logging import logging
    mlog_SCD = logging.getLogger( 'LArSCDumpSkeleton' )

    from AthenaConfiguration.AllConfigFlags import ConfigFlags    

    from LArCafJobs.LArSCDumperFlags import addSCDumpFlags
    addSCDumpFlags(ConfigFlags)

    commonRunArgsToFlags(runArgs, ConfigFlags)

    processPreInclude(runArgs, ConfigFlags)
    processPreExec(runArgs, ConfigFlags)

    ConfigFlags.Input.Files=runArgs.inputBSFile
    ConfigFlags.LArSCDump.outputNtup=runArgs.outputNTUP_SCMONFile

    # real geom not working yet
    ConfigFlags.LArSCDump.doGeom=False

    from LArConditionsCommon.LArRunFormat import getLArDTInfoForRun
    try:
       runinfo=getLArDTInfoForRun(ConfigFlags.Input.RunNumber[0], connstring="COOLONL_LAR/CONDBR2")
    except Exception:
       mlog_SCD.warning("Could not get DT run info, using defaults !")   
       ConfigFlags.LArSCDump.doEt=True
       ConfigFlags.LArSCDump.nSamples=5
       ConfigFlags.LArSCDump.nEt=1
       CKeys=["SC_ET"]    
    else:   
       CKeys=[]
       ConfigFlags.LArSCDump.digitsKey=""
       for i in range(0,len(runinfo.streamTypes())):
          if runinfo.streamTypes()[i] ==  "SelectedEnergy":
                CKeys += ["SC_ET_ID"]
                ConfigFlags.LArSCDump.doEt=True
                ConfigFlags.LArSCDump.nEt=runinfo.streamLengths()[i]
          elif runinfo.streamTypes()[i] ==  "Energy":
                CKeys += ["SC_ET"]
                ConfigFlags.LArSCDump.doEt=True
                ConfigFlags.LArSCDump.nEt=runinfo.streamLengths()[i]
          elif runinfo.streamTypes()[i] ==  "RawADC":
                ConfigFlags.LArSCDump.digitsKey="SC"
                ConfigFlags.LArSCDump.nSamples=runinfo.streamLengths()[i]
          elif runinfo.streamTypes()[i] ==  "ADC":
                CKeys += ["SC_ADC_BAS"]
                ConfigFlags.LArSCDump.nSamples=runinfo.streamLengths()[i]
                
    finally:
       ConfigFlags.LArSCDump.doRawChan=True
       ConfigFlags.LArSCDump.fillNoisyRO=False
       CKeys+=["LArRawChannels"]

    mlog_SCD.debug("CKeys generated %s",str(CKeys))   

    ConfigFlags.Trigger.triggerConfig = 'DB'
    ConfigFlags.Trigger.L1.doCTP = True
    ConfigFlags.Trigger.L1.doMuon = False
    ConfigFlags.Trigger.L1.doCalo = False
    ConfigFlags.Trigger.L1.doTopo = False

    ConfigFlags.Trigger.enableL1CaloLegacy = True
    ConfigFlags.Trigger.enableL1CaloPhase1 = True

    ConfigFlags.lock()
    
    cfg=MainServicesCfg(ConfigFlags)
    cfg.merge(L1CaloMenuCfg(ConfigFlags))

    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    tdt = cfg.getPrimaryAndMerge(TrigDecisionToolCfg(ConfigFlags))

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    cfg.merge(LArGMCfg(ConfigFlags))

    if ConfigFlags.LArSCDump.doBC:
       # FIXME should be SC version
       from LArBadChannelTool.LArBadChannelConfig import  LArBadFebCfg, LArBadChannelCfg
       cfg.merge(LArBadChannelCfg(ConfigFlags))
       cfg.merge(LArBadFebCfg(ConfigFlags))

    cfg.merge(LArSC2NtupleCfg(ConfigFlags, AddBadChannelInfo=ConfigFlags.LArSCDump.doBC, AddFEBTempInfo=False, isSC=True, isFlat=False,
                            OffId=ConfigFlags.LArSCDump.doOfflineId, AddHash=ConfigFlags.LArSCDump.doHash, AddCalib=ConfigFlags.LArSCDump.doCalib, RealGeometry=ConfigFlags.LArSCDump.doGeom, ExpandId=ConfigFlags.LArSCDump.expandId, # from LArCond2NtupleBase 
                            NSamples=ConfigFlags.LArSCDump.nSamples, FTlist={}, FillBCID=ConfigFlags.LArSCDump.doBCID, ContainerKey=ConfigFlags.LArSCDump.digitsKey,  # from LArDigits2Ntuple
                            SCContainerKeys=CKeys, OverwriteEventNumber = ConfigFlags.LArSCDump.overwriteEvN, Net=ConfigFlags.LArSCDump.nEt, # from LArSC2Ntuple
                            FillRODEnergy = ConfigFlags.LArSCDump.doRawChan, FillLB = True, FillTriggerType = True,
                            TrigNames=["L1_EM3","L1_EM7","L1_EM15","L1_EM22VHI","L1_eEM5","L1_eEM15","L1_eEM22M"],
                            TrigDecisionTool=tdt,
                            OutputLevel=3))

    if os.path.exists(ConfigFlags.LArSCDump.outputNtup):
          os.remove(ConfigFlags.LArSCDump.outputNtup)
    from AthenaConfiguration.ComponentFactory import CompFactory
    cfg.addService(CompFactory.NTupleSvc(Output = [ "FILE1 DATAFILE='"+ConfigFlags.LArSCDump.outputNtup+"' OPT='NEW'" ]))
    cfg.setAppProperty("HistogramPersistency","ROOT")


    processPostInclude(runArgs, ConfigFlags, cfg)
    processPostExec(runArgs, ConfigFlags, cfg)

    #example how to dump the stores
    #cfg.getService("StoreGateSvc").Dump=True
    #from AthenaCommon.Constants import DEBUG
    #cfg.getService("MessageSvc").OutputLevel=DEBUG
    # Run the final accumulator
    sc = cfg.run()
    sys.exit(not sc.isSuccess())
