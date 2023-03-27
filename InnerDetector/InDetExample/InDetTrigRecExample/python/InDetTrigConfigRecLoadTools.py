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
  # smart ROT creator in case we do the TRT LR in the refit
  #
  if InDetTrigFlags.redoTRT_LR():

    from InDetRecExample.TrackingCommon import getInDetTRT_DriftCircleOnTrackTool
    from TRT_DriftCircleOnTrackTool.TRT_DriftCircleOnTrackToolConf import \
        InDet__TRT_DriftCircleOnTrackUniversalTool
    InDetTrigTRT_RefitRotCreator = \
        InDet__TRT_DriftCircleOnTrackUniversalTool(name  = 'InDetTrigTRT_RefitRotCreator',
                                                   RIOonTrackToolDrift = getInDetTRT_DriftCircleOnTrackTool(), # special settings for trigger needed ?
                                                   ScaleHitUncertainty = 2.5) # fix from Thijs
#    if InDetTrigFlags.doCommissioning():    #introduced for cosmics do not use for collisions
#      InDetTrigTRT_RefitRotCreator.ScaleHitUncertainty = 5.
    ToolSvc += InDetTrigTRT_RefitRotCreator
      
    if (InDetTrigFlags.doPrintConfigurables()):
      print (     InDetTrigTRT_RefitRotCreator)
  
    from TrkRIO_OnTrackCreator.TrkRIO_OnTrackCreatorConf import Trk__RIO_OnTrackCreator
    InDetTrigRefitRotCreator = Trk__RIO_OnTrackCreator(name              = 'InDetTrigRefitRotCreator',
                                                       ToolPixelCluster= InDetTrigPixelClusterOnTrackTool,
                                                       ToolSCT_Cluster     = SCT_ClusterOnTrackTool,
                                                       ToolTRT_DriftCircle = InDetTrigTRT_RefitRotCreator,
                                                       Mode                = 'indet')
    if InDetTrigFlags.useBroadClusterErrors():
      InDetTrigRefitRotCreator.ToolPixelCluster = InDetTrigBroadPixelClusterOnTrackTool
      InDetTrigRefitRotCreator.ToolSCT_Cluster  = InDetTrigBroadSCT_ClusterOnTrackTool

    ToolSvc += InDetTrigRefitRotCreator
    if (InDetTrigFlags.doPrintConfigurables()):
      print (     InDetTrigRefitRotCreator)
         
  else:
    InDetTrigRefitRotCreator = InDetTrigRotCreator

#
# ----------- control loading of the kalman updator
#
if InDetTrigFlags.loadUpdator():
   
  if InDetTrigFlags.kalmanUpdator() == "fast" :
    # fast Kalman updator tool
    from TrkMeasurementUpdator_xk.TrkMeasurementUpdator_xkConf import Trk__KalmanUpdator_xk
    InDetTrigUpdator = Trk__KalmanUpdator_xk(name = 'InDetTrigUpdator')
  elif InDetTrigFlags.kalmanUpdator() == "weight" :
    from TrkMeasurementUpdator.TrkMeasurementUpdatorConf import Trk__KalmanWeightUpdator as ConfiguredWeightUpdator
    InDetTrigUpdator = ConfiguredWeightUpdator(name='InDetTrigUpdator')
  else :
    from TrkMeasurementUpdator.TrkMeasurementUpdatorConf import Trk__KalmanUpdator as ConfiguredKalmanUpdator
    InDetTrigUpdator = ConfiguredKalmanUpdator('InDetTrigUpdator')

  ToolSvc += InDetTrigUpdator
  if (InDetTrigFlags.doPrintConfigurables()):
    print (     InDetTrigUpdator)

#
# ----------- control loading extrapolation
#
if InDetTrigFlags.loadExtrapolator():

  
  # get propagator
  from TrkConfig.TrkExRungeKuttaPropagatorConfig import RungeKuttaPropagatorCfg
  InDetTrigPropagator = CAtoLegacyPublicToolWrapper(RungeKuttaPropagatorCfg,
                                                      name="InDetTrigRKPropagator")
    
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

from InDetRecExample import TrackingCommon

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

#
# ------load association tool from Inner Detector to handle pixel ganged ambiguities
#
if InDetTrigFlags.loadAssoTool():
  from InDetAssociationTools.InDetAssociationToolsConf import InDet__InDetPRD_AssociationToolGangedPixels
  InDetTrigPrdAssociationTool = InDet__InDetPRD_AssociationToolGangedPixels(name = "InDetTrigPrdAssociationTool",
                                                                             PixelClusterAmbiguitiesMapName = "TrigPixelClusterAmbiguitiesMap")
   
  ToolSvc += InDetTrigPrdAssociationTool
  if (InDetTrigFlags.doPrintConfigurables()):
    print (     InDetTrigPrdAssociationTool)

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
  
  #Load inner Pixel layer tool
  InDetTrigTestPixelLayerToolInner = TrackingCommon.getInDetTrigTestPixelLayerToolInner()
  ToolSvc += InDetTrigTestPixelLayerToolInner
  if (InDetTrigFlags.doPrintConfigurables()):
    print ( InDetTrigTestPixelLayerToolInner)

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
  # Configurable version of TrkTrackSummaryTool
  #
  from TrkTrackSummaryTool.TrkTrackSummaryToolConf import Trk__TrackSummaryTool
  InDetTrigTrackSummaryTool = Trk__TrackSummaryTool(name = "InDetTrigTrackSummaryTool",
                                                    InDetSummaryHelperTool = InDetTrigTrackSummaryHelperTool,
                                                    doHolesInDet           = True,
                                                    #this may be temporary #61512 (and used within egamma later)
                                                    )
  ToolSvc += InDetTrigTrackSummaryTool
  if (InDetTrigFlags.doPrintConfigurables()):
     print (     InDetTrigTrackSummaryTool)

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


if InDetTrigFlags.doNewTracking():

  #
  # ------ load new track selector (common for all vertexing algorithms, except for the moment VKalVrt
  #
  from InDetTrigRecExample.ConfiguredVertexingTrigCuts import EFIDVertexingCuts
  from InDetTrackSelectionTool.InDetTrackSelectionToolConf import InDet__InDetTrackSelectionTool

  InDetTrigTrackSelectorTool = \
      InDet__InDetTrackSelectionTool(name = "InDetTrigDetailedTrackSelectorTool",
                                     CutLevel                   =  EFIDVertexingCuts.TrackCutLevel(),
                                     minPt                      =  EFIDVertexingCuts.minPT(),
                                     maxD0			=  EFIDVertexingCuts.IPd0Max(),
                                     maxZ0			=  EFIDVertexingCuts.z0Max(),
                                     maxZ0SinTheta              =  EFIDVertexingCuts.IPz0Max(),
                                     maxSigmaD0 = EFIDVertexingCuts.sigIPd0Max(),
                                     maxSigmaZ0SinTheta = EFIDVertexingCuts.sigIPz0Max(),
                                     # maxChiSqperNdf = EFIDVertexingCuts.fitChi2OnNdfMax(), # Seems not to be implemented?
                                     maxAbsEta = EFIDVertexingCuts.etaMax(),
                                     minNInnermostLayerHits = EFIDVertexingCuts.nHitInnermostLayer(),
                                     minNPixelHits = EFIDVertexingCuts.nHitPix(),
                                     maxNPixelHoles = EFIDVertexingCuts.nHolesPix(),
                                     minNSctHits = EFIDVertexingCuts.nHitSct(),
                                     minNTrtHits = EFIDVertexingCuts.nHitTrt(),
                                     minNSiHits = EFIDVertexingCuts.nHitSi(),
                                     TrackSummaryTool =  InDetTrigTrackSummaryTool,
                                     Extrapolator     = InDetTrigExtrapolator,
                                     #TrtDCCutTool     = InDetTrigTRTDriftCircleCut,
                                     )



  ToolSvc += InDetTrigTrackSelectorTool
  if (InDetTrigFlags.doPrintConfigurables()):
    print (     InDetTrigTrackSelectorTool)


# --- set Data/MC flag
isMC = False
if globalflags.DataSource == "geant4" :
    isMC = True

# Calibration DB Service
from TRT_ConditionsServices.TRT_ConditionsServicesConf import TRT_CalDbTool
InDetTRTCalDbTool = TRT_CalDbTool(name = "TRT_CalDbTool")


# TRT_DriftFunctionTool
from TRT_DriftFunctionTool.TRT_DriftFunctionToolConf import TRT_DriftFunctionTool

InDetTrigTRT_DriftFunctionTool = TRT_DriftFunctionTool(name = "InDetTrigTRT_DriftFunctionTool",
                                                       TRTCalDbTool        = InDetTRTCalDbTool,
                                                       AllowDataMCOverride = True,
                                                       ForceData = True,
                                                       IsMC = isMC)

# Second calibration DB Service in case pile-up and physics hits have different calibrations
if DetFlags.overlay.TRT_on() :

    InDetTrigTRTCalDbTool2 = TRT_CalDbTool(name = "TRT_CalDbSvc2")
    InDetTrigTRTCalDbTool2.RtFolderName = "/TRT/Calib/MC/RT"             
    InDetTrigTRTCalDbTool2.T0FolderName = "/TRT/Calib/MC/T0"             
    InDetTrigTRT_DriftFunctionTool.TRTCalDbTool2 = InDetTrigTRTCalDbTool2
    InDetTrigTRT_DriftFunctionTool.IsOverlay = True
    InDetTrigTRT_DriftFunctionTool.IsMC = False

# --- set HT corrections
InDetTrigTRT_DriftFunctionTool.HTCorrectionBarrelXe = 1.5205
InDetTrigTRT_DriftFunctionTool.HTCorrectionEndcapXe = 1.2712
InDetTrigTRT_DriftFunctionTool.HTCorrectionBarrelAr = 1.5205
InDetTrigTRT_DriftFunctionTool.HTCorrectionEndcapAr = 1.2712
         
# --- set ToT corrections
InDetTrigTRT_DriftFunctionTool.ToTCorrectionsBarrelXe = [0., 4.358121, 3.032195, 1.631892, 0.7408397, -0.004113, -0.613288, -0.73758, -0.623346, -0.561229,-0.29828, -0.21344, -0.322892, -0.386718, -0.534751, -0.874178, -1.231799, -1.503689, -1.896464, -2.385958]
InDetTrigTRT_DriftFunctionTool.ToTCorrectionsEndcapXe = [0., 5.514777, 3.342712, 2.056626, 1.08293693, 0.3907979, -0.082819, -0.457485, -0.599706, -0.427493, -0.328962, -0.403399, -0.663656, -1.029428, -1.46008, -1.919092, -2.151582, -2.285481, -2.036822, -2.15805]
InDetTrigTRT_DriftFunctionTool.ToTCorrectionsBarrelAr = [0., 4.358121, 3.032195, 1.631892, 0.7408397, -0.004113, -0.613288, -0.73758, -0.623346, -0.561229, -0.29828, -0.21344, -0.322892, -0.386718, -0.534751, -0.874178, -1.231799, -1.503689, -1.896464, -2.385958]
InDetTrigTRT_DriftFunctionTool.ToTCorrectionsEndcapAr = [0., 5.514777, 3.342712, 2.056626, 1.08293693, 0.3907979, -0.082819, -0.457485, -0.599706, -0.427493, -0.328962, -0.403399, -0.663656, -1.029428, -1.46008, -1.919092, -2.151582, -2.285481, -2.036822, -2.15805]


ToolSvc += InDetTrigTRT_DriftFunctionTool

from AthenaCommon.GlobalFlags import globalflags

# TRT_DriftCircleTool
import AthenaCommon.SystemOfUnits as Units

MinTrailingEdge = 11.0*Units.ns
MaxDriftTime = 60.0*Units.ns
LowGate         = 14.0625*Units.ns # 4.5*3.125 ns
HighGate        = 42.1875*Units.ns # LowGate + 9*3.125 ns
LowGateArgon         = LowGate
HighGateArgon        = HighGate

if globalflags.DataSource == 'data':
    MinTrailingEdge = 11.0*Units.ns
    MaxDriftTime    = 60.0*Units.ns
    LowGate         = 17.1875*Units.ns
    HighGate        = 45.3125*Units.ns
    LowGateArgon    = 18.75*Units.ns
    HighGateArgon   = 43.75*Units.ns



from TRT_DriftCircleTool.TRT_DriftCircleToolConf import InDet__TRT_DriftCircleTool
InDetTrigTRT_DriftCircleTool = InDet__TRT_DriftCircleTool( name = "InDetTrigTRT_DriftCircleTool",
                                                           TRTDriftFunctionTool = InDetTrigTRT_DriftFunctionTool,
                                                           ConditionsSummaryTool           = InDetTrigTRTStrawStatusSummaryTool,
                                                           UseConditionsStatus  = True,
                                                           UseConditionsHTStatus  = True,
                                                           SimpleOutOfTimePileupSupression = False,
                                                           RejectIfFirstBit                = False, # fixes 50 nsec issue 
                                                           MinTrailingEdge                 = MinTrailingEdge,
                                                           MaxDriftTime                    = MaxDriftTime,
                                                           ValidityGateSuppression         = InDetTrigFlags.InDet25nsec(),
                                                           LowGate = LowGate,
                                                           HighGate = HighGate,
                                                           SimpleOutOfTimePileupSupressionArgon = False,# no OOT rejection for argon
                                                           RejectIfFirstBitArgon                = False, # no OOT rejection for argon
                                                           MinTrailingEdgeArgon                 = MinTrailingEdge,
                                                           MaxDriftTimeArgon                    = MaxDriftTime,
                                                           ValidityGateSuppressionArgon         = InDetTrigFlags.InDet25nsec(),
                                                           LowGateArgon                         = LowGateArgon,
                                                           HighGateArgon                        = HighGateArgon,
                                                           useDriftTimeHTCorrection        = True,
                                                           useDriftTimeToTCorrection       = True, # reenable ToT
                                                           )


ToolSvc += InDetTrigTRT_DriftCircleTool
  
