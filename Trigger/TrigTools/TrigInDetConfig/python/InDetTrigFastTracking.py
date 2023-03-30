#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaCommon.Include import include

from AthenaCommon.Logging import logging
log = logging.getLogger("InDetTrigFastTracking")

include("InDetTrigRecExample/InDetTrigRec_jobOptions.py")

def makeInDetTrigFastTrackingNoView( inflags, config = None, rois = 'EMViewRoIs', doFTF = True, secondStageConfig = None, LRTInputCollection = None ):

  viewAlgs, viewVerify = makeInDetTrigFastTracking( inflags, config, rois, doFTF, None, secondStageConfig, LRTInputCollection)
  return viewAlgs

def makeInDetTrigFastTracking( inflags, config = None, rois = 'EMViewRoIs', doFTF = True, viewVerifier='IDViewDataVerifier', secondStageConfig = None, LRTInputCollection = None):

  if config is None :
    raise ValueError('makeInDetTrigFastTracking() No config provided!')

  #Global keys/names for Trigger collections
  from InDetRecExample.InDetKeys import InDetKeys
  from TrigInDetConfig.TrigInDetConfig import InDetCacheNames
  from AthenaConfiguration.Enums import Format

  from TriggerMenuMT.HLT.Config.MenuComponents import algorithmCAToGlobalWrapper
  from InDetTrigRecExample.InDetTrigCommonTools import CAtoLegacyPublicToolWrapper

  try:
    if inflags.InDet.Tracking.ActiveConfig.input_name == config.input_name:
      log.debug("flags.InDet.Tracking.ActiveConfig is for %s", inflags.InDet.Tracking.ActiveConfig.input_name)
      flags = inflags
    else:
      log.warning("flags.InDet.Tracking.ActiveConfig is not for %s but %s", 
                  config.input_name, inflags.InDet.Tracking.ActiveConfig.input_name)
      raise RuntimeError("makeInDetTrigFastTracking invoked with incorrect flags instance")
  except RuntimeError:
    log.info("Menu code invoked ID config without or with incorrect flags.InDet.Tracking.ActiveConfig for %s", config.input_name)
    flags = inflags.cloneAndReplace("InDet.Tracking.ActiveConfig", "Trigger.InDetTracking."+config.input_name)

  #temporary until imports of public tools via CAtoLegacyPublicToolWrapper not needed anymore  
  from InDetTrigRecExample import InDetTrigCA
  InDetTrigCA.InDetTrigConfigFlags = flags
     
  #Add suffix to the algorithms
  signature =  '_{}'.format( flags.InDet.Tracking.ActiveConfig.input_name )
  
  log.info( "Fast tracking using new configuration: %s %s and suffix %s", flags.InDet.Tracking.ActiveConfig.input_name, flags.InDet.Tracking.ActiveConfig.name, flags.InDet.Tracking.ActiveConfig.suffix )

  # Load RDOs if we aren't loading bytestream                                                                                       
  from AthenaCommon.AlgSequence import AlgSequence
  topSequence = AlgSequence()

  isByteStream = flags.Input.Format == Format.BS
  if not isByteStream:
     topSequence.SGInputLoader.Load += [( 'PixelRDO_Container' , InDetKeys.PixelRDOs() ),
                                        ( 'SCT_RDO_Container' , InDetKeys.SCT_RDOs() )]

  viewAlgs = []

  ViewDataVerifier = None
  if viewVerifier:
    import AthenaCommon.CfgMgr as CfgMgr
    ViewDataVerifier = CfgMgr.AthViews__ViewDataVerifier( viewVerifier + signature )
    ViewDataVerifier.DataObjects = [( 'InDet::PixelClusterContainerCache' , InDetCacheNames.Pixel_ClusterKey ),
                                    ( 'PixelRDO_Cache' , InDetCacheNames.PixRDOCacheKey ),
                                    ( 'InDet::SCT_ClusterContainerCache' , InDetCacheNames.SCT_ClusterKey ),
                                    ( 'SCT_RDO_Cache' , InDetCacheNames.SCTRDOCacheKey ),
                                    ( 'SpacePointCache' , InDetCacheNames.SpacePointCachePix ),
                                    ( 'SpacePointCache' , InDetCacheNames.SpacePointCacheSCT ),
                                    ( 'IDCInDetBSErrContainer_Cache' , InDetCacheNames.PixBSErrCacheKey ),
                                    ( 'IDCInDetBSErrContainer_Cache' , InDetCacheNames.SCTBSErrCacheKey ),
                                    ( 'xAOD::EventInfo' , 'StoreGateSvc+EventInfo' ),
                                    ( 'TagInfo' , 'DetectorStore+ProcessingTags' )]
    if doFTF and flags.InDet.Tracking.ActiveConfig.name == 'fullScanUTT' :
      ViewDataVerifier.DataObjects += [ ( 'DataVector< LVL1::RecJetRoI >' , 'StoreGateSvc+HLT_RecJETRoIs' ) ]

    if not isByteStream:
      ViewDataVerifier.DataObjects +=   [( 'PixelRDO_Container' , InDetKeys.PixelRDOs() ),
                                         ( 'SCT_RDO_Container' , InDetKeys.SCT_RDOs() )]

    viewAlgs.append( ViewDataVerifier )


  from InDetTrigRecExample.InDetTrigFlags import InDetTrigFlags

  #Only add raw data decoders if we're running over raw data
  if isByteStream:

    #Pixel
    from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConfig import TrigPixelRawDataProviderAlgCfg
    pixelRawDataProviderAlg = algorithmCAToGlobalWrapper(TrigPixelRawDataProviderAlgCfg, flags,
                                                         suffix=signature,
                                                         RoIs=rois)
    viewAlgs.extend(pixelRawDataProviderAlg)

    #SCT
    from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConfig import TrigSCTRawDataProviderCfg
    sctRawDataProviderAlg = algorithmCAToGlobalWrapper(TrigSCTRawDataProviderCfg, flags,
                                                       suffix=signature,
                                                       RoIs=rois)
    viewAlgs.extend(sctRawDataProviderAlg)


  #Pixel clusterisation
  from InDetConfig.InDetPrepRawDataFormationConfig import TrigPixelClusterizationCfg
  pixClusterizationAlg = algorithmCAToGlobalWrapper(TrigPixelClusterizationCfg, flags, 
                                                    name="InDetPixelClusterization" + signature, RoIs=rois)
  viewAlgs.extend(pixClusterizationAlg)

  #SCT clusterization 
  from InDetConfig.InDetPrepRawDataFormationConfig import TrigSCTClusterizationCfg
  sctClusterizationAlg = algorithmCAToGlobalWrapper(TrigSCTClusterizationCfg, flags, 
                                                    name="InDetSCTClusterization" + signature, RoIs = rois)
  viewAlgs.extend(sctClusterizationAlg)

  #TBD make Cfg function for MonTools Pix/SCT/SPFinder
  #from InDetPrepRawDataFormation.MonitoringTool import SCT_Clusterization_MonitoringTool
  #InDetSCT_Clusterization.MonTool = SCT_Clusterization_MonitoringTool(flags)
  

  #Space points
  from InDetConfig.SiSpacePointFormationConfig import TrigSiTrackerSpacePointFinderCfg
  spacePointFinderAlg = algorithmCAToGlobalWrapper(TrigSiTrackerSpacePointFinderCfg, flags,
                                                   name="TrigSpacePointFinder"+signature)
                                                   
  viewAlgs.extend(spacePointFinderAlg)

  #FIXME have a flag for now set for True( as most cases call FTF) but potentially separate
  #do not add if the config is LRT
  if doFTF:
    
      from TrkConfig.TrkTrackSummaryToolConfig import InDetTrigTrackSummaryToolCfg
      trackSummaryTool = CAtoLegacyPublicToolWrapper(InDetTrigTrackSummaryToolCfg)

      if config is None:
            raise ValueError('makeInDetTrigFastTracking() No signature config specified')

      if flags.InDet.Tracking.ActiveConfig.useSiSPSeededTrackFinder and "LRT" in flags.InDet.Tracking.ActiveConfig.name:
        # use SiSPSeededTrackFinder for fast tracking
        from InDetRecExample.ConfiguredNewTrackingCuts import ConfiguredNewTrackingCuts
        trackingCuts = ConfiguredNewTrackingCuts( "R3LargeD0" )
        # --- Loading Pixel, SCT conditions
        from AthenaCommon.AlgSequence import AthSequencer
        condSeq = AthSequencer("AthCondSeq")
        if not hasattr(condSeq, "InDetSiDetElementBoundaryLinksPixelCondAlg"):
          from SiCombinatorialTrackFinderTool_xk.SiCombinatorialTrackFinderTool_xkConf import InDet__SiDetElementBoundaryLinksCondAlg_xk
          condSeq += InDet__SiDetElementBoundaryLinksCondAlg_xk(name     = "InDetSiDetElementBoundaryLinksPixelCondAlg",
                                                                ReadKey  = "PixelDetectorElementCollection",
                                                                WriteKey = "PixelDetElementBoundaryLinks_xk",)


        if not hasattr(condSeq, "InDetSiDetElementBoundaryLinksSCTCondAlg"):
          from SiCombinatorialTrackFinderTool_xk.SiCombinatorialTrackFinderTool_xkConf import InDet__SiDetElementBoundaryLinksCondAlg_xk
          condSeq += InDet__SiDetElementBoundaryLinksCondAlg_xk(name     = "InDetSiDetElementBoundaryLinksSCTCondAlg",
                                                                ReadKey  = "SCT_DetectorElementCollection",
                                                                WriteKey = "SCT_DetElementBoundaryLinks_xk")

        from .InDetTrigCommon import siSPSeededTrackFinder_builder, add_prefix
        siSPSeededTrackFinder = siSPSeededTrackFinder_builder( name                  = add_prefix( 'siSPSeededTrackFinder', flags.InDet.Tracking.ActiveConfig.input_name ),
                                                               config                = config,
                                                               outputTracks          = flags.InDet.Tracking.ActiveConfig.trkTracks_FTF(), 
                                                               trackingCuts          = trackingCuts,
                                                               usePrdAssociationTool = False,
                                                               nameSuffix            = flags.InDet.Tracking.ActiveConfig.input_name,
                                                               trackSummaryTool      = trackSummaryTool )


        viewAlgs.append( siSPSeededTrackFinder )

      else:

        from TrigFastTrackFinder.TrigFastTrackFinderConfig import TrigFastTrackFinderCfg
        #TODO: eventually adapt IDTrigConfig also in FTF configuration (pass as additional param)
        theFTF = algorithmCAToGlobalWrapper(TrigFastTrackFinderCfg, flags, "TrigFastTrackFinder_" + signature, 
                                            flags.InDet.Tracking.ActiveConfig.input_name, rois, inputTracksName = LRTInputCollection)
        viewAlgs.extend(theFTF)

      if not flags.InDet.Tracking.ActiveConfig.doZFinderOnly: 
        from TrigInDetConfig.TrigInDetConfig import trackFTFConverterCfg
        xaodcnv = algorithmCAToGlobalWrapper(trackFTFConverterCfg, flags, signature)
        viewAlgs.extend(xaodcnv)

        if secondStageConfig is not None:
          #have been supplied with a second stage config, create another instance of FTF

          inputTracksname = flags.InDet.Tracking.ActiveConfig.trkTracks_FTF   #before ActiveConfig gets replaced -needs restructuring
          myflags2 = flags.cloneAndReplace("InDet.Tracking.ActiveConfig", "Trigger.InDetTracking."+secondStageConfig.name)

          theFTF2 = algorithmCAToGlobalWrapper(TrigFastTrackFinderCfg,myflags2, "TrigFastTrackFinder_" + secondStageConfig.input_name, 
                                               secondStageConfig.input_name, rois, inputTracksName = inputTracksname)
          viewAlgs.extend(theFTF2)

          
          xaodcnv2 = algorithmCAToGlobalWrapper(trackFTFConverterCfg, myflags2, myflags2.InDet.Tracking.ActiveConfig.input_name)
          viewAlgs.extend(xaodcnv2)


      if (InDetTrigFlags.doTruth()):   

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
                                                                          PixelClustesContainer  = 'PixelTrigClusters',
                                                                          SCT_ClustesContainer   = 'SCT_TrigClusters',
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
