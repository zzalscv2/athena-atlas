#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
#           Setup of offline pattern recognition tracking for ID Trigger
#Heavily inspired by the offline version:
#https://gitlab.cern.ch/atlas/athena/blob/master/InnerDetector/InDetExample/InDetRecExample/share/ConfiguredNewTrackingSiPattern.py

from AthenaCommon.Logging import logging 
log = logging.getLogger("EFIDTracking")

from TriggerMenuMT.HLT.Config.MenuComponents import extractAlgorithmsAndAppendCA
  
#Create a view verifier for necessary data collections
def get_idtrig_view_verifier(name):
   import AthenaCommon.CfgMgr as CfgMgr
   from AthenaCommon.GlobalFlags import globalflags
   from .InDetTrigCollectionKeys import  TrigPixelKeys, TrigSCTKeys
   from InDetRecExample.InDetKeys import InDetKeys
   from TrigInDetConfig.TrigInDetConfig import InDetCacheNames
   viewDataVerifier = CfgMgr.AthViews__ViewDataVerifier( name )
   viewDataVerifier.DataObjects = []

   #Having these (clusters) uncommented breaks cosmic when data preparation is right before offline pattern rec
   #Probably it tries to fetch the data before the actual alg producing them runs?
   #Not case in other signatures where data preparation and offline patern recognition are in different views
   if 'cosmics' not in name:
      viewDataVerifier.DataObjects += [
                                       ( 'SpacePointContainer',           TrigSCTKeys.SpacePoints ),
                                       ( 'SpacePointContainer',           TrigPixelKeys.SpacePoints ),
                                       ( 'SpacePointOverlapCollection',   'StoreGateSvc+OverlapSpacePoints' ),
                                       ( 'InDet::PixelGangedClusterAmbiguities' , 'StoreGateSvc+TrigPixelClusterAmbiguitiesMap' ),
                                       ( 'InDet::SCT_ClusterContainer',   TrigSCTKeys.Clusters ),
                                       ( 'InDet::PixelClusterContainer',  TrigPixelKeys.Clusters ),
                                      ]
      if globalflags.InputFormat.is_bytestream():
         viewDataVerifier.DataObjects += [
                                          ( 'IDCInDetBSErrContainer' , 'StoreGateSvc+SCT_ByteStreamErrs' ),
                                          ( 'IDCInDetBSErrContainer' , 'StoreGateSvc+PixelByteStreamErrs' ),
                                         ]
   
   #FIXME:
   #Align with the data preparation, are all of them  really needed in the EFID ?
   viewDataVerifier.DataObjects += [ ( 'InDet::PixelClusterContainerCache' , InDetCacheNames.Pixel_ClusterKey ),
                                     ( 'PixelRDO_Cache' , InDetCacheNames.PixRDOCacheKey ),
                                     ( 'InDet::SCT_ClusterContainerCache' , InDetCacheNames.SCT_ClusterKey ),
                                     ( 'SCT_RDO_Cache' , InDetCacheNames.SCTRDOCacheKey ),
                                     ( 'SpacePointCache' , InDetCacheNames.SpacePointCachePix ),
                                     ( 'SpacePointCache' , InDetCacheNames.SpacePointCacheSCT ),
                                     ( 'IDCInDetBSErrContainer_Cache' , InDetCacheNames.PixBSErrCacheKey ),
                                     ( 'IDCInDetBSErrContainer_Cache' , InDetCacheNames.SCTBSErrCacheKey ),
                                     ( 'xAOD::EventInfo' , 'StoreGateSvc+EventInfo' ),
                                     ( 'TagInfo' , 'DetectorStore+ProcessingTags' )]

   
  # Load RDOs if we aren't loading bytestream
   from AthenaCommon.AlgSequence import AlgSequence
   topSequence = AlgSequence()
  
   topSequence.SGInputLoader.Load += [ ( 'TagInfo' , 'DetectorStore+ProcessingTags' ) ]
   
   if not globalflags.InputFormat.is_bytestream():
     viewDataVerifier.DataObjects +=   [( 'PixelRDO_Container' , InDetKeys.PixelRDOs() ),
                                        ( 'SCT_RDO_Container' , InDetKeys.SCT_RDOs() ),
                                        ]
   return viewDataVerifier


def makeInDetPatternRecognition( inflags, config, verifier = 'IDTrigViewDataVerifier' ):
      viewAlgs = [] #list of all algs running in this module

      dataVerifier = None
      if verifier:
         dataVerifier = get_idtrig_view_verifier(verifier+config.input_name)
         viewAlgs.append( dataVerifier )


      from TrigInDetConfig.utils import getFlagsForActiveConfig
      flags = getFlagsForActiveConfig(inflags, config.input_name, log)
      
      from TrigInDetConfig import InDetTrigCA
      InDetTrigCA.InDetTrigConfigFlags = flags
      
      from TrigInDetConfig.InDetTrigSequence import InDetTrigSequence
      seq = InDetTrigSequence(flags, flags.Tracking.ActiveConfig.input_name, rois = None, inView = dataVerifier)
      offlineTrackFinder = extractAlgorithmsAndAppendCA(seq.offlinePattern())
      viewAlgs.extend(offlineTrackFinder)

      afterPattern = extractAlgorithmsAndAppendCA(seq.sequenceAfterPattern())
      viewAlgs.extend(afterPattern)
      

      return  viewAlgs, dataVerifier


