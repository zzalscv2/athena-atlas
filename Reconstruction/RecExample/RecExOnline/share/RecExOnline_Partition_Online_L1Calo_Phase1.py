# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

## Job options for Running the L1Calo Athena Online Monitoring for Phase1
## run through athena
##    offline: athena --CA RecExOnline/RecExOnline_Partition_Online_L1Calo_Phase1.py --filesInput path/to/raw.data --evtMax 10
##    online:  athena --CA RecExOnline/RecExOnline_Partition_Online_L1Calo_Phase1.py
## Author: Will Buttinger

from AthenaCommon.Configurable import Configurable,ConfigurableCABehavior
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator,appendCAtoAthena
from AthenaConfiguration.ComponentFactory import CompFactory,isComponentAccumulatorCfg
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.Enums import LHCPeriod
from AthenaCommon import Constants
import os
import ispy
partition = ispy.IPCPartition(os.getenv("TDAQ_PARTITION","ATLAS"))

if partition.isValid():
  print("Running Online with Partition:",partition.name())
else:
  print("Partition",partition.name()," not found. Running Offline - must provide input files!")

ConfigFlags = initConfigFlags()
ConfigFlags.Input.Files = [] # so that when no files given we can detect that

# Note: The order in which all these flag defaults get set is very fragile
# so don't reorder the setup of this ConfigFlags stuff

if not isComponentAccumulatorCfg():
  # running as a jobo
  # from AthenaCommon.DetFlags import DetFlags
  # DetFlags.detdescr.all_setOff()
  # DetFlags.detdescr.Calo_setOn()
  decodeInputs = True
  # next three lines so that conddb set up ok
  if len(jps.AthenaCommonFlags.FilesInput())>0:
    from RecExConfig import AutoConfiguration
    AutoConfiguration.ConfigureSimulationOrRealData() #sets globalflags.DataSource
    AutoConfiguration.ConfigureFromListOfKeys(['ProjectName']) #sets rec.projectName, necessary to infer DatabaseInstance if that is left to 'auto' (default value)
    from AthenaConfiguration.OldFlags2NewFlags import getNewConfigFlags
    ConfigFlags = getNewConfigFlags() # replace flags with old flags
    ConfigFlags.Exec.MaxEvents = jps.AthenaCommonFlags.EvtMax() # didn't get copied so do it manually now
  elif partition.isValid():
    # running online, need to set some flags for conddb to be setup correctly at this point
    jps.Global.DataSource = 'data'
    jps.AthenaCommonFlags.isOnline = True

ConfigFlags.Trigger.triggerConfig='DB' #for L1menu if running offline
## Modify default flags
ConfigFlags.GeoModel.Run = LHCPeriod.Run3 # needed for LArGMConfig - or can infer from above
ConfigFlags.Common.useOnlineLumi = True # needed for lumi-scaled monitoring, only have lumi in online DB at this time
ConfigFlags.DQ.enableLumiAccess = False # in fact, we don't need lumi access for now ... this terms it all off
ConfigFlags.DQ.FileKey = "" if partition.isValid() else "EXPERT" # histsvc file "name" to record to - Rafal asked it to be blank @ P1 ... means monitoring.root will be empty
ConfigFlags.Output.HISTFileName = "monitoring.root" # control names of monitoring root file
ConfigFlags.DQ.useTrigger = False # don't do TrigDecisionTool in MonitorCfg helper methods
# flags for rerunning simulation
ConfigFlags.Trigger.L1.doeFex = True
ConfigFlags.Trigger.L1.dojFex = False
ConfigFlags.Trigger.L1.dogFex = False
# if running online, override these with autoconfig values
# will set things like the GlobalTag automatically
if partition.isValid():
  from AthenaConfiguration.AutoConfigOnlineRecoFlags import autoConfigOnlineRecoFlags
  autoConfigOnlineRecoFlags(ConfigFlags, partition.name()) # sets things like projectName etc which would otherwise be inferred from input file
ConfigFlags.IOVDb.GlobalTag = "CONDBR2-ES1PA-2022-07" #"CONDBR2-HLTP-2022-02"
# now parse
if isComponentAccumulatorCfg():
  parser = ConfigFlags.getArgumentParser()
  parser.add_argument('--runNumber',default=None,help="specify to select a run number")
  parser.add_argument('--lumiBlock',default=None,help="specify to select a lumiBlock")
  parser.add_argument('--stream',default="physics_L1Calo",help="stream to lookup files in")
  parser.add_argument('--decodeInputs',default=True,help="enable/disable input decoding+monitoring")
  parser.add_argument('--fexReadoutFilter',action='store_true',help="If specified, will skip events without fexReadout")
  args = ConfigFlags.fillFromArgs(parser=parser)
  decodeInputs = args.decodeInputs
  if args.runNumber is not None:
    from glob import glob
    if args.lumiBlock is None: args.lumiBlock="*"
    print("Looking up files in atlastier0 for run",args.runNumber,"lb =",args.lumiBlock)
    ConfigFlags.Input.Files = []
    for lb in args.lumiBlock.split(","):
      print("Trying",f"/eos/atlas/atlastier0/rucio/data*/{args.stream}/*{args.runNumber}/*/*lb{int(lb):04}.*")
      ConfigFlags.Input.Files += glob(f"/eos/atlas/atlastier0/rucio/data*/{args.stream}/*{args.runNumber}/*/*lb{int(lb):04}.*")
    print("Found",len(ConfigFlags.Input.Files),"files")

# require at least 1 input file if running offline
if not partition.isValid() and len(ConfigFlags.Input.Files)==0:
  print("FATAL: Running in offline mode but no input files provided")
  import sys
  sys.exit(1)

# add detector conditions flags required for rerunning simulation
# needs input files declared if offline, hence doing after parsing
from AthenaConfiguration.DetectorConfigFlags import setupDetectorsFromList
setupDetectorsFromList(ConfigFlags,['LAr','Tile','MBTS'],True)


if isComponentAccumulatorCfg():
  from AthenaConfiguration.MainServicesConfig import MainServicesCfg
  acc = MainServicesCfg(ConfigFlags)
else:
  # for an unknown reason it seems like its necessary to create
  # setup the IOVDbSvc now otherwise conditions aren't done properly - its like CA config isn't getting applied properly?
  # and the CALIBRATIONS folder isnt found
  from IOVDbSvc.CondDB import conddb
  with ConfigurableCABehavior():
    from AthenaConfiguration.MainServicesConfig import MessageSvcCfg
    acc = MessageSvcCfg(ConfigFlags) # start with just messageSvc ComponentAccumulator() # use empty accumulator

with ConfigurableCABehavior(): # need to temporarily activate run3 behaviour to use this configuration method

  ConfigFlags.lock()
  if ConfigFlags.Exec.MaxEvents == 0: ConfigFlags.dump(evaluate=True)

  if partition.isValid():
    from ByteStreamEmonSvc.EmonByteStreamConfig import EmonByteStreamCfg
    acc.merge(EmonByteStreamCfg(ConfigFlags)) # setup EmonSvc
    bsSvc = acc.getService("ByteStreamInputSvc")
    bsSvc.Partition = partition.name()
    bsSvc.Key = os.environ.get("L1CALO_PTIO_KEY", "REB" if partition.name()=="L1CaloStandalone" else "dcm") # set the Sampler Key Type name (default is SFI)
    if partition.name()=="L1CaloSTF": bsSvc.Key = "SWROD"
    bsSvc.KeyCount = int(os.environ.get("L1CALO_PTIO_KEY_COUNT","25"))
    bsSvc.ISServer = "Histogramming" # IS server on which to create this provider
    bsSvc.BufferSize = 10 # event buffer size for each sampler
    bsSvc.UpdatePeriod = 30 # time in seconds between updating plots
    bsSvc.Timeout = 240000 # timeout (not sure what this does)
    bsSvc.PublishName = os.getenv("L1CALO_ATHENA_JOB_NAME","testing") # set name of this publisher as it will appear in IS (default is "l1calo-athenaHLT"; change to something sensible for testing)
    bsSvc.StreamType = os.getenv("L1CALO_PTIO_STREAM_TYPE","physics") # name of the stream type (physics,express, etc.)
    bsSvc.ExitOnPartitionShutdown = False
    bsSvc.ClearHistograms = True # clear hists at start of new run
    bsSvc.GroupName = "RecExOnline"
    # name of the stream (Egamma,JetTauEtmiss,MinBias,Standby, etc.), this can be a colon(:) separated list of streams that use the 'streamLogic' to combine stream for 2016 HI run
    bsSvc.StreamNames = os.getenv("L1CALO_PTIO_STREAM_NAME","L1Calo:Main:MinBias:MinBiasOverlay:UPC:EnhancedBias:ZeroBias:HardProbes:Standby:ALFACalib").split(":")
    bsSvc.StreamLogic = os.getenv("L1CALO_PTIO_STREAM_LOGIC","Or") if partition.name() != "L1CaloStandalone" else "Ignore"
    bsSvc.LVL1Names = [] # name of L1 items to select
    bsSvc.LVL1Logic = "Ignore" # one of: Ignore, Or, And
  else:
    print("Running Offline on", len(ConfigFlags.Input.Files),"files")
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    acc.merge(ByteStreamReadCfg(ConfigFlags)) # configure reading bytestream

  # ensure histsvc is set up
  from AthenaMonitoring.AthMonitorCfgHelper import getDQTHistSvc
  acc.merge(getDQTHistSvc(ConfigFlags))

  # Create run3 L1 menu (needed for L1Calo EDMs)
  from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg
  acc.merge(L1ConfigSvcCfg(ConfigFlags))

  # -------- CHANGES GO BELOW ------------
  # setup the L1Calo software we want to monitor

  decoderTools = []

  from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import eFexByteStreamToolCfg, jFexRoiByteStreamToolCfg, jFexInputByteStreamToolCfg, gFexByteStreamToolCfg, gFexInputByteStreamToolCfg
  from TrigT1ResultByteStream.TrigT1ResultByteStreamMonitoring import L1TriggerByteStreamDecoderMonitoring
  decoderTools += [acc.popToolsAndMerge(eFexByteStreamToolCfg(flags=ConfigFlags,name='eFexBSDecoder',TOBs=True,xTOBs=True,decodeInputs=decodeInputs))]
  decoderTools += [acc.popToolsAndMerge(jFexRoiByteStreamToolCfg(flags=ConfigFlags,name="jFexBSDecoderTool",writeBS=False))]
  decoderTools += [acc.popToolsAndMerge(gFexByteStreamToolCfg(flags=ConfigFlags,name="gFexBSDecoderTool",writeBS=False))]

  if decodeInputs:
    decoderTools += [acc.popToolsAndMerge(jFexInputByteStreamToolCfg(flags=ConfigFlags,name='jFexInputBSDecoderTool',writeBS=False))]
    decoderTools += [acc.popToolsAndMerge(gFexInputByteStreamToolCfg(flags=ConfigFlags,name='gFexInputBSDecoderTool',writeBS=False))]


  acc.addEventAlgo(CompFactory.L1TriggerByteStreamDecoderAlg(
      name="L1TriggerByteStreamDecoder",
      OutputLevel=Constants.ERROR, # hides warnings about non-zero status codes in fragments ... will show up in hists
      DecoderTools=decoderTools,
      MaybeMissingROBs= [id for tool in decoderTools for id in tool.ROBIDs ] if partition.name()!="ATLAS" or not partition.isValid() else [], # allow missing ROBs away from online ATLAS partition
      MonTool=L1TriggerByteStreamDecoderMonitoring("L1TriggerByteStreamDecoder", ConfigFlags, decoderTools)
    ),sequenceName='AthAlgSeq'
  )

  # rerun sim if required
  if ConfigFlags.Trigger.L1.doeFex or ConfigFlags.Trigger.L1.dojFex or ConfigFlags.Trigger.L1.dogFex:
    from L1CaloFEXSim.L1CaloFEXSimCfg import L1CaloFEXSimCfg
    acc.merge(L1CaloFEXSimCfg(ConfigFlags, eFexTowerInputs = ['L1_eFexDataTowers','L1_eFexEmulatedTowers'], deadMaterialCorrections=True)) # configures dual-input mode


  from TrigT1CaloMonitoring.EfexMonitorAlgorithm import EfexMonitoringConfig
  acc.merge(EfexMonitoringConfig(ConfigFlags))
  EfexMonAlg = acc.getEventAlgo('EfexMonAlg')
  # do we need next lines??
  EfexMonAlg.eFexEMTobKeyList = ['L1_eEMRoI', 'L1_eEMxRoI'] # default is just L1_eEMRoI
  EfexMonAlg.eFexTauTobKeyList = ['L1_eTauRoI', 'L1_eTauxRoI']

  if ConfigFlags.Trigger.L1.doeFex or ConfigFlags.Trigger.L1.dojFex or ConfigFlags.Trigger.L1.dogFex:
    #  Adjust eFEX containers to be monitored to also monitor the sim RoI
    for list in [EfexMonAlg.eFexEMTobKeyList,EfexMonAlg.eFexTauTobKeyList]: list += [x + "Sim" for x in list ]
    # monitoring of simulation vs hardware
    from TrigT1CaloMonitoring.EfexSimMonitorAlgorithm import EfexSimMonitoringConfig
    acc.merge(EfexSimMonitoringConfig(ConfigFlags))
    # EfexSimMonitorAlgorithm = acc.getEventAlgo('EfexSimMonAlg')

  # and now book the histograms that depend on the containers
  from TrigT1CaloMonitoring.EfexMonitorAlgorithm import EfexMonitoringHistConfig
  acc.merge(EfexMonitoringHistConfig(ConfigFlags,EfexMonAlg))
  from TrigT1CaloMonitoring.JfexMonitorAlgorithm import JfexMonitoringConfig
  acc.merge(JfexMonitoringConfig(ConfigFlags))
  from TrigT1CaloMonitoring.GfexMonitorAlgorithm import GfexMonitoringConfig
  acc.merge(GfexMonitoringConfig(ConfigFlags))

  # input data monitoring
  if decodeInputs:
    from TrigT1CaloMonitoring.EfexInputMonitorAlgorithm import EfexInputMonitoringConfig
    acc.merge(EfexInputMonitoringConfig(ConfigFlags))
    from TrigT1CaloMonitoring.JfexInputMonitorAlgorithm import JfexInputMonitoringConfig
    acc.merge(JfexInputMonitoringConfig(ConfigFlags))
    from TrigT1CaloMonitoring.GfexInputMonitorAlgorithm import GfexInputMonitoringConfig
    acc.merge(GfexInputMonitoringConfig(ConfigFlags))

  if args.fexReadoutFilter:
    # want to take existing AthAllSeqSeq and move it inside a new sequence
    topSeq = acc.getSequence("AthAlgEvtSeq")
    algSeq = acc.getSequence("AthAllAlgSeq")
    # topSeq has three sub-sequencers ... preserve first and last
    topSeq.Members = [topSeq.Members[0],CompFactory.AthSequencer("NewAthAllAlgSeq"),topSeq.Members[-1]]
    acc.addEventAlgo(CompFactory.L1IDFilterAlgorithm(),sequenceName="NewAthAllAlgSeq")
    acc.getSequence("NewAthAllAlgSeq").Members += [algSeq]
    acc.printConfig()

  from AthenaConfiguration.Utils import setupLoggingLevels
  setupLoggingLevels(ConfigFlags,acc)

  if acc.getService("AthenaEventLoopMgr"): acc.getService("AthenaEventLoopMgr").IntervalInSeconds = 30

  # -------- CHANGES GO ABOVE ------------

if ConfigFlags.Exec.MaxEvents==0: acc.printConfig()
print("Configured Services:",*[svc.name for svc in acc.getServices()])
print("Configured EventAlgos:",*[alg.name for alg in acc.getEventAlgos()])
print("Configured CondAlgos:",*[alg.name for alg in acc.getCondAlgos()])


if not isComponentAccumulatorCfg():
  appendCAtoAthena(acc)
  #svcMgr.StoreGateSvc.Dump=True;svcMgr.DetectorStore.Dump=True
  #svcMgr.IOVDbSvc.OutputLevel=DEBUG; print(svcMgr.IOVDbSvc)
else:
  if acc.run().isFailure():
    import sys
    sys.exit(1)
