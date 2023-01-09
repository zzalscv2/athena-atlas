# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Author: William L. (william.axel.leight@cern.ch)
# Author: FY T. (fang-ying.tsai@cern.ch)

from AthenaConfiguration.AllConfigFlags import ConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg, MessageSvcCfg
from AthenaConfiguration.Enums import Format, BeamType

ConfigFlags.Input.Files = ['../RDO.29616558._005525.pool.root.1']
ConfigFlags.Input.isMC = True
#see MainServicesConfig.py
ConfigFlags.Concurrency.NumThreads = 1
ConfigFlags.Input.Format = Format.POOL

# Set final Cond/Geo tag based on input file
ConfigFlags.GeoModel.AtlasVersion='ATLAS-R3S-2021-02-00-00'
ConfigFlags.IOVDb.GlobalTag = 'OFLCOND-MC21-SDR-RUN3-07'

ConfigFlags.Beam.Type = BeamType.Collisions
ConfigFlags.Beam.BunchSpacing = 25
ConfigFlags.Beam.NumberOfCollisions = 60

ConfigFlags.Common.isOnline = False

#ConfigFlags.Output.doWriteBS = True # default False. #  write out RDO ByteStream file
ConfigFlags.Output.doWriteRDO = True  
if not ConfigFlags.Output.RDOFileName:
    ConfigFlags.Output.RDOFileName = 'RDO_test.pool.root'
# DetectorConfigFlags.py is based upon DetFlags.py. Auto configured from the ATLAS-R3S-2021-02-00-00 tag
ConfigFlags.Detector.GeometryID = True
ConfigFlags.Detector.GeometryCalo = True
ConfigFlags.Detector.GeometryForward = False
ConfigFlags.Detector.EnableCalo = True
ConfigFlags.Reco.EnableTracking = True

# InDetConfigFlags.py is based upon InDetFlags. Dropped lots of flags. 
ConfigFlags.InDet.doTruth = False #Turn running of truth matching on and off (by default on for MC off for data)
ConfigFlags.InDet.Tracking.doCaloSeededAmbi = False
ConfigFlags.InDet.Tracking.doTIDE_Ambi = True
ConfigFlags.InDet.Tracking.doCaloSeededBrem = True
ConfigFlags.InDet.Tracking.doBackTracking = True
ConfigFlags.InDet.PriVertex.doVertexFinding = False # from VertexFindingFlags.py

# Updated for Monitoring from DQConfigFlags.py
# Missing? doMonitoringPrimaryVertexingEnhanced
ConfigFlags.DQ.Steering.InDet.doGlobalMon = False
ConfigFlags.DQ.Steering.doPixelMon = False
ConfigFlags.DQ.Steering.doSCTMon = False
ConfigFlags.DQ.Steering.doTRTMon = False
ConfigFlags.DQ.Steering.InDet.doAlignMon = False

##Updates are based on LArConfigFlags.py. This stuff is in the LArConfigRun3PileUp, see LArConfigRun3.py
ConfigFlags.LAr.ROD.DoOFCPileupOptimization = True # default is False, but if setting up LArConfigRun3PileUp it must be True
ConfigFlags.LAr.ROD.UseHighestGainAutoCorr = True # default is False, but if setting up LArConfigRun3PileUp it must be True
ConfigFlags.LAr.ROD.FirstSample = 0 # default
ConfigFlags.LAr.ROD.nSamples = 4 # default is 5
ConfigFlags.LAr.ROD.NumberOfCollisions = 60 # default is 0, but Run-3 default is 60

# Updates are based DigitizationConfigFlags.py
ConfigFlags.Digitization.HighGainEMECIW = False #default is True, but the LArConfigRun3.py sets this to False
ConfigFlags.Digitization.HighGainFCal = False # default is False
# Updates are  based on TileConfigFlags.py
ConfigFlags.Tile.RunType = 'PHY' # physics run type. # Tile run types: UNDEFINED, PHY, PED, LAS, BILAS, CIS, MONOCIS
ConfigFlags.Tile.doOpt2 = False  # disable optimal filter with iterations
ConfigFlags.Tile.doOptATLAS = True # run optimal filter without iterations
ConfigFlags.Tile.correctAmplitude = True # apply parabolic correction
ConfigFlags.Tile.correctTime = False # don't need time correction if best phase is used
ConfigFlags.Tile.BestPhaseFromCOOL = True # use best phase stored in DB
#To set the output stream eventInfoKey to be Bkg_EventInfo, see OutputStreamConfig.py
from AthenaConfiguration.Enums import ProductionStep
ConfigFlags.Common.ProductionStep = ProductionStep.PileUpPresampling

ConfigFlags.lock()
ConfigFlags.dump()
acc = MainServicesCfg(ConfigFlags)

from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
acc.merge(IOVDbSvcCfg(ConfigFlags))

# ----------------------------------------------------------------
# Pool input
# ----------------------------------------------------------------
print("ConfigFlags.Input.Format", ConfigFlags.Input.Format)
print("ConfigFlags.Trigger.Online.isPartition", ConfigFlags.Trigger.Online.isPartition)

# Load input collection list from POOL metadata
from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
acc.merge(SGInputLoaderCfg( ConfigFlags, Load=[( 'xAOD::EventInfo' , 'StoreGateSvc+Bkg_EventInfo' )]))

from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
acc.merge(PoolReadCfg(ConfigFlags))
#set up detector stuff
from CaloRec.CaloRecoConfig import CaloRecoCfg
acc.merge(CaloRecoCfg(ConfigFlags))
#from CaloRecoConfig.py
acc.getEventAlgo('CaloCellMaker').CaloCellMakerToolNames[1].EventInfo="Bkg_EventInfo"
from TileRecUtils.TileRawChannelMakerConfig import TileRawChannelMakerCfg
acc.merge(TileRawChannelMakerCfg(ConfigFlags))
acc.getEventAlgo('TileRChMaker').TileDigitsContainer="Bkg_TileDigitsCnt"
from TileRecAlgs.TileDigitsFilterConfig import TileDigitsFilterCfg
acc.merge(TileDigitsFilterCfg(ConfigFlags))
acc.getEventAlgo('TileDigitsFilter').InputDigitsContainer="Bkg_TileDigitsCnt"
from LArROD.LArRawChannelBuilderAlgConfig import LArRawChannelBuilderAlgCfg
acc.merge(LArRawChannelBuilderAlgCfg(ConfigFlags,  LArDigitKey = "Bkg_LArDigitContainer_MC"))

#RuntimeError: Attempt to modify locked flag container
# from LArConfiguration.LArConfigRun3 import LArConfigRun3PileUp
# acc.merge(LArConfigRun3PileUp(ConfigFlags))

#to get the eGammaTopoCluster collection (copied from SystemRec_config.py)
# Detector flags from DetectorConfigFlags.py
if ConfigFlags.Detector.EnableCalo:
    from egammaAlgs.egammaTopoClusterCopierConfig import egammaTopoClusterCopierCfg
    acc.merge(egammaTopoClusterCopierCfg(ConfigFlags))
# Reco flags from RecoConfigFlags.py
if ConfigFlags.Reco.EnableTracking:
    from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
    acc.merge(InDetTrackRecoCfg(ConfigFlags))
from InDetConfig.InDetPrepRawDataFormationConfig import PixelClusterizationCfg, SCTClusterizationCfg, InDetTRT_RIO_MakerCfg

acc.merge(PixelClusterizationCfg(ConfigFlags, DataObjectName="Bkg_PixelRDOs"))
acc.merge(SCTClusterizationCfg(ConfigFlags, DataObjectName="Bkg_SCT_RDOs"))
acc.merge(InDetTRT_RIO_MakerCfg(ConfigFlags, TRTRDOLocation="Bkg_TRT_RDOs"))
from LumiBlockComps.LuminosityCondAlgConfig import LuminosityCondAlgCfg
acc.merge(LuminosityCondAlgCfg(ConfigFlags))
acc.getCondAlgo('LuminosityCondAlg').EventInfoKey="Bkg_EventInfo"
acc.getCondAlgo('LuminosityCondAlg').actualMuKey="Bkg_EventInfo.actualInteractionsPerCrossing"
acc.getCondAlgo('LuminosityCondAlg').averageMuKey="Bkg_EventInfo.averageInteractionsPerCrossing"

# See TRTStandaloneConfig.py
flagsTRT = ConfigFlags.cloneAndReplace("InDet.Tracking.ActiveConfig", "InDet.Tracking.TRTStandalonePass")
from InDetConfig.InDetTrackScoringToolsConfig import InDetTRT_StandaloneScoringToolCfg
InDetTRT_StandaloneScoringTool = acc.popToolsAndMerge(InDetTRT_StandaloneScoringToolCfg(flagsTRT))
acc.getPublicTool('InDetTRT_StandaloneScoringTool').LuminosityTool.EventInfoKey="Bkg_EventInfo"
acc.getPublicTool('InDetTRT_StandaloneScoringTool').LuminosityTool.actualInteractionsPerCrossingKey="Bkg_EventInfo.actualInteractionsPerCrossing"
acc.getPublicTool('InDetTRT_StandaloneScoringTool').LuminosityTool.averageInteractionsPerCrossingKey="Bkg_EventInfo.averageInteractionsPerCrossing"
acc.getEventAlgo('InDetSiSpTrackFinder').EventInfoKey="Bkg_EventInfo"
acc.getEventAlgo('InDetSiSpTrackFinderR3LargeD0').EventInfoKey="Bkg_EventInfo"
acc.getEventAlgo('InDetSiSpTrackFinderForward').EventInfoKey="Bkg_EventInfo"
acc.getEventAlgo('InDetSiSpTrackFinderDisappearing').EventInfoKey="Bkg_EventInfo"

from CaloRec.CaloBCIDAvgAlgConfig import CaloBCIDAvgAlgCfg
acc.merge(CaloBCIDAvgAlgCfg(ConfigFlags))
acc.getEventAlgo('CaloBCIDAvgAlg').EventInfoKey="Bkg_EventInfo"
from InDetConfig.BCM_ZeroSuppressionConfig import BCM_ZeroSuppressionCfg
acc.merge(BCM_ZeroSuppressionCfg(ConfigFlags))
acc.getEventAlgo('InDetBCM_ZeroSuppression').BcmContainerName="Bkg_BCM_RDOs"
acc.getPublicTool('InDetTRT_StandaloneScoringTool').LuminosityTool.EventInfoKey="Bkg_EventInfo"
acc.getPublicTool('InDetTRT_StandaloneScoringTool').LuminosityTool.actualInteractionsPerCrossingKey="Bkg_EventInfo.actualInteractionsPerCrossing"
acc.getPublicTool('InDetTRT_StandaloneScoringTool').LuminosityTool.averageInteractionsPerCrossingKey="Bkg_EventInfo.averageInteractionsPerCrossing"

itemsToRecord = ['TrackCollection#CombinedInDetTracks', 'TrackCollection#DisappearingTracks', 'TrackCollection#ResolvedForwardTracks', 'TrackCollection#ResolvedLargeD0Tracks',
                 'InDet::TRT_DriftCircleContainer#TRT_DriftCircles', "InDet::PixelClusterContainer#PixelClusters", "InDet::SCT_ClusterContainer#SCT_Clusters"]

from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
acc.merge(OutputStreamCfg(ConfigFlags,"RDO", ItemList=itemsToRecord))
acc.getEventAlgo("EventInfoTagBuilder").EventInfoKey="Bkg_EventInfo" # see OutputStreamConfig.py

# Keep input RDO objects in the output file
streamAlg=acc.getEventAlgo("OutputStreamRDO")
streamAlg.TakeItemsFromInput=True

# -------------------------------------------------------------
# MessageSvc
# -------------------------------------------------------------
acc.merge(MessageSvcCfg(ConfigFlags))
acc.getService("MessageSvc").Format = "% F%40W%C%4W%R%e%s%8W%R%T %0W%M"
acc.getService("MessageSvc").enableSuppression = True
acc.run(2)

