# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration


from __future__ import print_function

# ------------------------------------------------------------
#
# ----------- Loading of all the Tools needed to configure
#
# ------------------------------------------------------------
#
# common
from AthenaCommon.AppMgr import ToolSvc
from InDetTrigRecExample.InDetTrigFlags import InDetTrigFlags

from InDetTrigRecExample.ConfiguredNewTrackingTrigCuts import EFIDTrackingCuts
InDetTrigCutValues = EFIDTrackingCuts

from AthenaCommon.DetFlags import DetFlags
from AthenaCommon.Logging import logging 
log = logging.getLogger("InDetTrigConfigRecLoadTools.py")

from InDetTrigRecExample.InDetTrigCommonTools import CAtoLegacyPublicToolWrapper

from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_StrawStatusSummaryToolCfg
InDetTrigTRTStrawStatusSummaryTool = CAtoLegacyPublicToolWrapper(TRT_StrawStatusSummaryToolCfg, name="InDetTrigTRTStrawStatusSummaryTool")


from InDetTrigRecExample.InDetTrigConditionsAccess import PixelConditionsSetup, SCT_ConditionsSetup
from AthenaCommon.CfgGetter import getPublicTool,getPrivateTool
TrigPixelLorentzAngleTool = getPublicTool("PixelLorentzAngleTool")
TrigSCTLorentzAngleTool = getPrivateTool("SCTLorentzAngleTool") 



#
# ----------- control loading of ROT_creator
#
if InDetTrigFlags.loadRotCreator():

  #4 clusterOnTrack Tools
  #
  from SiClusterOnTrackTool.SiClusterOnTrackToolConf import InDet__SCT_ClusterOnTrackTool
  SCT_ClusterOnTrackTool = InDet__SCT_ClusterOnTrackTool ("SCT_ClusterOnTrackTool",
                                                          CorrectionStrategy = 0,  # do correct position bias
                                                          ErrorStrategy      = 2,  # do use phi dependent errors
                                                          LorentzAngleTool   = TrigSCTLorentzAngleTool)

  ToolSvc += SCT_ClusterOnTrackTool
  if (InDetTrigFlags.doPrintConfigurables()):
    print (SCT_ClusterOnTrackTool)

  # tool to always make conservative pixel cluster errors
  from SiClusterOnTrackTool.SiClusterOnTrackToolConf import InDet__PixelClusterOnTrackTool

  if InDetTrigFlags.doPixelClusterSplitting():
    from TrkNeuralNetworkUtils.TrkNeuralNetworkUtilsConf import Trk__NeuralNetworkToHistoTool
    NeuralNetworkToHistoTool=Trk__NeuralNetworkToHistoTool(name = "NeuralNetworkToHistoTool")
      
    ToolSvc += NeuralNetworkToHistoTool
    if (InDetTrigFlags.doPrintConfigurables()):
      print (NeuralNetworkToHistoTool)
    
    from SiClusterizationTool.SiClusterizationToolConf import InDet__NnClusterizationFactory
    from AtlasGeoModel.CommonGMJobProperties import CommonGeometryFlags as geoFlags
    do_runI = geoFlags.Run() not in ["RUN2", "RUN3"]
    from InDetRecExample.TrackingCommon import createAndAddCondAlg,getPixelClusterNnCondAlg,getPixelClusterNnWithTrackCondAlg
    createAndAddCondAlg( getPixelClusterNnCondAlg,         'PixelClusterNnCondAlg',          GetInputsInfo = do_runI)
    createAndAddCondAlg( getPixelClusterNnWithTrackCondAlg,'PixelClusterNnWithTrackCondAlg', GetInputsInfo = do_runI)
    if do_runI :
      TrigNnClusterizationFactory = InDet__NnClusterizationFactory( name                 = "TrigNnClusterizationFactory",
                                                                    PixelLorentzAngleTool              = TrigPixelLorentzAngleTool,
                                                                    doRunI                             = True,
                                                                    useToT                             = False,
                                                                    useRecenteringNNWithoutTracks      = True,
                                                                    useRecenteringNNWithTracks         = False,
                                                                    correctLorShiftBarrelWithoutTracks = 0,
                                                                    correctLorShiftBarrelWithTracks    = 0.030,
                                                                    NnCollectionReadKey                = 'PixelClusterNN',
                                                                    NnCollectionWithTrackReadKey       = 'PixelClusterNNWithTrack')
    else:
        TrigNnClusterizationFactory = InDet__NnClusterizationFactory( name                         = "TrigNnClusterizationFactory",
                                                                      PixelLorentzAngleTool        = TrigPixelLorentzAngleTool,
                                                                      useToT                       = InDetTrigFlags.doNNToTCalibration(),
                                                                      NnCollectionReadKey          = 'PixelClusterNN',
                                                                      NnCollectionWithTrackReadKey = 'PixelClusterNNWithTrack')

    ToolSvc += TrigNnClusterizationFactory

  else:
    TrigNnClusterizationFactory = None

  if (InDetTrigFlags.doPrintConfigurables()):
    print (TrigNnClusterizationFactory)

  InDetTrigPixelClusterOnTrackTool = InDet__PixelClusterOnTrackTool("InDetTrigPixelClusterOnTrackTool",
                                                                    ErrorStrategy = 2,
                                                                    LorentzAngleTool = TrigPixelLorentzAngleTool,
                                                                    NnClusterizationFactory= TrigNnClusterizationFactory,
  )

  ToolSvc += InDetTrigPixelClusterOnTrackTool

  if (InDetTrigFlags.doPrintConfigurables()):
    print (InDetTrigPixelClusterOnTrackTool)

  # tool to always make conservative sct cluster errors
  from SiClusterOnTrackTool.SiClusterOnTrackToolConf import InDet__SCT_ClusterOnTrackTool
  InDetTrigBroadSCT_ClusterOnTrackTool = InDet__SCT_ClusterOnTrackTool ("InDetTrigBroadSCT_ClusterOnTrackTool",
                                         CorrectionStrategy = 0,  # do correct position bias
                                         ErrorStrategy      = 0,  # do use broad errors
                                         LorentzAngleTool   = TrigSCTLorentzAngleTool)
  ToolSvc += InDetTrigBroadSCT_ClusterOnTrackTool
  if (InDetTrigFlags.doPrintConfigurables()):
    print (InDetTrigBroadSCT_ClusterOnTrackTool)

  #--
  InDetTrigBroadPixelClusterOnTrackTool = InDet__PixelClusterOnTrackTool("InDetTrigBroadPixelClusterOnTrackTool",
                                                                         ErrorStrategy = 0,
                                                                         LorentzAngleTool = TrigPixelLorentzAngleTool,
                                                                         NnClusterizationFactory= TrigNnClusterizationFactory
  )
  ToolSvc += InDetTrigBroadPixelClusterOnTrackTool
  if (InDetTrigFlags.doPrintConfigurables()):
    print (InDetTrigBroadPixelClusterOnTrackTool)

  # load RIO_OnTrackCreator for Inner Detector
  #

  from TrkRIO_OnTrackCreator.TrkRIO_OnTrackCreatorConf import Trk__RIO_OnTrackCreator
  InDetTrigRotCreator = Trk__RIO_OnTrackCreator(name = 'InDetTrigRotCreator',
                                                ToolPixelCluster= InDetTrigPixelClusterOnTrackTool,
                                                ToolSCT_Cluster = SCT_ClusterOnTrackTool,
                                                Mode = 'indet')
  ToolSvc += InDetTrigRotCreator

  if InDetTrigFlags.useBroadClusterErrors():
    InDetTrigRotCreator.ToolPixelCluster = InDetTrigBroadPixelClusterOnTrackTool
    InDetTrigRotCreator.ToolSCT_Cluster  = InDetTrigBroadSCT_ClusterOnTrackTool

  if (InDetTrigFlags.doPrintConfigurables()):
    print (InDetTrigRotCreator)

  #--
  from TrkRIO_OnTrackCreator.TrkRIO_OnTrackCreatorConf import Trk__RIO_OnTrackCreator
  InDetTrigBroadInDetRotCreator = \
      Trk__RIO_OnTrackCreator(name            = 'InDetTrigBroadInDetRotCreator',
                              ToolPixelCluster= InDetTrigBroadPixelClusterOnTrackTool,
                              ToolSCT_Cluster = InDetTrigBroadSCT_ClusterOnTrackTool,
                              Mode            = 'indet')
  ToolSvc += InDetTrigBroadInDetRotCreator
  if (InDetTrigFlags.doPrintConfigurables()):
    print (InDetTrigBroadInDetRotCreator)

  # load error scaling
  #TODO - instanceName?
  from InDetRecExample.TrackingCommon import createAndAddCondAlg, getRIO_OnTrackErrorScalingCondAlg
  createAndAddCondAlg(getRIO_OnTrackErrorScalingCondAlg,'RIO_OnTrackErrorScalingCondAlg')



#
# ----------- control loading extrapolation
#
if InDetTrigFlags.loadExtrapolator():

  from TrkConfig.AtlasExtrapolatorToolsConfig import AtlasNavigatorCfg,AtlasMaterialEffectsUpdatorCfg
  InDetTrigNavigator = CAtoLegacyPublicToolWrapper(AtlasNavigatorCfg, 
                                                     name="InDetTrigNavigator")

  # Setup the MaterialEffectsUpdator
  InDetTrigMaterialUpdator = CAtoLegacyPublicToolWrapper(AtlasMaterialEffectsUpdatorCfg, 
                                                           name ="InDetTrigMaterialEffectsUpdator")

  from TrkConfig.TrkExRungeKuttaPropagatorConfig import InDetPropagatorCfg
  InDetTrigPropagator = CAtoLegacyPublicToolWrapper(InDetPropagatorCfg, name = "InDetTrigPropagator")
  
  from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
  InDetTrigExtrapolator = CAtoLegacyPublicToolWrapper(InDetExtrapolatorCfg, name="InDetTrigExtrapolator")
  
#
# ----------- control loading of fitters
#

from TrkConfig.TrkGlobalChi2FitterConfig import InDetTrigGlobalChi2FitterCfg,InDetTrigGlobalChi2FitterCosmicsCfg
InDetTrigTrackFitter = CAtoLegacyPublicToolWrapper(InDetTrigGlobalChi2FitterCfg)
InDetTrigTrackFitterCosmics = CAtoLegacyPublicToolWrapper(InDetTrigGlobalChi2FitterCosmicsCfg)

InDetTrigPixelConditionsSummaryTool = PixelConditionsSetup.summaryTool

if DetFlags.haveRIO.SCT_on():
  from SCT_ConditionsTools.SCT_ConditionsToolsConf import SCT_ConditionsSummaryTool
  InDetTrigSCTConditionsSummaryTool = SCT_ConditionsSummaryTool(SCT_ConditionsSetup.instanceName('InDetSCT_ConditionsSummaryTool'))

  # Fix the conditions tools - please tell me there's a better way
  from AthenaCommon.GlobalFlags import globalflags
  fixedTools = []
  for tool in InDetTrigSCTConditionsSummaryTool.ConditionsTools:
    if hasattr( tool, "SCT_FlaggedCondData" ):
      continue
    if not globalflags.InputFormat.is_bytestream() and "ByteStream" in tool.getName():
      continue
    fixedTools.append( tool )
  InDetTrigSCTConditionsSummaryTool.ConditionsTools = fixedTools

else:
  InDetTrigSCTConditionsSummaryTool = None

# ------load association tool from Inner Detector to handle pixel ganged ambiguities
from InDetConfig.InDetAssociationToolsConfig import TrigPrdAssociationToolCfg
InDetTrigPrdAssociationTool = CAtoLegacyPublicToolWrapper(TrigPrdAssociationToolCfg)

#
# ----------- control loading of Summary Tool
#
if InDetTrigFlags.loadSummaryTool():

  # Load Pixel Layer tool
  # used as private tool only, don't add it to ToolSvc
  from InDetTestPixelLayer.InDetTestPixelLayerConf import InDet__InDetTestPixelLayerTool
  InDetTrigTestPixelLayerTool = InDet__InDetTestPixelLayerTool(name             = "InDetTrigTestPixelLayerTool",
                                                               PixelSummaryTool = InDetTrigPixelConditionsSummaryTool,
                                                               Extrapolator     = InDetTrigExtrapolator,
                                                               CheckActiveAreas = True,
                                                               CheckDeadRegions = True)
  if (InDetTrigFlags.doPrintConfigurables()):
    print ( InDetTrigTestPixelLayerTool)

  from InDetBoundaryCheckTool.InDetBoundaryCheckToolConf import InDet__InDetBoundaryCheckTool
  # used as private tool only, don't add it to ToolSvc
  InDetTrigBoundaryCheckTool = InDet__InDetBoundaryCheckTool(name="InDetTrigBoundaryCheckTool",
                                                             UsePixel=DetFlags.haveRIO.pixel_on(),
                                                             UseSCT=DetFlags.haveRIO.SCT_on(),
                                                             SctSummaryTool = InDetTrigSCTConditionsSummaryTool,
                                                             PixelLayerTool = InDetTrigTestPixelLayerTool
                                                             )

  #
  # Loading Configurable HoleSearchTool
  #

  from InDetConfig.InDetTrackHoleSearchConfig import TrigHoleSearchToolCfg
  InDetTrigHoleSearchTool = CAtoLegacyPublicToolWrapper(TrigHoleSearchToolCfg)
  
  #
  # Configrable version of loading the InDetTrackSummaryHelperTool
  #
  from AthenaCommon.GlobalFlags import globalflags
  
  from InDetTrackSummaryHelperTool.InDetTrackSummaryHelperToolConf import InDet__InDetTrackSummaryHelperTool
  from InDetTrigRecExample.InDetTrigConditionsAccess import TRT_ConditionsSetup  # noqa: F401
  InDetTrigTrackSummaryHelperTool = InDet__InDetTrackSummaryHelperTool(name          = "InDetTrigSummaryHelper",
                                                                       HoleSearch    = InDetTrigHoleSearchTool,
                                                                       TRTStrawSummarySvc=InDetTrigTRTStrawStatusSummaryTool,
                                                                       usePixel      = DetFlags.haveRIO.pixel_on(),
                                                                       useSCT        = DetFlags.haveRIO.SCT_on(),
                                                                       useTRT        = DetFlags.haveRIO.TRT_on())

  ToolSvc += InDetTrigTrackSummaryHelperTool
  if (InDetTrigFlags.doPrintConfigurables()):
    print (     InDetTrigTrackSummaryHelperTool)

  #
  # Configurable version of TRT_ElectronPidTools
  #
  from IOVDbSvc.CondDB import conddb

  if not (conddb.folderRequested("/TRT/Calib/PID_vector") or \
            conddb.folderRequested("/TRT/Onl/Calib/PID_vector")):
    conddb.addFolderSplitOnline("TRT","/TRT/Onl/Calib/PID_vector","/TRT/Calib/PID_vector",className='CondAttrListVec')
  if not (conddb.folderRequested("/TRT/Calib/ToT/ToTVectors") or \
            conddb.folderRequested("/TRT/Onl/Calib/ToT/ToTVectors")):
    conddb.addFolderSplitOnline("TRT","/TRT/Onl/Calib/ToT/ToTVectors","/TRT/Calib/ToT/ToTVectors",className='CondAttrListVec')
  if not (conddb.folderRequested("/TRT/Calib/ToT/ToTValue") or \
            conddb.folderRequested("/TRT/Onl/Calib/ToT/ToTValue")):
    conddb.addFolderSplitOnline("TRT","/TRT/Onl/Calib/ToT/ToTValue","/TRT/Calib/ToT/ToTValue",className='CondAttrListCollection')
  if InDetTrigFlags.doTRTPIDNN():
    if not (conddb.folderRequested( "/TRT/Calib/PID_NN") or \
           conddb.folderRequested( "/TRT/Onl/Calib/PID_NN")):
      conddb.addFolderSplitOnline( "TRT", "/TRT/Onl/Calib/PID_NN", "/TRT/Calib/PID_NN",className='CondAttrListCollection')
    # FIXME: need to force an override for the online DB until this folder has been added to the latest tag
    conddb.addOverride("/TRT/Onl/Calib/PID_NN", "TRTCalibPID_NN_v2")

  # Calibration DB Tool
  from TRT_ConditionsServices.TRT_ConditionsServicesConf import TRT_CalDbTool
  InDetTRTCalDbTool = TRT_CalDbTool(name = "TRT_CalDbTool")

 
#
# ----------- control loading of tools which are needed by new tracking and backtracking
#
if InDetTrigFlags.doNewTracking() or InDetTrigFlags.doBackTracking() or InDetTrigFlags.doTrtSegments():
  # Igor's propagator and updator for the pattern
  #
  from TrkExRungeKuttaPropagator.TrkExRungeKuttaPropagatorConf import Trk__RungeKuttaPropagator as Propagator
  InDetTrigPatternPropagator = Propagator(name = 'InDetTrigPatternPropagator')
  
  ToolSvc += InDetTrigPatternPropagator
  if (InDetTrigFlags.doPrintConfigurables()):
     print (     InDetTrigPatternPropagator)

  # fast Kalman updator tool
  #
  from TrkMeasurementUpdator_xk.TrkMeasurementUpdator_xkConf import Trk__KalmanUpdator_xk
  InDetTrigPatternUpdator = Trk__KalmanUpdator_xk(name = 'InDetTrigPatternUpdator')

  ToolSvc += InDetTrigPatternUpdator
  if (InDetTrigFlags.doPrintConfigurables()):
     print (     InDetTrigPatternUpdator)


#
# TRT segment minimum number of drift circles tool
#
from InDetConfig.InDetTrackSelectorToolConfig import (InDetTrigTRTDriftCircleCutToolCfg)
InDetTrigTRTDriftCircleCut = CAtoLegacyPublicToolWrapper(InDetTrigTRTDriftCircleCutToolCfg)

from InDetConfig.InDetAmbiTrackSelectionToolConfig import InDetTrigAmbiTrackSelectionToolCfg
InDetTrigAmbiTrackSelectionTool = CAtoLegacyPublicToolWrapper(InDetTrigAmbiTrackSelectionToolCfg)


# --- set Data/MC flag
isMC = False
if globalflags.DataSource == "geant4" :
    isMC = True

# Calibration DB Service
from TRT_ConditionsServices.TRT_ConditionsServicesConf import TRT_CalDbTool
InDetTRTCalDbTool = TRT_CalDbTool(name = "TRT_CalDbTool")


  
