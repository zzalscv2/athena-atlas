#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format
from InDetRecExample.InDetKeys import InDetKeys

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

def InDetIDCCacheCreatorCfg(flags):
  #Create IdentifiableCaches
  acc = ComponentAccumulator()
  InDet__CacheCreator=CompFactory.getComp("InDet::CacheCreator")
  InDetCacheCreatorTrig = InDet__CacheCreator(name = "InDetCacheCreatorTrig",
                                              Pixel_ClusterKey   = InDetCacheNames.Pixel_ClusterKey,
                                              SCT_ClusterKey     = InDetCacheNames.SCT_ClusterKey,
                                              SpacePointCachePix = InDetCacheNames.SpacePointCachePix,
                                              SpacePointCacheSCT = InDetCacheNames.SpacePointCacheSCT,
                                              SCTRDOCacheKey     = InDetCacheNames.SCTRDOCacheKey,
                                              SCTBSErrCacheKey   = InDetCacheNames.SCTBSErrCacheKey,
                                              SCTFlaggedCondCacheKey = InDetCacheNames.SCTFlaggedCondCacheKey,
                                              PixRDOCacheKey     = InDetCacheNames.PixRDOCacheKey,
                                              PixBSErrCacheKey   = InDetCacheNames.PixBSErrCacheKey)
  if not flags.Detector.GeometryTRT:
    InDetCacheCreatorTrig.disableTRT = True

  acc.addEventAlgo( InDetCacheCreatorTrig )
  return acc

def trtCondCfg(flags):
  acc = ComponentAccumulator()
  from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline, addFolders
  #TODO switch to use config from TRT_ConditionsConfig
  if flags.Common.isOnline:
    acc.merge(addFolders(flags, "/TRT/Onl/ROD/Compress","TRT_ONL", className='CondAttrListCollection'))
  acc.merge(addFoldersSplitOnline(flags, "TRT","/TRT/Onl/Calib/RT","/TRT/Calib/RT",className="TRTCond::RtRelationMultChanContainer"))
  acc.merge(addFoldersSplitOnline(flags, "TRT","/TRT/Onl/Calib/T0","/TRT/Calib/T0",className="TRTCond::StrawT0MultChanContainer"))
  acc.merge(addFoldersSplitOnline (flags, "TRT","/TRT/Onl/Calib/errors","/TRT/Calib/errors",className="TRTCond::RtRelationMultChanContainer"))

  return acc


def pixelDataPrepCfg(flags, roisKey, signature):
  from PixelReadoutGeometry.PixelReadoutGeometryConfig import PixelReadoutManagerCfg
  acc = PixelReadoutManagerCfg(flags)

  from RegionSelector.RegSelToolConfig import regSelTool_Pixel_Cfg
  RegSelTool_Pixel = acc.popToolsAndMerge(regSelTool_Pixel_Cfg(flags))

  from PixelConditionsAlgorithms.PixelConditionsConfig import PixelCablingCondAlgCfg
  acc.merge(PixelCablingCondAlgCfg(flags)) # To produce PixelCablingCondData for PixelRodDecoder + PixelRawDataProvider

  if flags.Input.Format is Format.BS:
    from PixelConditionsAlgorithms.PixelConditionsConfig import PixelHitDiscCnfgAlgCfg
    acc.merge(PixelHitDiscCnfgAlgCfg(flags)) # To produce PixelHitDiscCnfgData for PixelRodDecoder
    PixelRodDecoder=CompFactory.PixelRodDecoder
    InDetPixelRodDecoder = PixelRodDecoder(name = "InDetPixelRodDecoder"+ signature)
    # Disable duplcated pixel check for data15 because duplication mechanism was used.
    if len(flags.Input.ProjectName)>=6 and flags.Input.ProjectName[:6]=="data15":
      InDetPixelRodDecoder.CheckDuplicatedPixel=False
    acc.addPublicTool(InDetPixelRodDecoder)

    PixelRawDataProviderTool=CompFactory.PixelRawDataProviderTool
    InDetPixelRawDataProviderTool = PixelRawDataProviderTool(name    = "InDetPixelRawDataProviderTool"+ signature,
                                                             Decoder = InDetPixelRodDecoder)
    acc.addPublicTool(InDetPixelRawDataProviderTool)

    # load the PixelRawDataProvider
    PixelRawDataProvider=CompFactory.PixelRawDataProvider
    InDetPixelRawDataProvider = PixelRawDataProvider(name         = "InDetPixelRawDataProvider"+ signature,
                                                     RDOKey       = InDetKeys.PixelRDOs(),
                                                     ProviderTool = InDetPixelRawDataProviderTool,
                                                     isRoI_Seeded = True,
                                                     RoIs         = roisKey,
                                                     RDOCacheKey  = InDetCacheNames.PixRDOCacheKey,
                                                     BSErrorsCacheKey = InDetCacheNames.PixBSErrCacheKey,
                                                     RegSelTool   = RegSelTool_Pixel)

    acc.addEventAlgo(InDetPixelRawDataProvider)

  return acc

def sctDataPrepCfg(flags, roisKey, signature):
  acc = ComponentAccumulator()

  from RegionSelector.RegSelToolConfig import regSelTool_SCT_Cfg
  RegSelTool_SCT   = acc.popToolsAndMerge(regSelTool_SCT_Cfg(flags))

  # load the SCTRawDataProvider

  if flags.Input.Format is Format.BS:
    from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConfig import SCTRawDataProviderCfg
    sctProviderArgs = {}
    sctProviderArgs["RDOKey"] = InDetKeys.SCT_RDOs()
    sctProviderArgs["isRoI_Seeded"] = True
    sctProviderArgs["RoIs"] = roisKey
    sctProviderArgs["RDOCacheKey"] = InDetCacheNames.SCTRDOCacheKey
    sctProviderArgs["BSErrCacheKey"] = InDetCacheNames.SCTBSErrCacheKey
    sctProviderArgs["RegSelTool"] = RegSelTool_SCT
    acc.merge(SCTRawDataProviderCfg(flags, suffix=signature, **sctProviderArgs))
    # load the SCTEventFlagWriter
    from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConfig import SCTEventFlagWriterCfg
    acc.merge(SCTEventFlagWriterCfg(flags, suffix=signature))

  return acc




def trtDataPrep(flags, roisKey, signature):
  acc = ComponentAccumulator()

  acc.merge(trtCondCfg(flags))

  from RegionSelector.RegSelToolConfig import regSelTool_TRT_Cfg
  RegSelTool_TRT = acc.popToolsAndMerge(regSelTool_TRT_Cfg(flags))

  if flags.Input.Format is Format.BS:
    TRT_RodDecoder=CompFactory.TRT_RodDecoder
    InDetTRTRodDecoder = TRT_RodDecoder(name = "InDetTRTRodDecoder")
    if flags.Input.isMC:
      InDetTRTRodDecoder.LoadCompressTableDB = False
      InDetTRTRodDecoder.keyName=""
    acc.addPublicTool(InDetTRTRodDecoder)

    TRTRawDataProviderTool=CompFactory.TRTRawDataProviderTool
    InDetTRTRawDataProviderTool = TRTRawDataProviderTool(name    = "InDetTRTRawDataProviderTool"+ signature,
                                                         Decoder = InDetTRTRodDecoder)
    acc.addPublicTool(InDetTRTRawDataProviderTool)

     # load the TRTRawDataProvider
    from .InDetTrigCollectionKeys import TrigTRTKeys
    TRTRawDataProvider=CompFactory.TRTRawDataProvider
    InDetTRTRawDataProvider = TRTRawDataProvider(name         = "InDetTRTRawDataProvider"+ signature,
                                                 RDOKey       = TrigTRTKeys.RDOs,
                                                 ProviderTool = InDetTRTRawDataProviderTool,
                                                 RegSelTool   = RegSelTool_TRT,
                                                 isRoI_Seeded = True,
                                                 RoIs         = roisKey)

    acc.addEventAlgo(InDetTRTRawDataProvider)

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
  return _trackConverterCfg(flags, signature, flags.InDet.Tracking.ActiveConfig.trkTracks_FTF, flags.InDet.Tracking.ActiveConfig.tracks_FTF)


def trigInDetFastTrackingCfg( inflags, roisKey="EMRoIs", signatureName='', in_view=True ):
  if inflags.Detector.GeometryITk:
    from TrigInDetConfig.TrigInDetConfigITk import ITktrigInDetFastTrackingCfg
    return  ITktrigInDetFastTrackingCfg( inflags, roisKey, signatureName, in_view )
 
  """ Generates precision fast tracking config, it is a primary config function """

  # redirect InDet.Tracking.ActiveConfig flags to point to a specific trigger setting

  flags = inflags.cloneAndReplace("InDet.Tracking.ActiveConfig", "Trigger.InDetTracking."+signatureName)

  #If signature specified add suffix to the name of each algorithms
  signature =  ("_" + signatureName if signatureName else '').lower()

  acc = ComponentAccumulator()

  if in_view:
    verifier = CompFactory.AthViews.ViewDataVerifier( name = 'VDVInDetFTF'+signature,
                                                      DataObjects= [('xAOD::EventInfo', 'StoreGateSvc+EventInfo'),
                                                                    ('InDet::PixelClusterContainerCache', InDetCacheNames.Pixel_ClusterKey),
                                                                    ('PixelRDO_Cache', InDetCacheNames.PixRDOCacheKey),
                                                                    ('InDet::SCT_ClusterContainerCache', InDetCacheNames.SCT_ClusterKey),
                                                                    ('SCT_RDO_Cache', InDetCacheNames.SCTRDOCacheKey),
                                                                    ('SpacePointCache', InDetCacheNames.SpacePointCachePix),
                                                                    ('SpacePointCache', InDetCacheNames.SpacePointCacheSCT),
                                                                    ( 'IDCInDetBSErrContainer_Cache' , InDetCacheNames.PixBSErrCacheKey ),
                                                                    ( 'IDCInDetBSErrContainer_Cache' , InDetCacheNames.SCTBSErrCacheKey ),
                                                                    ( 'IDCInDetBSErrContainer_Cache' , InDetCacheNames.SCTFlaggedCondCacheKey ),
                                                                    ('xAOD::EventInfo', 'EventInfo'),
                                                                    ('TrigRoiDescriptorCollection', str(roisKey)),
                                                                    ( 'TagInfo' , 'DetectorStore+ProcessingTags' )] )
    if flags.Input.isMC:
        verifier.DataObjects += [( 'PixelRDO_Container' , 'StoreGateSvc+PixelRDOs' ),
                                  ( 'SCT_RDO_Container' , 'StoreGateSvc+SCT_RDOs' ) ]
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        sgil_load = [( 'PixelRDO_Container' , 'StoreGateSvc+PixelRDOs' ),
                    ( 'SCT_RDO_Container' , 'StoreGateSvc+SCT_RDOs' ) ]
        acc.merge(SGInputLoaderCfg(flags, Load=sgil_load))

    acc.addEventAlgo(verifier)
  #Only add raw data decoders if we're running over raw data
  acc.merge(pixelDataPrepCfg(flags, roisKey, signature))
  acc.merge(sctDataPrepCfg(flags, roisKey, signature))
  acc.merge(trtDataPrep(flags, roisKey, signature))

  from InDetConfig.InDetPrepRawDataFormationConfig import TrigPixelClusterizationCfg, TrigSCTClusterizationCfg
  acc.merge(TrigPixelClusterizationCfg(flags, name="InDetPixelClusterization"+signature, RoIs=roisKey))
  acc.merge(TrigSCTClusterizationCfg(flags, name="InDetSCT_Clusterization"+signature, RoIs=roisKey))

  from InDetConfig.SiSpacePointFormationConfig import TrigSiTrackerSpacePointFinderCfg
  acc.merge(TrigSiTrackerSpacePointFinderCfg(flags, name="InDetSiTrackerSpacePointFinder_"+signature))
  
  #this should not be needed once SiDetElementsRoadMaker_xkCfg invoked internally (causes count changes atm)
  #i.e. remove RoadTool arg from TrigSiTrackMaker
  acc.addCondAlgo(CompFactory.InDet.SiDetElementsRoadCondAlg_xk(name = "SiDetElementsRoadCondAlg_xk"))

  from TrigFastTrackFinder.TrigFastTrackFinderConfig import TrigFastTrackFinderCfg
  acc.merge(TrigFastTrackFinderCfg(flags, "TrigFastTrackFinder_"+signatureName, signatureName, roisKey))
  
  acc.merge(trackFTFConverterCfg(flags, signature))
  return acc

############################################################################################################################
# precision tracking
############################################################################################################################
prefix="InDetTrigMT"

def TRTDataProviderCfg(flags):
  acc = ComponentAccumulator()
  rodDecoder = CompFactory.TRT_RodDecoder(f"{prefix}TRTRodDecoder{flags.InDet.Tracking.ActiveConfig.name}",
                                          LoadCompressTableDB=True)
  acc.addPublicTool( rodDecoder )
  dataProviderTool = CompFactory.TRTRawDataProviderTool(f"{prefix}TRTRawDataProviderTool{flags.InDet.Tracking.ActiveConfig.name}",
                                                    Decoder=rodDecoder)

  acc.addPublicTool( dataProviderTool )
  from .InDetTrigCollectionKeys import TrigTRTKeys
  from RegionSelector.RegSelToolConfig import regSelTool_TRT_Cfg
  dataProviderAlg = CompFactory.TRTRawDataProvider(f"{prefix}TRTRawDataProvider{flags.InDet.Tracking.ActiveConfig.name}",
                                                   RDOKey       = TrigTRTKeys.RDOs,
                                                   ProviderTool = dataProviderTool,
                                                   RegSelTool   = acc.popToolsAndMerge( regSelTool_TRT_Cfg(flags)),
                                                   isRoI_Seeded = True,
                                                   RoIs         = flags.InDet.Tracking.ActiveConfig.roi )
  acc.addEventAlgo(dataProviderAlg)
  return acc


def TRTExtrensionBuilderCfg(flags):
  acc = ComponentAccumulator()
  if flags.Input.Format is Format.BS:
    acc.merge( TRTDataProviderCfg(flags) )

  from InDetConfig.InDetPrepRawDataFormationConfig import TrigTRTRIOMakerCfg
  acc.merge( TrigTRTRIOMakerCfg(flags) )

  from InDetConfig.TRT_TrackExtensionAlgConfig import Trig_TRT_TrackExtensionAlgCfg
  acc.merge( Trig_TRT_TrackExtensionAlgCfg(flags) )

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
  acc.merge(TrkAmbiguityScore_Trig_Cfg(flags, name = f"{prefix}TrkAmbiguityScore_{flags.InDet.Tracking.ActiveConfig.input_name}"))
  acc.merge(TrkAmbiguitySolver_Trig_Cfg(flags, name  = f"{prefix}TrkAmbiguitySolver_{flags.InDet.Tracking.ActiveConfig.input_name}"))

  return acc

def trackEFIDConverterCfg(flags):
  return _trackConverterCfg(flags, "_Precision"+flags.InDet.Tracking.ActiveConfig.name, flags.InDet.Tracking.ActiveConfig.trkTracks_IDTrig, flags.InDet.Tracking.ActiveConfig.tracks_IDTrig)


def trigInDetPrecisionTrackingCfg( inflags, signatureName, in_view=True ):
  if inflags.Detector.GeometryITk:
    from TrigInDetConfig.TrigInDetConfigITk import ITktrigInDetPrecisionTrackingCfg
    return  ITktrigInDetPrecisionTrackingCfg(inflags, signatureName, in_view=True)
 
  """ Generates precision tracking config, it is a primary config function """

  acc = ComponentAccumulator()
  flags = inflags.cloneAndReplace("InDet.Tracking.ActiveConfig", "Trigger.InDetTracking."+signatureName)

  from .InDetTrigCollectionKeys import TrigPixelKeys,TrigTRTKeys
  if in_view:
    #TODO share setup with FTF
    TRT_RDO_Key = "TRT_RDOs"
    if flags.Input.Format is Format.BS:
          TRT_RDO_Key = TrigTRTKeys.RDOs
    verifier = CompFactory.AthViews.ViewDataVerifier( name = 'VDVInDetPrecision'+flags.InDet.Tracking.ActiveConfig.suffix,
                                                      DataObjects= [('xAOD::EventInfo', 'StoreGateSvc+EventInfo'),
                                                                    ('InDet::PixelClusterContainerCache', 'PixelTrigClustersCache'),
                                                                    ('PixelRDO_Cache', 'PixRDOCache'),
                                                                    ('SCT_RDO_Cache', 'SctRDOCache'),
                                                                    ( 'TRT_RDO_Container' , TRT_RDO_Key),
                                                                    ('SpacePointCache', 'PixelSpacePointCache'),
                                                                    ('SpacePointCache', 'SctSpacePointCache'),
                                                                    ('TrigRoiDescriptorCollection', flags.InDet.Tracking.ActiveConfig.roi),
                                                                    ( 'TagInfo', 'DetectorStore+ProcessingTags' ), 
                                                                    ( 'InDet::PixelGangedClusterAmbiguities' , TrigPixelKeys.PixelClusterAmbiguitiesMap),
                                                                    ( 'TrackCollection', flags.InDet.Tracking.ActiveConfig.trkTracks_FTF )] )

    if flags.Input.Format is Format.BS:
        verifier.DataObjects += [ ('IDCInDetBSErrContainer' , 'PixelByteStreamErrs'),
                                  ('IDCInDetBSErrContainer_Cache', 'SctBSErrCache'),
                                  ('IDCInDetBSErrContainer_Cache', 'SctFlaggedCondCache'), ]
    acc.addEventAlgo(verifier)

  acc.merge(ambiguitySolverAlgCfg(flags))
  acc.merge(TRTExtrensionBuilderCfg(flags))
  acc.merge(trackEFIDConverterCfg(flags))

#   Members = ['Trk::TrkAmbiguityScore/InDetTrigMTTrkAmbiguityScore_electronLRT', 
#  'Trk::TrkAmbiguitySolver/InDetTrigMTTrkAmbiguitySolver_electronLRT', 
#  'xAODMaker::TrackParticleCnvAlg/InDetTrigMTxAODParticleCreatorAlgelectronLRT_IDTrig']

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
    acc.merge( trigInDetFastTrackingCfg( flags, roisKey="ElectronRoIs", signatureName="Electron" ) )

    acc.merge( trigInDetPrecisionTrackingCfg( flags, signatureName="Electron" , in_view=True) )


    acc.printConfig(withDetails=True, summariseProps=True)
    acc.store( open("test.pkl", "wb") )
