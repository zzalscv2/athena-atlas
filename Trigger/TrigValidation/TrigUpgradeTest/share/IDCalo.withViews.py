#
#  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
#

from AthenaCommon.GlobalFlags import globalflags
globalflags.DetGeo.set_Value_and_Lock('atlas')
globalflags.Luminosity.set_Value_and_Lock('zero')
globalflags.DataSource.set_Value_and_Lock('data')
globalflags.InputFormat.set_Value_and_Lock('bytestream')
globalflags.DatabaseInstance.set_Value_and_Lock('CONDBR2')

#workaround to prevent online trigger folders to be enabled
from InDetTrigRecExample.InDetTrigFlags import InDetTrigFlags
InDetTrigFlags.useConditionsClasses.set_Value_and_Lock(False)



from AthenaCommon.AlgScheduler import AlgScheduler
AlgScheduler.OutputLevel( INFO )
AlgScheduler.CheckDependencies( True )
AlgScheduler.ShowControlFlow( True )
AlgScheduler.ShowDataDependencies( True )
AlgScheduler.setDataLoaderAlg( 'SGInputLoader' )
 
from AthenaCommon.JobProperties import jobproperties
jobproperties.Global.DetDescrVersion = "ATLAS-R2-2015-03-01-00"
 
from AthenaCommon.DetFlags import DetFlags
DetFlags.Calo_setOn()  #Switched off to avoid geometry
DetFlags.ID_setOn()
DetFlags.Muon_setOff()
DetFlags.Truth_setOff()
DetFlags.LVL1_setOff()
DetFlags.digitize.all_setOff()

from InDetRecExample.InDetJobProperties import InDetFlags
InDetFlags.doCaloSeededBrem = False

#DetFlags.haveRIO.all_off()
#DetFlags.haveRIO.TRT_on()
#DetFlags.haveRIO.pixel_setOff()
#DetFlags.haveRIO.SCT_setOff()
#DetFlags.TRT_setOn()
#include("InDetRecExample/InDetRecConditionsAccess.py")

#DetFlags.Print()
 
theApp.EvtMax=10

from IOVDbSvc.CondDB import conddb
conddb.setGlobalTag("CONDBR2-BLKPA-2015-17")
#conddb.addFolder("TRT_ONL","/TRT/Onl/ROD/Compress")
#conddb.addFolder("TRT_OFL","/TRT/Calib/HTCalib")
#conddb.addFolder("TRT_OFL","/TRT/Calib/ToTCalib")
#conddb.addFolder("TRT_OFL","/TRT/Calib/RT")
#conddb.addFolder("TRT_OFL","/TRT/Calib/T0")


 #Set up GeoModel (not really needed but crashes without)
from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import GeoModelInit

from AthenaCommon.AlgSequence import AlgSequence 
topSequence = AlgSequence()
from SGComps.SGCompsConf import SGInputLoader
topSequence += SGInputLoader( )
#topSequence.SGInputLoader.Load
topSequence.SGInputLoader.FailIfNoProxy=False
# -------------------- Condition Data Access --------------------------------
# Conditions Service for reading conditions data in serial and MT Athena

from IOVSvc.IOVSvcConf import CondSvc
svcMgr += CondSvc()

# Special Condition Sequence for CondInputLoader and client Condition Algorithms
from AthenaCommon.AlgSequence import AthSequencer
condSeq = AthSequencer("AthCondSeq")

# CondInputLoader and Condition Store
from IOVSvc.IOVSvcConf import CondInputLoader
condSeq += CondInputLoader( "CondInputLoader")

include( "ByteStreamCnvSvc/BSEventStorageEventSelector_jobOptions.py" )
inputfile="root://eosatlas//eos/atlas/atlascerngroupdisk/trig-daq/validation/test_data/data16_13TeV.00309640.physics_EnhancedBias.merge.RAW/data16_13TeV.00309640.physics_EnhancedBias.merge.RAW._lb0628._SFO-1._0001.1"
svcMgr.ByteStreamInputSvc.FullFileName=[inputfile,]
from AthenaCommon.AthenaCommonFlags  import athenaCommonFlags
athenaCommonFlags.FilesInput=[inputfile,]

from TrigConfigSvc.TrigConfigSvcConf import TrigConf__LVL1ConfigSvc
from TrigConfigSvc.TrigConfigSvcConfig import findFileInXMLPATH
from TriggerJobOpts.TriggerFlags import TriggerFlags

l1svc = TrigConf__LVL1ConfigSvc("LVL1ConfigSvc")
l1svc.XMLMenuFile = findFileInXMLPATH(TriggerFlags.inputLVL1configFile())
svcMgr += l1svc

if not hasattr( svcMgr, "ByteStreamAddressProviderSvc" ):
    from ByteStreamCnvSvcBase.ByteStreamCnvSvcBaseConf import ByteStreamAddressProviderSvc 
    svcMgr += ByteStreamAddressProviderSvc()

svcMgr.ByteStreamAddressProviderSvc.TypeNames += [
    "ROIB::RoIBResult/RoIBResult" ]


l1svc = TrigConf__LVL1ConfigSvc("LVL1ConfigSvc")
l1svc.XMLMenuFile = findFileInXMLPATH(TriggerFlags.inputLVL1configFile())
svcMgr += l1svc


#from TriggerJobOpts.TriggerFlags import TriggerFlags
#TriggerFlags.enableMonitoring = ['Validation', 'Time']

# if not hasattr(svcMgr, 'THistSvc'):
#   from GaudiSvc.GaudiSvcConf import THistSvc
#   svcMgr += THistSvc()
# svcMgr.THistSvc.Output = ["EXPERT DATAFILE='expert-monitoring.root', OPT='RECREATE'"]




from ByteStreamCnvSvcBase.ByteStreamCnvSvcBaseConf import ROBDataProviderSvc
ServiceMgr += ROBDataProviderSvc()

#Run calo decoder
from L1Decoder.L1DecoderMonitoring import CTPUnpackingMonitoring, RoIsUnpackingMonitoring
from L1Decoder.L1DecoderConf import CTPUnpackingTool, EMRoIsUnpackingTool, L1Decoder, MURoIsUnpackingTool
from L1Decoder.L1DecoderConf import CTPUnpackingEmulationTool, RoIsUnpackingEmulationTool
l1Decoder = L1Decoder( OutputLevel=DEBUG )
ctpUnpacker = CTPUnpackingTool( OutputLevel =  DEBUG, ForceEnableAllChains=True )

allChains = [ "HLT_e5_perf", "HLT_e5_lhloose", "HLT_e5_tight", "HLT_e7_perf" ]

l1Decoder.ctpUnpacker = ctpUnpacker
l1Decoder.ctpUnpacker.MonTool = CTPUnpackingMonitoring(512, 200)
#l1Decoder.ctpUnpacker.CTPToChainMapping = ["0:HLT_e3",  "0:HLT_g5", "1:HLT_e7", "2:HLT_2e3", "15:HLT_mu6", "33:HLT_2mu6", "15:HLT_mu6idperf", "42:HLT_e15mu4"] # this are real IDs of L1_* items in pp_v5 menu

emUnpacker = EMRoIsUnpackingTool( OutputLevel=DEBUG, OutputTrigRoIs="EMRoIs" )
emUnpacker.ThresholdToChainMapping = ["EM3 : "+c for c in allChains ]  # CAVEAT this needs real mapping to from the chain to L1 threshold

#emUnpacker.MonTool = RoIsUnpackingMonitoring( prefix="EM", maxCount=30 )

l1Decoder.roiUnpackers = [emUnpacker]
l1Decoder.Chains="HLTChainsResult"

topSequence += l1Decoder


from InDetRecExample.InDetJobProperties import InDetFlags
InDetFlags.InDet25nsec = True 
InDetFlags.doPrimaryVertex3DFinding = False 
InDetFlags.doPrintConfigurables = False
InDetFlags.doResolveBackTracks = True 
InDetFlags.doSiSPSeededTrackFinder = True
InDetFlags.doTRTPhaseCalculation = True
InDetFlags.doTRTSeededTrackFinder = True
InDetFlags.doTruth = False
InDetFlags.init()
#Determine whether we're running in threaded mode (threads= >=1)
from AthenaCommon.ConcurrencyFlags import jobproperties as jp
nThreads = jp.ConcurrencyFlags.NumThreads()

if nThreads >= 1:
  ## get a handle on the Scheduler
  from AthenaCommon.AlgScheduler import AlgScheduler
  AlgScheduler.CheckDependencies( True )

import MagFieldServices.SetupField

# PixelLorentzAngleSvc and SCTLorentzAngleSvc
include("InDetRecExample/InDetRecConditionsAccess.py")

from InDetRecExample.InDetKeys import InDetKeys

include ("InDetRecExample/InDetRecCabling.py")


### Begin view setup

viewTest = True

if ( viewTest ):

  # Make a separate alg pool for the view algs
  from GaudiHive.GaudiHiveConf import AlgResourcePool
  viewAlgPoolName = "ViewAlgPool"
  svcMgr += AlgResourcePool( viewAlgPoolName )

  # Set of view algs
  from AthenaCommon.AlgSequence import AthSequencer
  allViewAlgorithms = AthSequencer( "allViewAlgorithms" )
  allViewAlgorithms.ModeOR = False
  allViewAlgorithms.Sequential = True
  allViewAlgorithms.StopOverride = False

  # view maker
  viewMaker = CfgMgr.AthViews__RoiCollectionToViews( "viewMaker" )
  viewMaker.ViewBaseName = "testView"
  viewMaker.AlgPoolName = viewAlgPoolName
  viewMaker.InputRoICollection = "EMRoIs"
  viewMaker.OutputRoICollection = "EMViewRoIs"
  topSequence += viewMaker

  # Filter to stop view algs from running on whole event
  allViewAlgorithms += CfgMgr.AthPrescaler( "alwaysFail" )
  allViewAlgorithms.alwaysFail.PercentPass = 0.0

  # dummy alg that just says you're running in a view
  allViewAlgorithms += CfgMgr.AthViews__ViewTestAlg( "viewTest" )
  svcMgr.ViewAlgPool.TopAlg += [ "viewTest" ]
  viewMaker.AlgorithmNameSequence = [ "viewTest" ] #Eventually scheduler will do this

  # test setup
  topSequence += allViewAlgorithms

### End view setup

#Pixel

from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConf import PixelRodDecoder
InDetPixelRodDecoder = PixelRodDecoder(name = "InDetPixelRodDecoder")
ToolSvc += InDetPixelRodDecoder

from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConf import PixelRawDataProviderTool
InDetPixelRawDataProviderTool = PixelRawDataProviderTool(name    = "InDetPixelRawDataProviderTool",
                                                         Decoder = InDetPixelRodDecoder)
ToolSvc += InDetPixelRawDataProviderTool
if (InDetFlags.doPrintConfigurables()):
  print      InDetPixelRawDataProviderTool

# load the PixelRawDataProvider
from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConf import PixelRawDataProvider
InDetPixelRawDataProvider = PixelRawDataProvider(name         = "InDetPixelRawDataProvider",
                                                 RDOKey       = InDetKeys.PixelRDOs(),
                                                 ProviderTool = InDetPixelRawDataProviderTool)

if ( viewTest ):
  allViewAlgorithms += InDetPixelRawDataProvider
  allViewAlgorithms.InDetPixelRawDataProvider.isRoI_Seeded = True
  allViewAlgorithms.InDetPixelRawDataProvider.RoIs = "EMViewRoIs"
  svcMgr.ViewAlgPool.TopAlg += [ "InDetPixelRawDataProvider" ]
  topSequence.viewMaker.AlgorithmNameSequence += [ "InDetPixelRawDataProvider" ]
else:
  topSequence += InDetPixelRawDataProvider
  topSequence.InDetPixelRawDataProvider.isRoI_Seeded = True
  topSequence.InDetPixelRawDataProvider.RoIs = "EMRoIs"


if (InDetFlags.doPrintConfigurables()):
  print          InDetPixelRawDataProvider


#SCT
from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConf import SCT_RodDecoder
InDetSCTRodDecoder = SCT_RodDecoder(name        = "InDetSCTRodDecoder",
                                    TriggerMode = False)
ToolSvc += InDetSCTRodDecoder

from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConf import SCTRawDataProviderTool
InDetSCTRawDataProviderTool = SCTRawDataProviderTool(name    = "InDetSCTRawDataProviderTool",
                                                    Decoder = InDetSCTRodDecoder)
ToolSvc += InDetSCTRawDataProviderTool
if (InDetFlags.doPrintConfigurables()):
  print      InDetSCTRawDataProviderTool

# load the SCTRawDataProvider
from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConf import SCTRawDataProvider
InDetSCTRawDataProvider = SCTRawDataProvider(name         = "InDetSCTRawDataProvider",
                                            RDOKey       = InDetKeys.SCT_RDOs(),
                                            ProviderTool = InDetSCTRawDataProviderTool)

if ( viewTest ):
  allViewAlgorithms += InDetSCTRawDataProvider
  allViewAlgorithms.InDetSCTRawDataProvider.isRoI_Seeded = True
  allViewAlgorithms.InDetSCTRawDataProvider.RoIs = "EMViewRoIs"
  svcMgr.ViewAlgPool.TopAlg += [ "InDetSCTRawDataProvider" ]
  topSequence.viewMaker.AlgorithmNameSequence += [ "InDetSCTRawDataProvider" ]
else:
  topSequence += InDetSCTRawDataProvider
  topSequence.InDetSCTRawDataProvider.isRoI_Seeded = True
  topSequence.InDetSCTRawDataProvider.RoIs = "EMRoIs"


#TRT
from TRT_ConditionsServices.TRT_ConditionsServicesConf import TRT_CalDbSvc
InDetTRTCalDbSvc = TRT_CalDbSvc()
ServiceMgr += InDetTRTCalDbSvc

from TRT_ConditionsServices.TRT_ConditionsServicesConf import TRT_StrawStatusSummarySvc
InDetTRTStrawStatusSummarySvc = TRT_StrawStatusSummarySvc(name = "InDetTRTStrawStatusSummarySvc")
ServiceMgr += InDetTRTStrawStatusSummarySvc

from xAODEventInfoCnv.xAODEventInfoCreator import xAODMaker__EventInfoCnvAlg
topSequence+=xAODMaker__EventInfoCnvAlg()


from TRT_RawDataByteStreamCnv.TRT_RawDataByteStreamCnvConf import TRT_RodDecoder
InDetTRTRodDecoder = TRT_RodDecoder(name = "InDetTRTRodDecoder",
                                    LoadCompressTableDB = True)#(globalflags.DataSource() != 'geant4'))  
ToolSvc += InDetTRTRodDecoder
  
from TRT_RawDataByteStreamCnv.TRT_RawDataByteStreamCnvConf import TRTRawDataProviderTool
InDetTRTRawDataProviderTool = TRTRawDataProviderTool(name    = "InDetTRTRawDataProviderTool",
                                                      Decoder = InDetTRTRodDecoder)
ToolSvc += InDetTRTRawDataProviderTool

  
# load the TRTRawDataProvider
from TRT_RawDataByteStreamCnv.TRT_RawDataByteStreamCnvConf import TRTRawDataProvider
InDetTRTRawDataProvider = TRTRawDataProvider(name         = "InDetTRTRawDataProvider",
                                             RDOKey       = "TRT_RDOs",
                                              ProviderTool = InDetTRTRawDataProviderTool)

if ( viewTest ):
  allViewAlgorithms += InDetTRTRawDataProvider
  allViewAlgorithms.InDetTRTRawDataProvider.isRoI_Seeded = True
  allViewAlgorithms.InDetTRTRawDataProvider.RoIs = "EMViewRoIs"
  svcMgr.ViewAlgPool.TopAlg += [ "InDetTRTRawDataProvider" ]
  topSequence.viewMaker.AlgorithmNameSequence += [ "InDetTRTRawDataProvider" ]
else:
  topSequence += InDetTRTRawDataProvider
  topSequence.InDetTRTRawDataProvider.isRoI_Seeded = True
  topSequence.InDetTRTRawDataProvider.RoIs = "EMRoIs"


# MYSTERY LINES THAT CAUSE A PROBLEM
#include ("InDetRecExample/ConfiguredInDetPreProcessingTRT.py")
#InDetPreProcessingTRT = ConfiguredInDetPreProcessingTRT(True,False)
#include("InDetBeamSpotService/BeamCondSvc.py")


#Pixel clusterisation

from SiClusterizationTool.SiClusterizationToolConf import InDet__ClusterMakerTool
InDetClusterMakerTool = InDet__ClusterMakerTool(name                 = "InDetClusterMakerTool",
    PixelCalibSvc        = None,
    PixelOfflineCalibSvc = None,
    UsePixelCalibCondDB  = False)

ToolSvc += InDetClusterMakerTool


from SiClusterizationTool.SiClusterizationToolConf import InDet__MergedPixelsTool
InDetMergedPixelsTool = InDet__MergedPixelsTool(name                    = "InDetMergedPixelsTool",
                                                globalPosAlg            = InDetClusterMakerTool,
                                                MinimalSplitSize        = 0,
                                                MaximalSplitSize        = 49,
                                                MinimalSplitProbability = 0,
                                                DoIBLSplitting = True,
                                                SplitClusterAmbiguityMap= InDetKeys.SplitClusterAmbiguityMap())
ToolSvc += InDetMergedPixelsTool

from SiClusterizationTool.SiClusterizationToolConf import InDet__PixelGangedAmbiguitiesFinder
InDetPixelGangedAmbiguitiesFinder = InDet__PixelGangedAmbiguitiesFinder(name = "InDetPixelGangedAmbiguitiesFinder")
ToolSvc += InDetPixelGangedAmbiguitiesFinder

from InDetPrepRawDataFormation.InDetPrepRawDataFormationConf import InDet__PixelClusterization
InDetPixelClusterization = InDet__PixelClusterization(name                    = "InDetPixelClusterization",
                                                      clusteringTool          = InDetMergedPixelsTool,
                                                      gangedAmbiguitiesFinder = InDetPixelGangedAmbiguitiesFinder,
                                                      DetectorManagerName     = InDetKeys.PixelManager(),
                                                      DataObjectName          = InDetKeys.PixelRDOs(),
                                                      ClustersName            = "PixelTrigClusters")
if ( viewTest ):
  allViewAlgorithms += InDetPixelClusterization
  allViewAlgorithms.InDetPixelClusterization.isRoI_Seeded = True
  allViewAlgorithms.InDetPixelClusterization.RoIs = "EMViewRoIs"
  svcMgr.ViewAlgPool.TopAlg += [ "InDetPixelClusterization" ]
  topSequence.viewMaker.AlgorithmNameSequence += [ "InDetPixelClusterization" ]
else:
  topSequence += InDetPixelClusterization
  topSequence.InDetPixelClusterization.isRoI_Seeded = True
  topSequence.InDetPixelClusterization.RoIs = "EMRoIs"



#
# --- SCT_ClusteringTool (public)
#
from SiClusterizationTool.SiClusterizationToolConf import InDet__SCT_ClusteringTool
InDetSCT_ClusteringTool = InDet__SCT_ClusteringTool(name              = "InDetSCT_ClusteringTool",
                                                    globalPosAlg      = InDetClusterMakerTool,
                                                    conditionsService = InDetSCT_ConditionsSummarySvc)
#
# --- SCT_Clusterization algorithm
#
from InDetPrepRawDataFormation.InDetPrepRawDataFormationConf import InDet__SCT_Clusterization
InDetSCT_Clusterization = InDet__SCT_Clusterization(name                    = "InDetSCT_Clusterization",
                                                    clusteringTool          = InDetSCT_ClusteringTool,
                                                    # ChannelStatus         = InDetSCT_ChannelStatusAlg,
                                                    DetectorManagerName     = InDetKeys.SCT_Manager(),
                                                    DataObjectName          = InDetKeys.SCT_RDOs(),
                                                    ClustersName            = "SCT_TrigClusters",
                                                    conditionsService       = InDetSCT_ConditionsSummarySvc,
                                                    FlaggedConditionService = InDetSCT_FlaggedConditionSvc)

if ( viewTest ):
  allViewAlgorithms += InDetSCT_Clusterization
  allViewAlgorithms.InDetSCT_Clusterization.isRoI_Seeded = True
  allViewAlgorithms.InDetSCT_Clusterization.RoIs = "EMViewRoIs"
  svcMgr.ViewAlgPool.TopAlg += [ "InDetSCT_Clusterization" ]
  topSequence.viewMaker.AlgorithmNameSequence += [ "InDetSCT_Clusterization" ]
else:
  topSequence += InDetSCT_Clusterization
  topSequence.InDetSCT_Clusterization.isRoI_Seeded = True
  topSequence.InDetSCT_Clusterization.RoIs = "EMRoIs"


#Space points and FTF

from SiSpacePointTool.SiSpacePointToolConf import InDet__SiSpacePointMakerTool
InDetSiSpacePointMakerTool = InDet__SiSpacePointMakerTool(name = "InDetSiSpacePointMakerTool")
ToolSvc += InDetSiSpacePointMakerTool

from SiSpacePointFormation.SiSpacePointFormationConf import InDet__SiTrackerSpacePointFinder
InDetSiTrackerSpacePointFinder = InDet__SiTrackerSpacePointFinder(name                   = "InDetSiTrackerSpacePointFinder",
                                                                  SiSpacePointMakerTool  = InDetSiSpacePointMakerTool,
                                                                  PixelsClustersName     = "PixelTrigClusters",
                                                                  SCT_ClustersName       = "SCT_TrigClusters",
                                                                  SpacePointsPixelName   = "PixelTrigSpacePoints",
                                                                  SpacePointsSCTName     = "SCT_TrigSpacePoints",
                                                                  SpacePointsOverlapName = InDetKeys.OverlapSpacePoints(),
                                                                  ProcessPixels          = DetFlags.haveRIO.pixel_on(),
                                                                  ProcessSCTs            = DetFlags.haveRIO.SCT_on(),
                                                                  ProcessOverlaps        = DetFlags.haveRIO.SCT_on())


from TrigFastTrackFinder.TrigFastTrackFinder_Config import TrigFastTrackFinder_eGamma
theFTF = TrigFastTrackFinder_eGamma()
theFTF.OutputLevel = VERBOSE

if ( viewTest ):
  allViewAlgorithms += InDetSiTrackerSpacePointFinder
  allViewAlgorithms += theFTF
  allViewAlgorithms.TrigFastTrackFinder_eGamma.isRoI_Seeded = True
  allViewAlgorithms.TrigFastTrackFinder_eGamma.RoIs = "EMViewRoIs"
  svcMgr.ViewAlgPool.TopAlg += [ "InDetSiTrackerSpacePointFinder", "TrigFastTrackFinder_eGamma" ]
  topSequence.viewMaker.AlgorithmNameSequence += [ "InDetSiTrackerSpacePointFinder", "TrigFastTrackFinder_eGamma" ]
else:
  topSequence += InDetSiTrackerSpacePointFinder
  theFTF.RoIs = "EMRoIs"
  topSequence += theFTF

from LArROD.LArRODFlags import larRODFlags
larRODFlags.doLArFebErrorSummary.set_Value_and_Lock(False)
#from CaloRec.CaloCellGetter import CaloCellGetter
#caloCells = CaloCellGetter()
include("LArConditionsCommon/LArIdMap_comm_jobOptions.py")
include("CaloConditions/LArTTCellMap_ATLAS_jobOptions.py")
include("CaloConditions/CaloTTIdMap_ATLAS_jobOptions.py")
include("LArConditionsCommon/LArConditionsCommon_comm_jobOptions.py")

from RegionSelector.RegSelSvcDefault import RegSelSvcDefault
svcMgr+=RegSelSvcDefault()
svcMgr.RegSelSvc.enableCalo=True
svcMgr.RegSelSvc.enableID=True

from TrigT2CaloCommon.TrigT2CaloCommonConfig import TrigDataAccess
svcMgr.ToolSvc+=TrigDataAccess()
svcMgr.ToolSvc.TrigDataAccess.ApplyOffsetCorrection=False

from TrigT2CaloEgamma.TrigT2CaloEgammaConfig import T2CaloEgamma_FastAlgo
algo=T2CaloEgamma_FastAlgo("testFastAlgo")
algo.RoIs="EMRoIs"
algo.OutputLevel=VERBOSE
#TopHLTSeq += algo
topSequence += algo

include ("RecExCond/AllDet_detDescr.py")
