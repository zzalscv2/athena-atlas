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
    self.__inView = inView
    #self.__cas = list()
    
    
  def sequence(self, recoType : str = "FastTrackFinder") -> ComponentAccumulator:
    ca = ComponentAccumulator()
    
    if self.__inView:
      ca.merge(self.viewDataVerifier(self.__inView))

    ca.merge(self.dataPreparation())
    if recoType == "dataPreparation":
      return ca
    
    ca.merge(self.spacePointFormation())
    if recoType =="spacePointFormation":
      return ca

    ca.merge(self.fastTrackFinder())
    if recoType =="FastTrackFinder":
      return ca

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

  def dataPreparation(self) -> ComponentAccumulator:
    
    signature = self.__flags.InDet.Tracking.ActiveConfig.input_name
    
    with ConfigurableCABehavior():
      acc = ComponentAccumulator()

      if self.__flags.Input.Format == Format.BS:
        from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConfig import TrigPixelRawDataProviderAlgCfg
        acc.merge(TrigPixelRawDataProviderAlgCfg(self.__flags,suffix=signature,RoIs=self.__rois))

        from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConfig import TrigSCTRawDataProviderCfg
        acc.merge(TrigSCTRawDataProviderCfg(self.__flags,suffix=signature,RoIs=self.__rois))
      elif self.__inView:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        loadRDOs = [( 'PixelRDO_Container' , 'StoreGateSvc+PixelRDOs' ),
                    ( 'SCT_RDO_Container' , 'StoreGateSvc+SCT_RDOs' ) ]
        acc.merge(SGInputLoaderCfg(self.__flags, Load=loadRDOs))

      #Clusterisation
      from InDetConfig.InDetPrepRawDataFormationConfig import TrigPixelClusterizationCfg
      acc.merge(TrigPixelClusterizationCfg(self.__flags,name="InDetPixelClusterization" + signature, RoIs=self.__rois))
      
      from InDetConfig.InDetPrepRawDataFormationConfig import TrigSCTClusterizationCfg
      acc.merge(TrigSCTClusterizationCfg(self.__flags, name="InDetSCTClusterization" + signature, RoIs = self.__rois))

      return acc
  
  def spacePointFormation(self) -> ComponentAccumulator:
    
    signature = self.__flags.InDet.Tracking.ActiveConfig.input_name
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
      
      signature = flags.InDet.Tracking.ActiveConfig.input_name

      if extraFlags:
        flags = extraFlags
        ftfargs["inputTracksName"] = self.__flags.InDet.Tracking.ActiveConfig.trkTracks_FTF
        #TODO move from .name to .input_name for consistency after migration to private tools
        signature = flags.InDet.Tracking.ActiveConfig.name
      elif inputTracksName:
        ftfargs["inputTracksName"] = inputTracksName
    
      from TrigFastTrackFinder.TrigFastTrackFinderConfig import TrigFastTrackFinderCfg
      acc.merge(TrigFastTrackFinderCfg(flags, "TrigFastTrackFinder_" + signature, 
                                       signature, self.__rois, **ftfargs))
      
      if not flags.InDet.Tracking.ActiveConfig.doZFinderOnly:
        from TrigInDetConfig.TrigInDetConfig import trackFTFConverterCfg
        acc.merge(trackFTFConverterCfg(flags, signature))

      return acc
               
  
  
