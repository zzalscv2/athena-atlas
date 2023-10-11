#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from InDetRecExample.InDetKeys import InDetKeys
from TrigInDetConfig.TrigInDetConfig import InDetCacheNames
from AthenaConfiguration.Enums import Format
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaCommon.Configurable import ConfigurableCABehavior
from AthenaCommon.Logging import logging
log = logging.getLogger("InDetTrigSequence")


class InDetTrigSequence:

  def __init__(self,flags : AthConfigFlags, signature : str, rois : str, inView : str):
    self.__flags = flags
    self.__signature = signature
    self.__rois = rois
    self.__lastRois = rois
    self.__inView = inView
    self.__lastTrkCollection = self.__flags.Tracking.ActiveConfig.trkTracks_FTF
    self.__ambiPrefix = "TrigAmbi"
    self.__log = logging.getLogger("InDetTrigSequence")
    self.__log.info(f"signature: {self.__signature} rois: {self.__rois} inview: {self.__inView}")
    
  def sequence(self, recoType : str = "FastTrackFinder") -> ComponentAccumulator:
    with ConfigurableCABehavior():
      ca = ComponentAccumulator()
    
      if self.__inView:
        ca.merge(self.viewDataVerifier(self.__inView))

      if recoType == "dataPreparation":
        ca.merge(self.dataPreparation())
        return ca
    
      if recoType =="spacePointFormation":
        ca.merge(self.dataPreparation())
        ca.merge(self.spacePointFormation())
        return ca

      if recoType =="FastTrackFinder":
        ca.merge(self.dataPreparation())
        ca.merge(self.spacePointFormation())
        ca.merge(self.fastTrackFinder())
        return ca

      if recoType =="Offline":
        ca.merge(self.dataPreparation())
        ca.merge(self.spacePointFormation())
        ca.merge(self.offlinePattern())
        ca.merge(self.sequenceAfterPattern())

      if recoType =="OfflineNoDataPrep":
        ca.merge(self.viewDataVerifierAfterDataPrep())
        ca.merge(self.offlinePattern())
        ca.merge(self.sequenceAfterPattern())
    
      return ca

  def sequenceAfterPattern(self, recoType : str = "PrecisionTracking", rois : str = "") -> ComponentAccumulator:
    with ConfigurableCABehavior():
    
      ca = ComponentAccumulator()

      if rois:
        self.__lastRois = rois
        if self.__lastRois != self.__rois:
          self.__log.info(f"Sequence after patternReco for signature: {self.__signature} RoIs: {self.__rois} inview: {self.__inView} with new RoIs {self.__lastRois} - they must be a subvolume.")
          
      ca.merge(self.ambiguitySolver())

      if self.__flags.Tracking.ActiveConfig.doTRT:
        ca.merge(self.trtExtensions())

      ca.merge(self.xAODParticleCreation())
    
      return ca
    
  def offlinePattern(self) -> ComponentAccumulator:
    with ConfigurableCABehavior():
      ca = ComponentAccumulator()

      from InDetConfig.SiSPSeededTrackFinderConfig import TrigSiSPSeededTrackFinderCfg
      ca.merge(TrigSiSPSeededTrackFinderCfg(self.__flags,
                                            name = 'EFsiSPSeededTrackFinder'+self.__flags.Tracking.ActiveConfig.input_name
      ))

      self.__lastTrkCollection = self.__flags.Tracking.ActiveConfig.trkTracks_IDTrig
      self.__ambiPrefix = "EFAmbi"
      
      return ca
    

  def viewDataVerifier(self, viewVerifier='IDViewDataVerifier') -> ComponentAccumulator:
    
    with ConfigurableCABehavior():
      acc = ComponentAccumulator()
    
      ViewDataVerifier = \
        CompFactory.AthViews.ViewDataVerifier( name = viewVerifier + "_" + self.__signature,
                                               DataObjects = [( 'InDet::PixelClusterContainerCache' , InDetCacheNames.Pixel_ClusterKey ),
                                                              ( 'PixelRDO_Cache' , InDetCacheNames.PixRDOCacheKey ),
                                                              ( 'InDet::SCT_ClusterContainerCache' , InDetCacheNames.SCT_ClusterKey ),
                                                              ( 'SCT_RDO_Cache' , InDetCacheNames.SCTRDOCacheKey ),
                                                              ( 'SpacePointCache' , InDetCacheNames.SpacePointCachePix ),
                                                              ( 'SpacePointCache' , InDetCacheNames.SpacePointCacheSCT ),
                                                              ( 'IDCInDetBSErrContainer_Cache' , InDetCacheNames.PixBSErrCacheKey ),
                                                              ( 'IDCInDetBSErrContainer_Cache' , InDetCacheNames.SCTBSErrCacheKey ),
                                                              ( 'xAOD::EventInfo' , 'StoreGateSvc+EventInfo' ),
                                                              ( 'TagInfo' , 'DetectorStore+ProcessingTags' )]
                                            )

      isByteStream = self.__flags.Input.Format == Format.BS
      if not isByteStream:
        ViewDataVerifier.DataObjects +=   [( 'PixelRDO_Container' , InDetKeys.PixelRDOs() ),
                                           ( 'SCT_RDO_Container' , InDetKeys.SCT_RDOs() )]

      ViewDataVerifier.DataObjects += [( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+%s' % self.__rois )]

      acc.addEventAlgo(ViewDataVerifier)
      return acc

    
  def viewDataVerifierTRT(self, viewVerifier='IDViewDataVerifierTRT') -> ComponentAccumulator:
    
    with ConfigurableCABehavior():
      acc = ComponentAccumulator()
    
      ViewDataVerifier = \
        CompFactory.AthViews.ViewDataVerifier( name = viewVerifier + "_" + self.__signature,
                                               DataObjects = [
                                                 ( 'InDet::TRT_DriftCircleContainerCache' , 'StoreGateSvc+TRT_DriftCircleCache'  ),
                                                 
                                               ]
                                              )
      
      if self.__flags.Input.Format == Format.BS:
        ViewDataVerifier.DataObjects += [( 'TRT_RDO_Cache' , 'StoreGateSvc+TrtRDOCache' )]
      else:        
        ViewDataVerifier.DataObjects += [( 'TRT_RDO_Container' , 'StoreGateSvc+TRT_RDOs' )]

      acc.addEventAlgo(ViewDataVerifier)
      return acc

  def viewDataVerifierAfterDataPrep(self, viewVerifier='IDViewDataVerifierAfterDataPrep') -> ComponentAccumulator:
    with ConfigurableCABehavior():
      
      acc = ComponentAccumulator()
      
      ViewDataVerifier = \
        CompFactory.AthViews.ViewDataVerifier( name = viewVerifier + "_" + self.__signature,
                                               DataObjects = [
                                                 ( 'SpacePointContainer',           'StoreGateSvc+SCT_TrigSpacePoints' ),                                             
                                                 ( 'SpacePointContainer',           'StoreGateSvc+PixelTrigSpacePoints' ),
                                                 ( 'SpacePointOverlapCollection',   'StoreGateSvc+OverlapSpacePoints' ),
                                                 #( 'InDet::PixelGangedClusterAmbiguities' , 'StoreGateSvc+TrigPixelClusterAmbiguitiesMap' ),
                                                 ( 'InDet::SCT_ClusterContainer',   'StoreGateSvc+SCT_TrigClusters' ),
                                                 ( 'InDet::PixelClusterContainer',  'StoreGateSvc+PixelTrigClusters' ),
                                               ]
                                              )
      
      if self.__flags.Input.Format == Format.BS:
        ViewDataVerifier.DataObjects += [
          ( 'IDCInDetBSErrContainer' , 'StoreGateSvc+SCT_ByteStreamErrs' ),
          ( 'IDCInDetBSErrContainer' , 'StoreGateSvc+PixelByteStreamErrs' ),
        ]
      
      acc.addEventAlgo(ViewDataVerifier)
      return acc
    
  def viewDataVerifierAfterPattern(self, viewVerifier='IDViewDataVerifierForAmbi') -> ComponentAccumulator:
    
    with ConfigurableCABehavior():
      
      acc = ComponentAccumulator()

      ViewDataVerifier = \
        CompFactory.AthViews.ViewDataVerifier( name = viewVerifier + "_" + self.__signature,
                                               DataObjects = [
                                                 ( 'InDet::PixelGangedClusterAmbiguities' , 'TrigPixelClusterAmbiguitiesMap'),
                                                 ]
                                              )
      if self.__flags.Input.Format == Format.BS:
        ViewDataVerifier.DataObjects += [
          ( 'IDCInDetBSErrContainer' , 'StoreGateSvc+PixelByteStreamErrs' ),
          ( 'IDCInDetBSErrContainer' , 'StoreGateSvc+SCT_ByteStreamErrs' ),
        ]
      
      acc.addEventAlgo(ViewDataVerifier)
      return acc

  def dataPreparation(self) -> ComponentAccumulator:
    
    signature = self.__flags.Tracking.ActiveConfig.input_name
    
    with ConfigurableCABehavior():
      acc = ComponentAccumulator()

      self.__log.info(f"DataPrep signature: {self.__signature} rois: {self.__rois} inview: {self.__inView}")

      if self.__flags.Input.Format == Format.BS:
        from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConfig import TrigPixelRawDataProviderAlgCfg
        acc.merge(TrigPixelRawDataProviderAlgCfg(self.__flags,suffix=signature,RoIs=self.__rois))

        from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConfig import TrigSCTRawDataProviderCfg
        acc.merge(TrigSCTRawDataProviderCfg(self.__flags,suffix=signature,RoIs=self.__rois))
      elif not self.__inView:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        loadRDOs = [( 'PixelRDO_Container' , 'StoreGateSvc+PixelRDOs' ),
                    ( 'SCT_RDO_Container' , 'StoreGateSvc+SCT_RDOs' ) ]
        acc.merge(SGInputLoaderCfg(self.__flags, Load=loadRDOs))

      #Clusterisation
      from InDetConfig.InDetPrepRawDataFormationConfig import TrigPixelClusterizationCfg
      acc.merge(TrigPixelClusterizationCfg(self.__flags,
                                           self.__rois,
                                           name=f"InDetPixelClusterization_{signature}"))

      
      from InDetConfig.InDetPrepRawDataFormationConfig import TrigSCTClusterizationCfg
      acc.merge(TrigSCTClusterizationCfg(self.__flags,
                                         self.__rois, 
                                         name=f"InDetSCTClusterization_{signature}"))
                                         
      return acc

  def dataPreparationTRT(self) ->ComponentAccumulator:

    with ConfigurableCABehavior():
      acc = ComponentAccumulator()

      acc.merge(self.viewDataVerifierTRT())
      
      if self.__flags.Input.Format == Format.BS:
        from TRT_RawDataByteStreamCnv.TRT_RawDataByteStreamCnvConfig import TrigTRTRawDataProviderCfg
        acc.merge(TrigTRTRawDataProviderCfg(self.__flags, RoIs=self.__lastRois))

      elif not self.__inView:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        loadRDOs = [( 'TRT_RDO_Container' , 'StoreGateSvc+TRT_RDOs' )]
        acc.merge(SGInputLoaderCfg(self.__flags, Load=loadRDOs))

      from InDetConfig.InDetPrepRawDataFormationConfig import TrigTRTRIOMakerCfg
      signature = self.__flags.Tracking.ActiveConfig.input_name
      acc.merge(TrigTRTRIOMakerCfg(self.__flags,
                                   self.__lastRois,
                                   name=f"TrigTRTDriftCircleMaker_{signature}"))
      

      return acc


  def spacePointFormation(self) -> ComponentAccumulator:
    
    signature = self.__flags.Tracking.ActiveConfig.input_name
    with ConfigurableCABehavior():
      acc = ComponentAccumulator()

      from InDetConfig.SiSpacePointFormationConfig import TrigSiTrackerSpacePointFinderCfg
      acc.merge(TrigSiTrackerSpacePointFinderCfg(self.__flags, name="TrigSpacePointFinder"+signature))
      return acc

  def fastTrackFinder(self, extraFlags : AthConfigFlags = None, inputTracksName : str = None) -> ComponentAccumulator:
    """
    return ComponentAccumulator of the FTF instance
    if another instance of flags is passed in this is for a second instance of FTF
    if inputTracksName is specified it is also a second instance but is invoked as first? what about the previous steps? TODO
    """

    with ConfigurableCABehavior():
      acc = ComponentAccumulator()

      ftfargs = {}
      flags = self.__flags
      
      signature = flags.Tracking.ActiveConfig.input_name

      if extraFlags:
        flags = extraFlags
        ftfargs["inputTracksName"] = self.__flags.Tracking.ActiveConfig.trkTracks_FTF
        #TODO move from .name to .input_name for consistency after migration to private tools
        signature = flags.Tracking.ActiveConfig.name
      elif inputTracksName:
        ftfargs["inputTracksName"] = inputTracksName
    
      from TrigFastTrackFinder.TrigFastTrackFinderConfig import TrigFastTrackFinderCfg
      acc.merge(TrigFastTrackFinderCfg(flags, "TrigFastTrackFinder_" + signature, 
                                       signature, self.__rois, **ftfargs))
      
      if not flags.Tracking.ActiveConfig.doZFinderOnly:
        from TrigInDetConfig.TrigInDetConfig import trackFTFConverterCfg
        acc.merge(trackFTFConverterCfg(flags, signature))

      return acc

  def ambiguitySolver(self) -> ComponentAccumulator:
    with ConfigurableCABehavior():
      acc = ComponentAccumulator()

      if self.__inView:
        acc.merge(self.viewDataVerifierAfterPattern())
                
      from TrkConfig.TrkAmbiguitySolverConfig import TrkAmbiguityScore_Trig_Cfg
      acc.merge(
        TrkAmbiguityScore_Trig_Cfg(
          self.__flags,
          name = f"{self.__ambiPrefix}Score_{self.__flags.Tracking.ActiveConfig.input_name}",
          TrackInput = [self.__lastTrkCollection],
          AmbiguityScoreProcessor = None
        )
      )

      from TrkConfig.TrkAmbiguitySolverConfig import TrkAmbiguitySolver_Trig_Cfg
      acc.merge(
        TrkAmbiguitySolver_Trig_Cfg(
          self.__flags,
          name = f"{self.__ambiPrefix}guitySolver_{self.__flags.Tracking.ActiveConfig.input_name}",
        )
      )
    
      self.__lastTrkCollection = self.__flags.Tracking.ActiveConfig.trkTracks_IDTrig+"_Amb"
      return acc

  def trtExtensions(self) -> ComponentAccumulator:
    with ConfigurableCABehavior():
      acc = self.dataPreparationTRT()
      
      from InDetConfig.TRT_TrackExtensionAlgConfig import Trig_TRT_TrackExtensionAlgCfg
      acc.merge(Trig_TRT_TrackExtensionAlgCfg(self.__flags, self.__lastTrkCollection, name="TrigTrackExtensionAlg%s"% self.__signature))

      from InDetConfig.InDetExtensionProcessorConfig import TrigInDetExtensionProcessorCfg
      acc.merge(TrigInDetExtensionProcessorCfg(self.__flags, name="TrigExtensionProcessor%s"% self.__signature))
                                                       
      self.__lastTrkCollection = self.__flags.Tracking.ActiveConfig.trkTracks_IDTrig   

      return acc
    
  def xAODParticleCreation(self) -> ComponentAccumulator:
    with ConfigurableCABehavior():
      if self.__flags.Tracking.ActiveConfig.doTRT:
        acc = self.dataPreparationTRT()
      else:
        acc = ComponentAccumulator()

      from xAODTrackingCnv.xAODTrackingCnvConfig import TrigTrackParticleCnvAlgCfg
      prefix = "InDet" 
      acc.merge(
        TrigTrackParticleCnvAlgCfg(
          self.__flags,
          name = prefix+'xAODParticleCreatorAlg'+self.__flags.Tracking.ActiveConfig.input_name+'_IDTrig', 
          TrackContainerName = self.__lastTrkCollection,
          xAODTrackParticlesFromTracksContainerName = self.__flags.Tracking.ActiveConfig.tracks_IDTrig,
        ))
      return acc
