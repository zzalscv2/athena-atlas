#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import Format
from AthenaCommon.Logging import logging

class InDetCacheNames(object):
  Pixel_ClusterKey   = "PixelTrigClustersCache"
  SCT_ClusterKey     = "SCT_ClustersCache"
  SpacePointCachePix = "PixelSpacePointCache"
  SpacePointCacheSCT = "SctSpacePointCache"
  SCTBSErrCacheKey   = "SctBSErrCache"
  SCTFlaggedCondCacheKey = "SctFlaggedCondCache"
  SCTRDOCacheKey     = "SctRDOCache"
  PixRDOCacheKey     = "PixRDOCache"
  PixBSErrCacheKey   = "PixBSErrCache"
  TRTRDOCacheKey     = "TrtRDOCache"
  TRT_DriftCircleCacheKey = "TRT_DriftCircleCache"

def InDetIDCCacheCreatorCfg(flags):
  #Create IdentifiableCaches
  acc = ComponentAccumulator()
  InDet__CacheCreator=CompFactory.getComp("InDet::CacheCreator")
  InDetCacheCreatorTrig = InDet__CacheCreator(name = "InDetCacheCreatorTrig",
                                              TRT_DriftCircleKey = InDetCacheNames.TRT_DriftCircleCacheKey,
                                              Pixel_ClusterKey   = InDetCacheNames.Pixel_ClusterKey,
                                              SCT_ClusterKey     = InDetCacheNames.SCT_ClusterKey,
                                              SpacePointCachePix = InDetCacheNames.SpacePointCachePix,
                                              SpacePointCacheSCT = InDetCacheNames.SpacePointCacheSCT,
                                              SCTRDOCacheKey     = InDetCacheNames.SCTRDOCacheKey,
                                              SCTBSErrCacheKey   = InDetCacheNames.SCTBSErrCacheKey,
                                              SCTFlaggedCondCacheKey = InDetCacheNames.SCTFlaggedCondCacheKey,
                                              PixRDOCacheKey     = InDetCacheNames.PixRDOCacheKey,
                                              PixBSErrCacheKey   = InDetCacheNames.PixBSErrCacheKey,
                                              TRTRDOCacheKey     = InDetCacheNames.TRTRDOCacheKey)
  if not flags.Detector.GeometryTRT:
    InDetCacheCreatorTrig.disableTRT = True

  acc.addEventAlgo( InDetCacheCreatorTrig )
  return acc

def _trackConverterCfg(flags, signature, inputTracksKey, outputTrackParticleKey):
  acc = ComponentAccumulator()

  from TrkConfig.TrkParticleCreatorConfig import InDetTrigParticleCreatorToolFTFCfg
  creatorTool = acc.popToolsAndMerge(InDetTrigParticleCreatorToolFTFCfg(flags))
  acc.addPublicTool(creatorTool)

  trackParticleCnv=CompFactory.InDet.TrigTrackingxAODCnvMT(name = "InDetTrigTrackParticleCreatorAlg" + signature,
                                                          TrackName           = inputTracksKey,
                                                          TrackParticlesName  = outputTrackParticleKey,
                                                          ParticleCreatorTool = creatorTool)
  acc.addEventAlgo(trackParticleCnv, primary=True)

  return acc

def trackFTFConverterCfg(flags, signature):
  return _trackConverterCfg(flags, signature,
                            flags.Tracking.ActiveConfig.trkTracks_FTF,
                            flags.Tracking.ActiveConfig.tracks_FTF)


def trigInDetFastTrackingCfg( inflags, roisKey="EMRoIs", signatureName='', in_view=True ):

  log = logging.getLogger("trigInDetFastTrackingCfg")
  from TrigInDetConfig.utils import getFlagsForActiveConfig
  flags = getFlagsForActiveConfig(inflags, signatureName, log)

  if flags.Detector.GeometryITk:
    from TrigInDetConfig.TrigInDetConfigITk import ITktrigInDetFastTrackingCfg
    return  ITktrigInDetFastTrackingCfg( flags, roisKey, signatureName, in_view )
 
  """ Generates precision fast tracking config, it is a primary config function """

  from TrigInDetConfig.InDetTrigSequence import InDetTrigSequence
  seq = InDetTrigSequence(flags, 
                          flags.Tracking.ActiveConfig.input_name, 
                          rois   = roisKey,
                          inView = "VDVInDetFTF" if in_view else None)
  acc = seq.sequence("FastTrackFinder")

  return acc


def trigInDetLRTCfg(flags, LRTInputCollection, roisKey, in_view, extra_view_inputs=[]):
  from TrigInDetConfig.InDetTrigSequence import InDetTrigSequence
  viewname = "VDVInDetLRT" if in_view else None
  seq = InDetTrigSequence(flags,
                          flags.Tracking.ActiveConfig.input_name,
                          rois   = roisKey,
                          inView = viewname)
  acc = ComponentAccumulator()
  if in_view:
    bserr_inputs = []
    if flags.Input.Format is Format.BS:
      bserr_inputs += [
        ('IDCInDetBSErrContainer' , 'PixelByteStreamErrs'),
        ('IDCInDetBSErrContainer',  'SCT_ByteStreamErrs'),
      ]
    acc.addEventAlgo( CompFactory.AthViews.ViewDataVerifier(
      name = viewname + "_" + flags.Tracking.ActiveConfig.input_name,
      DataObjects = [
        ( 'TrigRoiDescriptorCollection' ,  f'StoreGateSvc+{roisKey}' ),
        ( 'TrackCollection' ,               'StoreGateSvc+HLT_IDTrkTrack_FS_FTF' ),
        ( 'SpacePointContainer' ,           'StoreGateSvc+SCT_TrigSpacePoints' ),
        ( 'InDet::PixelClusterContainer' ,  'StoreGateSvc+PixelTrigClusters' ),
        ( 'InDet::SCT_ClusterContainer' ,   'StoreGateSvc+SCT_TrigClusters' ),
      ]+bserr_inputs+extra_view_inputs
    ) )
  acc.merge(seq.fastTrackFinder(inputTracksName = LRTInputCollection))
  return acc



############################################################################################################################
# precision tracking
############################################################################################################################
prefix="InDetTrigMT"

def TRTDataProviderCfg(flags : AthConfigFlags, rois : str, signatureName : str = None):
  if signatureName is None:
    suffix = flags.Tracking.ActiveConfig.input_name
  else:
    suffix = signatureName
    
  rodDecoderName =  f"TrigTRTRodDecoder{suffix}"
  providerToolName = f"TrigTRTRawDataProviderTool{suffix}"
  providerName = f"TrigTRTRawDataProvider{suffix}"
    
  acc = ComponentAccumulator()
  rodDecoder = CompFactory.TRT_RodDecoder(name = rodDecoderName,
                                          LoadCompressTableDB=True)
  acc.addPublicTool( rodDecoder )
  
  dataProviderTool = CompFactory.TRTRawDataProviderTool(name = providerToolName,
                                                        Decoder=rodDecoder)
  acc.addPublicTool( dataProviderTool )
  
  from .InDetTrigCollectionKeys import TrigTRTKeys
  from RegionSelector.RegSelToolConfig import regSelTool_TRT_Cfg
  dataProviderAlg = CompFactory.TRTRawDataProvider(name = providerName,
                                                   RDOKey       = TrigTRTKeys.RDOs,
                                                   RDOCacheKey       = InDetCacheNames.TRTRDOCacheKey,
                                                   ProviderTool = dataProviderTool,
                                                   RegSelTool   = acc.popToolsAndMerge( regSelTool_TRT_Cfg(flags)),
                                                   isRoI_Seeded = True,
                                                   RoIs         = rois )
  acc.addEventAlgo(dataProviderAlg)
  return acc


def TRTExtensionBuilderCfg(flags, rois):
  acc = ComponentAccumulator()
  if flags.Input.Format is Format.BS:
    acc.merge( TRTDataProviderCfg(flags, rois) )

  from InDetConfig.InDetPrepRawDataFormationConfig import TrigTRTRIOMakerCfg
  acc.merge( TrigTRTRIOMakerCfg(flags) )

  from InDetConfig.TRT_TrackExtensionAlgConfig import Trig_TRT_TrackExtensionAlgCfg
  acc.merge( Trig_TRT_TrackExtensionAlgCfg(flags, inputTracks = flags.Tracking.ActiveConfig.trkTracks_IDTrig+"_Amb") )

  from InDetConfig.InDetExtensionProcessorConfig import TrigInDetExtensionProcessorCfg
  acc.merge( TrigInDetExtensionProcessorCfg(flags) )
#  'TRTRawDataProvider/InDetTrigMTTRTRawDataProvider_electronLRT', 
#  'InDet::TRT_RIO_Maker/InDetTrigMTTRTDriftCircleMaker_electronLRT', 
#  'InDet::TRT_TrackExtensionAlg/InDetTrigMTTrackExtensionAlg_electronLRT', 
#  'InDet::InDetExtensionProcessor/InDetTrigMTExtensionProcessor_electronLRT', 

  return acc

def ambiguitySolverAlgCfg(flags):
  acc = ComponentAccumulator()

  from TrkConfig.TrkAmbiguitySolverConfig import TrkAmbiguityScore_Trig_Cfg, TrkAmbiguitySolver_Trig_Cfg
  acc.merge(TrkAmbiguityScore_Trig_Cfg(flags, name = f"{prefix}TrkAmbiguityScore_{flags.Tracking.ActiveConfig.input_name}"))
  acc.merge(TrkAmbiguitySolver_Trig_Cfg(flags, name = f"{prefix}TrkAmbiguitySolver_{flags.Tracking.ActiveConfig.input_name}"))

  return acc

def trackEFIDConverterCfg(flags):
  return _trackConverterCfg(flags, 
                            "_Precision"+flags.Tracking.ActiveConfig.name,
                            flags.Tracking.ActiveConfig.trkTracks_IDTrig,
                            flags.Tracking.ActiveConfig.tracks_IDTrig)


def trigInDetPrecisionTrackingCfg( inflags, rois, signatureName, in_view=True ):
  if inflags.Detector.GeometryITk:
    from TrigInDetConfig.TrigInDetConfigITk import ITktrigInDetPrecisionTrackingCfg
    return  ITktrigInDetPrecisionTrackingCfg(inflags, signatureName, in_view=True)
 
  """ Generates precision tracking config, it is a primary config function """

  acc = ComponentAccumulator()
  log = logging.getLogger("trigInDetPrecisionTrackingCfg")
  from TrigInDetConfig.utils import getFlagsForActiveConfig
  flags = getFlagsForActiveConfig(inflags, signatureName, log)

  from .InDetTrigCollectionKeys import TrigPixelKeys
  if in_view:
    #TODO share setup with FTF
    verifier = CompFactory.AthViews.ViewDataVerifier( name = 'VDVInDetPrecision'+flags.Tracking.ActiveConfig.suffix,
                                                      DataObjects= [('xAOD::EventInfo', 'StoreGateSvc+EventInfo'),
                                                                    ('InDet::PixelClusterContainerCache', InDetCacheNames.Pixel_ClusterKey),
                                                                    ('PixelRDO_Cache', InDetCacheNames.PixRDOCacheKey),
                                                                    ('InDet::TRT_DriftCircleContainerCache', InDetCacheNames.TRT_DriftCircleCacheKey),
                                                                    ('SCT_RDO_Cache', InDetCacheNames.SCTRDOCacheKey),
                                                                    ('TRT_RDO_Cache', InDetCacheNames.TRTRDOCacheKey),
                                                                    ('SpacePointCache', InDetCacheNames.SpacePointCachePix),
                                                                    ('SpacePointCache', InDetCacheNames.SpacePointCacheSCT),
                                                                    ('TrigRoiDescriptorCollection', flags.Tracking.ActiveConfig.roi),
                                                                    ( 'TagInfo', 'DetectorStore+ProcessingTags' ), 
                                                                    ( 'InDet::PixelGangedClusterAmbiguities' , TrigPixelKeys.PixelClusterAmbiguitiesMap),
                                                                    ( 'TrackCollection', flags.Tracking.ActiveConfig.trkTracks_FTF )] )

    if flags.Input.Format is Format.BS:
        verifier.DataObjects += [ ('IDCInDetBSErrContainer' , 'PixelByteStreamErrs'),
                                  ('IDCInDetBSErrContainer_Cache', 'SctBSErrCache'),
                                  ('IDCInDetBSErrContainer_Cache', 'SctFlaggedCondCache'), ]
    acc.addEventAlgo(verifier)

  acc.merge(ambiguitySolverAlgCfg(flags))
  acc.merge(TRTExtensionBuilderCfg(flags, rois))
  acc.merge(trackEFIDConverterCfg(flags))

#   Members = ['Trk::TrkAmbiguityScore/InDetTrigMTTrkAmbiguityScore_electronLRT', 
#  'Trk::TrkAmbiguitySolver/InDetTrigMTTrkAmbiguitySolver_electronLRT', 
#  'xAODMaker::TrackParticleCnvAlg/InDetTrigMTxAODParticleCreatorAlgelectronLRT_IDTrig']

  return acc

def trigInDetVertexingCfg(flags, inputTracks, outputVtx):
  
  acc = ComponentAccumulator()

  from InDetConfig.InDetPriVxFinderConfig import InDetTrigPriVxFinderCfg
  acc.merge(InDetTrigPriVxFinderCfg(flags, inputTracks = inputTracks, outputVtx =outputVtx))

  return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    ComponentAccumulator.debugMode = "trackCA trackPublicTool trackEventAlgo trackCondAlgo trackPrivateTool"
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.lock()
    # this configuration is not runable, the test checks if there is no mistake in python scripts above
    # output can be used by experts to check actual configuration (e.g. here we configure to run on RAW and it should be reflected in settings)
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg( flags )
    roisKey = "ElectronRoIs"
    
    flags = flags.cloneAndReplace("Tracking.ActiveConfig", "Trigger.InDetTracking.electron")
    acc.merge( trigInDetFastTrackingCfg( flags, roisKey=roisKey, signatureName="electron" ) )
    acc.merge( trigInDetPrecisionTrackingCfg( flags, rois=roisKey, signatureName="electron", in_view=True) )
    acc.merge( trigInDetVertexingCfg( flags, inputTracks=flags.Tracking.ActiveConfig.tracks_FTF, outputVtx="testVtx") )


    acc.printConfig(withDetails=True, summariseProps=True)
    acc.store( open("test.pkl", "wb") )
