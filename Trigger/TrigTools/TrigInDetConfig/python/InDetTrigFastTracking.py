#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaCommon.Include import include

from AthenaCommon.Logging import logging
log = logging.getLogger("InDetTrigFastTracking")

include("InDetTrigRecExample/InDetTrigRec_jobOptions.py")

def makeInDetTrigFastTrackingNoView( flags, config = None, rois = 'EMViewRoIs', doFTF = True, secondStageConfig = None, LRTInputCollection = None ):

  viewAlgs, viewVerify = makeInDetTrigFastTracking( flags, config, rois, doFTF, None, secondStageConfig, LRTInputCollection)
  return viewAlgs

def makeInDetTrigFastTracking( flags, config = None, rois = 'EMViewRoIs', doFTF = True, viewVerifier='IDViewDataVerifier', secondStageConfig = None, LRTInputCollection = None):

  if config is None :
    raise ValueError('makeInDetTrigFastTracking() No config provided!')

  #Global keys/names for Trigger collections
  from .InDetTrigCollectionKeys import  TrigPixelKeys, TrigSCTKeys
  from InDetRecExample.InDetKeys import InDetKeys
  from TrigInDetConfig.TrigInDetConfig import InDetCacheNames
  from AthenaConfiguration.Enums import Format

  from TriggerMenuMT.HLT.Config.MenuComponents import algorithmCAToGlobalWrapper
  from InDetTrigRecExample.InDetTrigCommonTools import CAtoLegacyPublicToolWrapper
  from InDetTrigRecExample import InDetTrigCA
  
  InDetTrigCA.InDetTrigConfigFlags = flags.cloneAndReplace("InDet.Tracking.ActiveConfig", "Trigger.InDetTracking."+config.input_name)
  flags = InDetTrigCA.InDetTrigConfigFlags

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
  from AthenaCommon.AppMgr import ToolSvc

  #Only add raw data decoders if we're running over raw data
  if isByteStream:

    #Pixel
    from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConf import PixelRodDecoder
    InDetPixelRodDecoder = PixelRodDecoder(name = "InDetPixelRodDecoder_" + signature)
    if "data15" in flags.Input.ProjectName:         # Disable for data15 because duplication mechanism was used.
       InDetPixelRodDecoder.CheckDuplicatedPixel=False
    ToolSvc += InDetPixelRodDecoder

    from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConf import PixelRawDataProviderTool
    InDetPixelRawDataProviderTool = PixelRawDataProviderTool(name    = "InDetPixelRawDataProviderTool_" + signature,
                                                             Decoder = InDetPixelRodDecoder)
    ToolSvc += InDetPixelRawDataProviderTool

    if (InDetTrigFlags.doPrintConfigurables()):
      print(InDetPixelRawDataProviderTool) # noqa: ATL901

    # load the PixelRawDataProvider
    from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConf import PixelRawDataProvider
    InDetPixelRawDataProvider = PixelRawDataProvider(name         = "InDetPixelRawDataProvider_"+ signature,
                                                     RDOKey       = InDetKeys.PixelRDOs(),
                                                     ProviderTool = InDetPixelRawDataProviderTool,)
    InDetPixelRawDataProvider.isRoI_Seeded = True
    InDetPixelRawDataProvider.RoIs = rois
    InDetPixelRawDataProvider.RDOCacheKey = InDetCacheNames.PixRDOCacheKey
    InDetPixelRawDataProvider.BSErrorsCacheKey = InDetCacheNames.PixBSErrCacheKey

    from RegionSelector.RegSelToolConfig import makeRegSelTool_Pixel

    InDetPixelRawDataProvider.RegSelTool = makeRegSelTool_Pixel()

    viewAlgs.append(InDetPixelRawDataProvider)


    if (InDetTrigFlags.doPrintConfigurables()):
      print(InDetPixelRawDataProvider) # noqa: ATL901


    #SCT
    from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConf import SCT_RodDecoder
    InDetSCTRodDecoder = SCT_RodDecoder(name          = "InDetSCTRodDecoder_" + signature)
    ToolSvc += InDetSCTRodDecoder

    from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConf import SCTRawDataProviderTool
    InDetSCTRawDataProviderTool = SCTRawDataProviderTool(name    = "InDetSCTRawDataProviderTool_" + signature,
                                                         Decoder = InDetSCTRodDecoder)
    ToolSvc += InDetSCTRawDataProviderTool
    if (InDetTrigFlags.doPrintConfigurables()):
      print(InDetSCTRawDataProviderTool) # noqa: ATL901


    # load the SCTRawDataProvider
    from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConf import SCTRawDataProvider
    InDetSCTRawDataProvider = SCTRawDataProvider(name         = "InDetSCTRawDataProvider_" + signature,
                                                 RDOKey       = InDetKeys.SCT_RDOs(),
                                                 ProviderTool = InDetSCTRawDataProviderTool)
    InDetSCTRawDataProvider.isRoI_Seeded = True
    InDetSCTRawDataProvider.RoIs = rois
    InDetSCTRawDataProvider.RDOCacheKey = InDetCacheNames.SCTRDOCacheKey
    InDetSCTRawDataProvider.BSErrCacheKey = InDetCacheNames.SCTBSErrCacheKey

    from RegionSelector.RegSelToolConfig import makeRegSelTool_SCT
    InDetSCTRawDataProvider.RegSelTool = makeRegSelTool_SCT()

    viewAlgs.append(InDetSCTRawDataProvider)

    # load the SCTEventFlagWriter
    from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConf import SCTEventFlagWriter
    InDetSCTEventFlagWriter = SCTEventFlagWriter(name = "InDetSCTEventFlagWriter_"+ signature)

    viewAlgs.append(InDetSCTEventFlagWriter)

  #Pixel clusterisation
  from InDetConfig.InDetPrepRawDataFormationConfig import TrigPixelClusterizationCfg
  pixClusterizationAlg = algorithmCAToGlobalWrapper(TrigPixelClusterizationCfg, flags, name="InDetPixelClusterization" + signature, RoIs=rois)
  viewAlgs.extend(pixClusterizationAlg)

  #SCT clusterization 
  from InDetConfig.InDetPrepRawDataFormationConfig import TrigSCTClusterizationCfg
  sctClusterizationAlg = algorithmCAToGlobalWrapper(TrigSCTClusterizationCfg, flags, name="InDetSCTClusterization" + signature, RoIs = rois)
  viewAlgs.extend(sctClusterizationAlg)

  #TBD make Cfg function for MonTools
  #from InDetPrepRawDataFormation.MonitoringTool import SCT_Clusterization_MonitoringTool
  #InDetSCT_Clusterization.MonTool = SCT_Clusterization_MonitoringTool(flags)
  

  #Space points and FTF

  from SiSpacePointTool.SiSpacePointToolConf import InDet__SiSpacePointMakerTool
  InDetSiSpacePointMakerTool = InDet__SiSpacePointMakerTool(name = "InDetSiSpacePointMakerTool_" + signature)

  from AthenaCommon.DetFlags import DetFlags
  from SiSpacePointFormation.SiSpacePointFormationConf import InDet__SiTrackerSpacePointFinder
  from SiSpacePointFormation.InDetOnlineMonitor import InDetMonitoringTool
  InDetSiTrackerSpacePointFinder = InDet__SiTrackerSpacePointFinder(name                   = "InDetSiTrackerSpacePointFinder_" + signature,
                                                                    SiSpacePointMakerTool  = InDetSiSpacePointMakerTool,
                                                                    PixelsClustersName     = TrigPixelKeys.Clusters,
                                                                    SpacePointsPixelName   = TrigPixelKeys.SpacePoints,
                                                                    SCT_ClustersName	    = TrigSCTKeys.Clusters,
                                                                    SpacePointsSCTName     = TrigSCTKeys.SpacePoints,
                                                                    SpacePointsOverlapName = InDetKeys.OverlapSpacePoints(),
                                                                    ProcessPixels          = DetFlags.haveRIO.pixel_on(),
                                                                    ProcessSCTs            = DetFlags.haveRIO.SCT_on(),
                                                                    ProcessOverlaps        = DetFlags.haveRIO.SCT_on(),
                                                                    SpacePointCacheSCT     = InDetCacheNames.SpacePointCacheSCT,
                                                                    SpacePointCachePix     = InDetCacheNames.SpacePointCachePix,
                                                                    monTool                = InDetMonitoringTool(flags))

  viewAlgs.append(InDetSiTrackerSpacePointFinder)

  # Condition algorithm for SiTrackerSpacePointFinder
  if InDetSiTrackerSpacePointFinder.ProcessSCTs:
    from AthenaCommon.AlgSequence import AthSequencer
    condSeq = AthSequencer("AthCondSeq")
    if not hasattr(condSeq, "InDetSiElementPropertiesTableCondAlg"):
      # Setup alignment folders and conditions algorithms
      from SiSpacePointFormation.SiSpacePointFormationConf import InDet__SiElementPropertiesTableCondAlg
      condSeq += InDet__SiElementPropertiesTableCondAlg(name = "InDetSiElementPropertiesTableCondAlg")

  #FIXME have a flag for now set for True( as most cases call FTF) but potentially separate
  #do not add if the config is LRT
  if doFTF:
    
      from TrkConfig.TrkTrackSummaryToolConfig import InDetTrigTrackSummaryToolCfg
      trackSummaryTool = CAtoLegacyPublicToolWrapper(InDetTrigTrackSummaryToolCfg)

      from TrkConfig.TrkParticleCreatorConfig import InDetTrigParticleCreatorToolFTFCfg
      InDetTrigParticleCreatorToolFTF = CAtoLegacyPublicToolWrapper(InDetTrigParticleCreatorToolFTFCfg)
      
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

        if not hasattr(condSeq, "InDet__SiDetElementsRoadCondAlg_xk"):
          from SiDetElementsRoadTool_xk.SiDetElementsRoadTool_xkConf import InDet__SiDetElementsRoadCondAlg_xk
          condSeq += InDet__SiDetElementsRoadCondAlg_xk(name = "InDet__SiDetElementsRoadCondAlg_xk")

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

        from TrigFastTrackFinder.TrigFastTrackFinder_Config import TrigFastTrackFinderBase
        #TODO: eventually adapt IDTrigConfig also in FTF configuration (pass as additional param)
        theFTF = TrigFastTrackFinderBase(flags, "TrigFastTrackFinder_" + signature, flags.InDet.Tracking.ActiveConfig.input_name)
        theFTF.RoIs           = rois

        if LRTInputCollection is not None:
          theFTF.inputTracksName = LRTInputCollection

        viewAlgs.append(theFTF)

      if not flags.InDet.Tracking.ActiveConfig.doZFinderOnly: 

        from InDetTrigParticleCreation.InDetTrigParticleCreationConf import InDet__TrigTrackingxAODCnvMT
        theTrackParticleCreatorAlg = InDet__TrigTrackingxAODCnvMT(name = "InDetTrigTrackParticleCreatorAlg" + signature,
                                                                  TrackName = flags.InDet.Tracking.ActiveConfig.trkTracks_FTF,
                                                                  ParticleCreatorTool = InDetTrigParticleCreatorToolFTF)

        #In general all FTF trackParticle collections are recordable except beamspot to save space
        theTrackParticleCreatorAlg.TrackParticlesName = flags.InDet.Tracking.ActiveConfig.tracks_FTF

        viewAlgs.append(theTrackParticleCreatorAlg)

        if secondStageConfig is not None:
          #have been supplied with a second stage config, create another instance of FTF

          inputTracksname = flags.InDet.Tracking.ActiveConfig.trkTracks_FTF   #before ActiveConfig gets replaced -needs restructuring
          flags = flags.cloneAndReplace("InDet.Tracking.ActiveConfig", "Trigger.InDetTracking."+secondStageConfig.name)

          theFTF2 = TrigFastTrackFinderBase(flags, "TrigFastTrackFinder_" + secondStageConfig.input_name, secondStageConfig.input_name)
          theFTF2.RoIs           = rois
          theFTF2.inputTracksName = inputTracksname
        
          
          viewAlgs.append(theFTF2)

          
          from InDetTrigParticleCreation.InDetTrigParticleCreationConf import InDet__TrigTrackingxAODCnvMT
          theTrackParticleCreatorAlg2 = InDet__TrigTrackingxAODCnvMT(name = "InDetTrigTrackParticleCreatorAlg_" + secondStageConfig.input_name,
                                                                  TrackName = secondStageConfig.trkTracks_FTF(),
                                                                     ParticleCreatorTool = InDetTrigParticleCreatorToolFTF)
          
          
          #In general all FTF trackParticle collections are recordable except beamspot to save space
          theTrackParticleCreatorAlg2.TrackParticlesName = secondStageConfig.tracks_FTF()
          
          viewAlgs.append(theTrackParticleCreatorAlg2)


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
