# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AllConfigFlags import ConfigFlags
from AthenaConfiguration.Enums import Format, BeamType
from AthenaCommon.AppMgr import theApp, ServiceMgr as svcMgr
from AthenaCommon.Include import include
from AthenaCommon.Logging import logging
from AthenaCommon import Constants
from AthenaCommon.GlobalFlags import globalflags
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
from AthenaCommon.BeamFlags import jobproperties
from AthenaCommon.Resilience import treatException
from AthenaCommon.AppMgr import ToolSvc
from RecExConfig.RecFlags import rec
log = logging.getLogger('runTrackingStandalone.py')

if len(athenaCommonFlags.FilesInput())>0:
    ConfigFlags.Input.Files = athenaCommonFlags.FilesInput()
    import PyUtils.AthFile as athFile
    af = athFile.fopen(athenaCommonFlags.FilesInput()[0])
    globalflags.InputFormat = 'bytestream' if af.fileinfos['file_type']=='bs' else 'pool'
    globalflags.DataSource = 'data' if af.fileinfos['evt_type'][0]=='IS_DATA' else 'geant4'
    ConfigFlags.Input.isMC = False if globalflags.DataSource=='data' else True
    globalflags.DetDescrVersion=af.fileinfos.get('geometry',None)
    if globalflags.DataSource=='data':
        globalflags.ConditionsTag = 'CONDBR2-BLKPA-2018-13'
    else:
        globalflags.ConditionsTag = 'OFLCOND-MC16-SDR-RUN2-09' #'OFLCOND-RUN12-SDR-31' #'OFLCOND-MC16-SDR-25-02'
else:
    globalflags.InputFormat = 'bytestream'
    globalflags.DataSource = 'data' if not opt.setupForMC else 'data'
    ConfigFlags.Input.isMC = False
    ConfigFlags.Input.Collections = []

#athenaCommonFlags.EvtMax=1
ConfigFlags.Input.Format = Format.BS if globalflags.InputFormat=='bytestream' else Format.POOL
# Load input collection list from POOL metadata
from RecExConfig.ObjKeyStore import objKeyStore
if ConfigFlags.Input.Format == Format.POOL:
    from PyUtils.MetaReaderPeeker import convert_itemList
    objKeyStore.addManyTypesInputFile(convert_itemList(layout='#join'))

# Set final Cond/Geo tag based on input file, command line or default
ConfigFlags.GeoModel.AtlasVersion = globalflags.DetDescrVersion()
ConfigFlags.IOVDb.GlobalTag = globalflags.ConditionsTag()

ConfigFlags.Beam.Type = BeamType.Collisions
jobproperties.Beam.beamType = 'collisions'
jobproperties.Beam.bunchSpacing = 25
if not ConfigFlags.Input.isMC:
    globalflags.DatabaseInstance='CONDBR2' if opt.useCONDBR2 else 'COMP200'
    ConfigFlags.IOVDb.DatabaseInstance=globalflags.DatabaseInstance()
athenaCommonFlags.isOnline.set_Value_and_Lock(False)
ConfigFlags.Common.isOnline = athenaCommonFlags.isOnline()

log.info('Configured the following global flags:')
globalflags.print_JobProperties()

rec.doWriteRDO = False  # RecExCommon flag
ConfigFlags.Output.doWriteRDO = True  # new JO flag
if not ConfigFlags.Output.RDOFileName:
    ConfigFlags.Output.RDOFileName = 'RDO_test.pool.root'

from AthenaCommon.DetFlags import DetFlags
DetFlags.BField_setOn()
DetFlags.ID_setOn()
DetFlags.Calo_setOn()
DetFlags.Muon_setOff()
DetFlags.Forward_setOff()
DetFlags.overlay.all_setOff()
DetFlags.digitize.all_setOff()
DetFlags.readRDOBS.all_setOff()
DetFlags.readRIOBS.all_setOff()
DetFlags.readRIOPool.all_setOff()
DetFlags.simulateLVL1.all_setOff()
#DetFlags.simulateLVL1.Tile_setOn()
#DetFlags.simulateLVL1.LAr_setOn()
#DetFlags.simulateLVL1.Calo_setOn()
#DetFlags.simulateLVL1.LVL1_setOn()
DetFlags.simulate.all_setOff()
DetFlags.writeBS.all_setOff()
DetFlags.Print()

from InDetRecExample.InDetJobProperties import InDetFlags
rec.doTruth=False
rec.doTrigger=False
InDetFlags.doTruth=False
InDetFlags.doCaloSeededAmbi=False
InDetFlags.doMonitoringGlobal.set_Value_and_Lock                   (False)
InDetFlags.doMonitoringPrimaryVertexingEnhanced.set_Value_and_Lock (False)
InDetFlags.doMonitoringPixel.set_Value_and_Lock                    (False)
InDetFlags.doMonitoringSCT.set_Value_and_Lock                      (False)
InDetFlags.doMonitoringTRT.set_Value_and_Lock                      (False)
InDetFlags.doMonitoringAlignment.set_Value_and_Lock                (False)
if globalflags.DataSource != 'data':
    from AtlasGeoModel.InDetGMJobProperties import InDetGeometryFlags;
    InDetGeometryFlags.useDynamicAlignFolders.set_Value_and_Lock            (False)
InDetFlags.doPrintConfigurables.set_Value_and_Lock                 (True)
InDetFlags.doTIDE_Ambi.set_Value_and_Lock                          (True)
InDetFlags.doVertexFinding=False
InDetFlags.doCaloSeededBrem=True
InDetFlags.doCaloSeededTRTSegments=True
InDetFlags.doStatistics=False
InDetFlags.doTTVADecos=False
InDetFlags.doLowBetaFinder=False
InDetFlags.doTRTSeededTrackFinder=True
InDetFlags.doBackTracking=True
InDetFlags.doSlimming=False

from InDetRecExample.InDetKeys import InDetKeys

#flags are for Run-2, no pileup optimization, from LArConfigRun2.py.  nSamples=4 is definitely needed, the others probably don't hurt.  need a better way to deal with this though
from LArROD.LArRODFlags import larRODFlags
larRODFlags.doOFCPileupOptimization = False
larRODFlags.useHighestGainAutoCorr = False
larRODFlags.firstSample = 0  # default
larRODFlags.nSamples.set_Value_and_Lock(4)     # default
larRODFlags.NumberOfCollisions = 0
#commenting these out because i'm not sure where they are in the old config
#flags.Digitization.HighGainEMECIW = True
#flags.Digitization.HighGainFCal = True

from TileRecUtils.TileRecFlags import jobproperties
jobproperties.TileRecFlags.TileRunType = 1  # physics run type
jobproperties.TileRecFlags.doTileOpt2 = False  # disable optimal filter with iterations
jobproperties.TileRecFlags.doTileOptATLAS = True  # run optimal filter without iterations
jobproperties.TileRecFlags.correctAmplitude = True  # apply parabolic correction
jobproperties.TileRecFlags.correctTime = False  # don't need time correction if best phase is used
jobproperties.TileRecFlags.BestPhaseFromCOOL = True  # use best phase stored in DB

from CaloRec.CaloTopoClusterFlags import jobproperties
jobproperties.CaloTopoClusterFlags.doCalibHitMoments=False
#jobproperties.CaloTopoClusterFlags.doTopoClusterLocalCalib=False

ConfigFlags.lock()
from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper

from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
CAtoGlobalWrapper(IOVDbSvcCfg, ConfigFlags)

# ----------------------------------------------------------------
# Pool input
# ----------------------------------------------------------------
print("ConfigFlags.Input.Format", ConfigFlags.Input.Format)
print("ConfigFlags.Trigger.Online.isPartition", ConfigFlags.Trigger.Online.isPartition)
if ConfigFlags.Input.Format == Format.POOL:
    import AthenaPoolCnvSvc.ReadAthenaPool   # noqa
    svcMgr.AthenaPoolCnvSvc.PoolAttributes = [ "DEFAULT_BUFFERSIZE = '2048'" ]
    svcMgr.PoolSvc.AttemptCatalogPatch=True

# ----------------------------------------------------------------
# ByteStream input
# ----------------------------------------------------------------
elif ConfigFlags.Input.Format == Format.BS and not ConfigFlags.Trigger.Online.isPartition:
    # Set up ByteStream reading services
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    CAtoGlobalWrapper(ByteStreamReadCfg, ConfigFlags)

from AthenaCommon.AlgSequence import AlgSequence,AthSequencer
topSequence = AlgSequence()
condSeq = AthSequencer ('AthCondSeq')
    
if ConfigFlags.Input.Format == Format.POOL:
    print("POOL input")
    if objKeyStore.isInInput("xAOD::EventInfo"):
        print("EventInfo in input")
        topSequence.SGInputLoader.Load += [( 'xAOD::EventInfo' , 'StoreGateSvc+Bkg_EventInfo' )]
else:
    topSequence.SGInputLoader.Load += [( 'xAOD::EventInfo' , 'StoreGateSvc+Bkg_EventInfo' )]

#set up detector stuff
include("RecExCond/AllDet_detDescr.py")
#make calo cells, from CaloRec_jobOptions.py
from AthenaCommon.Include import excludeTracePattern
excludeTracePattern.append("*/CaloClusterCorrection/common.py")
try:
    include ("TileRec/TileDefaults_jobOptions.py")
except Exception:
    treatException("Could not set up Tile default options.")
# CaloCellGetter
try:
    from CaloRec.CaloCellGetter import CaloCellGetter
    CaloCellGetter()
except Exception:
    treatException("Problem with CaloCellGetter. Switched off.")
    DetFlags.makeRIO.Calo_setOff()
#Need to explicitly turn on TileRawChannelMaker
topSequence.CaloCellMaker.CaloCellMakerToolNames[1].EventInfo="Bkg_EventInfo"
try:
    from TileRecUtils.TileRawChannelGetter import TileRawChannelGetter
    theTileRawChannelGetter = TileRawChannelGetter() # noqa: F841
    topSequence.TileRChMaker.TileDigitsContainer="Bkg_TileDigitsCnt"
except Exception:
    treatExeption("could not get handle to tileRawChannelGetter")
try:
    from TileRecAlgs.TileRecAlgsConf import TileDigitsFilter
    topSequence += TileDigitsFilter()
    topSequence.TileDigitsFilter.InputDigitsContainer="Bkg_TileDigitsCnt"
except Exception:
    treatExeption("could not get handle to TileDigitsFilter")

#CaloTopoCluster making, from CaloTopoCluster_jobOptions.py
try:
    from CaloRec.CaloClusterTopoGetter import CaloClusterTopoGetter
    CaloClusterTopoGetter()
except Exception:
    from AthenaCommon.Resilience import treatException
    treatException("Problem with CaloTopoCluster. Switched off")

#also for LAr, have to force this since it thinks it doesn't have to do it for reading from RDO
from LArROD.LArRODConfig import getLArRawChannelBuilder
theLArRawChannelBuilder=getLArRawChannelBuilder()
theLArRawChannelBuilder.LArDigitKey = "Bkg_LArDigitContainer_MC" 
topSequence+=theLArRawChannelBuilder

#to get the eGammaTopoCluster collection (copied from SystemRec_config.py)
from egammaAlgs.egammaTopoClusterCopierConfig import egammaTopoClusterCopierCfg
CAtoGlobalWrapper(egammaTopoClusterCopierCfg,ConfigFlags)
#to run the ID tracking
include("InDetRecExample/InDetRec_jobOptions.py")

#set various containers to be the Bkg_ ones from the overlay RDO
topSequence.InDetPixelClusterization.DataObjectName="Bkg_PixelRDOs"
topSequence.InDetSCT_Clusterization.DataObjectName="Bkg_SCT_RDOs"
topSequence.InDetTRT_RIO_Maker.TRTRDOLocation="Bkg_TRT_RDOs"
condSeq.LuminosityCondAlg.EventInfoKey="Bkg_EventInfo"
condSeq.LuminosityCondAlg.actualMuKey="Bkg_EventInfo.actualInteractionsPerCrossing"
condSeq.LuminosityCondAlg.averageMuKey="Bkg_EventInfo.averageInteractionsPerCrossing"
ToolSvc.InDetTRT_StandaloneScoringTool.LuminosityTool.EventInfoKey="Bkg_EventInfo"
ToolSvc.InDetTRT_StandaloneScoringTool.LuminosityTool.actualInteractionsPerCrossingKey="Bkg_EventInfo.actualInteractionsPerCrossing"
ToolSvc.InDetTRT_StandaloneScoringTool.LuminosityTool.averageInteractionsPerCrossingKey="Bkg_EventInfo.averageInteractionsPerCrossing"
topSequence.InDetSiSpTrackFinder.EventInfoKey="Bkg_EventInfo"
topSequence.InDetSiSpTrackFinderR3LargeD0.EventInfoKey="Bkg_EventInfo"
topSequence.InDetSiSpTrackFinderForwardTracks.EventInfoKey="Bkg_EventInfo"
topSequence.InDetSiSpTrackFinderDisappearing.EventInfoKey="Bkg_EventInfo"
topSequence.InDetBCM_ZeroSuppression.BcmContainerName="Bkg_BCM_RDOs"
topSequence.CaloBCIDAvgAlg.EventInfoKey="Bkg_EventInfo"

#set the output
itemsToRecord = ['TrackCollection#'+InDetKeys.Tracks(), 'TrackCollection#DisappearingTracks', 'TrackCollection#ResolvedForwardTracks', 'TrackCollection#ResolvedLargeD0Tracks',
                 'InDet::TRT_DriftCircleContainer#TRT_DriftCircles', "InDet::PixelClusterContainer#PixelClusters", "InDet::SCT_ClusterContainer#SCT_Clusters"]
#itemsToRecord = []
from OutputStreamAthenaPool.CreateOutputStreams import  createOutputStream
streamRDO=createOutputStream("StreamRDO","RDO_test.pool.root",True,False,"Bkg_EventInfo")
streamRDO.TakeItemsFromInput = True
streamRDO.ItemList+=itemsToRecord

#-------------------------------------------------------------
# MessageSvc
#-------------------------------------------------------------
svcMgr.MessageSvc.Format = "% F%40W%C%4W%R%e%s%8W%R%T %0W%M"
svcMgr.MessageSvc.enableSuppression = False

if ConfigFlags.Input.isMC:
    # Disable spurious warnings from HepMcParticleLink, ATR-21838
    svcMgr.MessageSvc.setError += ['HepMcParticleLink']
