#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaCommon.Logging import logging
log = logging.getLogger("InDetTrigFastTracking")

def makeInDetTrigFastTrackingNoView( inflags, config = None, rois = 'EMViewRoIs', doFTF = True, secondStageConfig = None, LRTInputCollection = None ):

  viewAlgs, viewVerify = makeInDetTrigFastTracking( inflags, config, rois, doFTF, None, secondStageConfig, LRTInputCollection)
  return viewAlgs

def makeInDetTrigFastTracking( inflags, config = None, rois = 'EMViewRoIs', doFTF = True, viewVerifier='IDViewDataVerifier', secondStageConfig = None, LRTInputCollection = None):

  if config is None :
    raise ValueError('makeInDetTrigFastTracking() No config provided!')

  from TriggerMenuMT.HLT.Config.MenuComponents import extractAlgorithmsAndAppendCA

  from TrigInDetConfig.utils import getFlagsForActiveConfig
  flags = getFlagsForActiveConfig(inflags, config.input_name, log)

  #temporary until imports of public tools via CAtoLegacyPublicToolWrapper not needed anymore  
  from TrigInDetConfig import InDetTrigCA
  InDetTrigCA.InDetTrigConfigFlags = flags
     
  
  log.info( "Fast tracking using new configuration: %s %s and suffix %s",
            flags.Tracking.ActiveConfig.input_name,
            flags.Tracking.ActiveConfig.name,
            flags.Tracking.ActiveConfig.suffix )

  viewAlgs = []

  from TrigInDetConfig.InDetTrigSequence import InDetTrigSequence

  seq = InDetTrigSequence(flags, flags.Tracking.ActiveConfig.input_name, rois = rois, inView = viewVerifier)
  
  ViewDataVerifier = None
  if viewVerifier:
    vdvCA = seq.viewDataVerifier(viewVerifier)           #won't be needed in pure CA
    vdvList = extractAlgorithmsAndAppendCA(vdvCA)
    viewAlgs.extend( vdvList )
    ViewDataVerifier = vdvList[0]

  dataPrep = extractAlgorithmsAndAppendCA(seq.dataPreparation())
  viewAlgs.extend(dataPrep)
  
  #TBD make Cfg function for MonTools Pix/SCT/SPFinder
  #from InDetPrepRawDataFormation.MonitoringTool import SCT_Clusterization_MonitoringTool
  #InDetSCT_Clusterization.MonTool = SCT_Clusterization_MonitoringTool(flags)
  

  #Space points
  spacePointFinderAlg = extractAlgorithmsAndAppendCA(seq.spacePointFormation())
  viewAlgs.extend(spacePointFinderAlg)

  if doFTF:
    
    if LRTInputCollection is None:          #TODO rework cases of second invocation of FTF 
      theFTFCA = seq.fastTrackFinder()
      theFTF = extractAlgorithmsAndAppendCA(theFTFCA)
      viewAlgs.extend(theFTF)
    else:
      theFTFCA2 = seq.fastTrackFinder(inputTracksName = LRTInputCollection)
      theFTF = extractAlgorithmsAndAppendCA(theFTFCA2)
      viewAlgs.extend(theFTF)

      
    if secondStageConfig is not None:
      #TODO move from .name to .input_name for consistency after migration to private tools
      myflags2 = flags.cloneAndReplace("Tracking.ActiveConfig", "Trigger.InDetTracking."+secondStageConfig.name)

      theFTFCA2 = seq.fastTrackFinder(extraFlags = myflags2)
      theFTF = extractAlgorithmsAndAppendCA(theFTFCA2)
      viewAlgs.extend(theFTF)


    if (flags.Tracking.ActiveConfig.doTruth):   
      
      from InDetTruthAlgs.InDetTruthAlgsConf import InDet__PRD_MultiTruthMaker
      InDetTrigPRD_MultiTruthMakerSi = InDet__PRD_MultiTruthMaker (name                    = 'InDetTrigPRD_MultiTruthMakerSi',
                                                               PixelClusterContainerName   = 'PixelTrigClusters',
                                                               SCTClusterContainerName     = 'SCT_TrigClusters',
                                                               TRTDriftCircleContainerName = '',
                                                               SimDataMapNamePixel         = 'PixelSDO_Map',
                                                               SimDataMapNameSCT           = 'SCT_SDO_Map',
                                                               SimDataMapNameTRT           = '',
                                                               TruthNamePixel              = 'PRD_MultiTruthPixel',
                                                               TruthNameSCT                = 'PRD_MultiTruthSCT',
                                                               TruthNameTRT                = '')
      
      viewAlgs.append(InDetTrigPRD_MultiTruthMakerSi)
      MyTrackCollections = ["HLT_IDTrkTrack_FS_FTF"]
      import AthenaCommon.SystemOfUnits as Units
      from InDetTrackClusterAssValidation.InDetTrackClusterAssValidationConf import InDet__TrackClusterAssValidation
      InDetTrigTrackClusterAssValidation = InDet__TrackClusterAssValidation(name              = "InDetTrigTrackClusterAssValidation",
                                                                        TracksLocation         = MyTrackCollections             ,
                                                                        SpacePointsPixelName   = "PixelTrigSpacePoints"    ,
                                                                        SpacePointsSCTName     = "SCT_TrigSpacePoints"    ,
                                                                        SpacePointsOverlapName = "OverlapSpacePoints",
                                                                        PixelClusterContainer  = 'PixelTrigClusters',
                                                                        SCT_ClusterContainer   = 'SCT_TrigClusters',
                                                                        MomentumCut            = 1.5 * Units.GeV,
                                                                        RapidityCut            = 2.7     ,
                                                                        RadiusMin              = 0.0     ,
                                                                        RadiusMax              = 20.0 * Units.mm    ,
                                                                        MinNumberClusters      = 7       ,
                                                                        MinNumberClustersTRT   = 0       ,
                                                                        MinNumberSpacePoints   = 3       ,
                                                                        usePixel               = True     ,
                                                                        useSCT                 = True     ,
                                                                        useTRT                 = False     )
      viewAlgs.append(InDetTrigTrackClusterAssValidation)
 
  return viewAlgs, ViewDataVerifier
