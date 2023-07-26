#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
#           Setup of precision tracking

from AthenaCommon.Logging import logging
log = logging.getLogger("InDetTrigPrecisionTracking")

from TriggerMenuMT.HLT.Config.MenuComponents import algorithmCAToGlobalWrapper

def makeInDetTrigPrecisionTracking( inflags, config=None, verifier=False, rois='EMViewRoIs', prefix="InDetTrigMT" ) :
    
    log.info( "makeInDetTrigPrecisionTracking:: {} {} doTRT: {} ".format(  config.input_name, config.name, config.doTRT ) )

    ptAlgs = [] # List containing all the precision tracking algorithms hence every new added alg has to be appended to the list
    
    # Expects configuration  
    if config is None:
      raise ValueError('PrecisionTracking No configuration provided!')

    from TrigInDetConfig.utils import getFlagsForActiveConfig
    flags = getFlagsForActiveConfig(inflags, config.input_name, log)

    from InDetTrigRecExample import InDetTrigCA
    InDetTrigCA.InDetTrigConfigFlags = flags
    
    doTRT = flags.Tracking.ActiveConfig.doTRT

    # Add suffix to the algorithms
    signature =  "_{}".format( flags.Tracking.ActiveConfig.input_name )

    inputTracks = flags.Tracking.ActiveConfig.trkTracks_FTF 
    # Name settings for output Tracks/TrackParticles
    outTrkTracks        = flags.Tracking.ActiveConfig.trkTracks_IDTrig # Final output Track collection
    outTrackParticles   = flags.Tracking.ActiveConfig.tracks_IDTrig # Final output xAOD::TrackParticle
    ambiTrackCollection = outTrkTracks+"_Amb"  # Ambiguity solver tracks
    
    #  Verifying input data for the algorithms
  
    # If run in views need to check data dependancies!
    # NOTE: this seems necessary only when PT is called from a different view than FTF otherwise causes stalls
    if verifier:
        from .InDetTrigCollectionKeys import TrigPixelKeys
        verifier.DataObjects += [( 'InDet::PixelGangedClusterAmbiguities' , 'StoreGateSvc+' + TrigPixelKeys.PixelClusterAmbiguitiesMap ),
                                 ( 'TrackCollection' , 'StoreGateSvc+' +inputTracks ),
                                 ( 'TRT_RDO_Cache' , 'StoreGateSvc+TrtRDOCache' ),
                                 ( 'InDet::TRT_DriftCircleContainerCache' , 'StoreGateSvc+TRT_DriftCircleCache'  )]

    from TrkConfig.TrkAmbiguitySolverConfig import TrkAmbiguityScore_Trig_Cfg
    ambiguityScore = algorithmCAToGlobalWrapper(
        TrkAmbiguityScore_Trig_Cfg,
        flags,
        name = f"{prefix}AmbiScore_{flags.Tracking.ActiveConfig.input_name}",
        TrackInput = [inputTracks],
        AmbiguityScoreProcessor = None,
    )

    from TrkConfig.TrkAmbiguitySolverConfig import TrkAmbiguitySolver_Trig_Cfg
    ambiguitySolver = algorithmCAToGlobalWrapper(
        TrkAmbiguitySolver_Trig_Cfg,
        flags,
        name = "TrigAmbiguitySolver"+flags.Tracking.ActiveConfig.input_name,
    )
    
    
    #Loading the alg to the sequence
    ptAlgs.extend( [ambiguityScore[0], ambiguitySolver[0]] )

    finalTrackCollection = ambiTrackCollection
    if doTRT:
        # do the TRT extension if requested
        finalTrackCollection = outTrkTracks
        trtAlgs = trtExtension_builder(flags, signature, config, rois, inputTracks=ambiTrackCollection, outputTracks=outTrkTracks, prefix=prefix ) 
        ptAlgs.extend( trtAlgs )

        
    #  Track particle conversion algorithm

    from xAODTrackingCnv.xAODTrackingCnvConfig import TrigTrackParticleCnvAlgCfg
    trackParticleCnvAlg = algorithmCAToGlobalWrapper(
        TrigTrackParticleCnvAlgCfg,
        flags,
        name = prefix+'xAODParticleCreatorAlg'+flags.Tracking.ActiveConfig.input_name+'_IDTrig', 
        TrackContainerName = finalTrackCollection,
        xAODTrackParticlesFromTracksContainerName = outTrackParticles,
    )
    
    ptAlgs.extend(trackParticleCnvAlg)
    
    # Potentialy other algs with more collections? 
    # Might Drop the list in the end and keep just one output key
    nameTrackCollections =[ outTrkTracks ]
    nameTrackParticles =  [ outTrackParticles ]
    
    # Return list of Track keys, TrackParticle keys, and PT algs
    return  nameTrackCollections, nameTrackParticles, ptAlgs




def trtExtension_builder(flags, signature, config, rois, inputTracks, outputTracks, prefix="InDetTrigMT" ): 

        
    trtRIOMaker           = trtRIOMaker_builder(flags, signature, config, rois, prefix  )
    trtExtensionAlg       = trtExtensionAlg_builder(flags, signature, config, inputTracks, prefix =prefix)
    trtExtensionProcessor = trtExtensionProcessor_builder(flags, signature, config, inputTracks, outputTracks, prefix )

    return [ trtRIOMaker, trtExtensionAlg, trtExtensionProcessor]
    

def trtRIOMaker_builder(inflags, signature, config, rois, prefix="InDetTrigMT" ): 
    
    log.info( "trtRIOMaker_builder: {} {}".format( signature, prefix ) )

    algs = []

    TRT_RDO_Key = "TRT_RDOs"

    from .InDetTrigCollectionKeys import TrigTRTKeys
    from AthenaCommon.GlobalFlags import globalflags

    #TODO invoke as photon from the menu
    if signature=="electrontrt":
      flags = inflags.cloneAndReplace("Tracking.ActiveConfig", "Trigger.InDetTracking."+"photon")
    else:
      flags = inflags

    # Only add raw data decoders if we're running over raw data
    if globalflags.InputFormat.is_bytestream():
        #Global keys/names for collections 
        TRT_RDO_Key = TrigTRTKeys.RDOs
        trtDataProvider = trtDataProvider_builder(flags, signature, config, TRT_RDO_Key, rois )
        algs.append( trtDataProvider )
        

    from InDetConfig.InDetPrepRawDataFormationConfig import TrigTRTRIOMakerCfg
    trtRIOMaker = algorithmCAToGlobalWrapper(TrigTRTRIOMakerCfg,
                                             flags, RoIs = rois)
    algs.extend( trtRIOMaker )
    return algs
    


def trtDataProvider_builder(flags, signature, config, TRT_RDO_Key, rois, prefix="InDetTrigMT" ) :

    
    from TrigInDetConfig.TrigInDetConfig import TRTDataProviderCfg
    trtRawDataProvider = algorithmCAToGlobalWrapper(TRTDataProviderCfg,
                                                    flags, rois, signature)
    return trtRawDataProvider[0]

    
def trtExtensionAlg_builder(flags, signature, config, inputTracks, prefix="InDetTrigMT" ): 

    from InDetConfig.TRT_TrackExtensionAlgConfig import Trig_TRT_TrackExtensionAlgCfg
    extensionAlg = algorithmCAToGlobalWrapper(Trig_TRT_TrackExtensionAlgCfg,
                                              flags, inputTracks, name="%sTrackExtensionAlg%s"%(prefix, signature),
                                              )
    return extensionAlg[0]


def trtExtensionProcessor_builder(flags, signature, config, inputTracks, outputTracks, prefix="InDetTrigMT" ):   


    from InDetConfig.InDetExtensionProcessorConfig import TrigInDetExtensionProcessorCfg
    trtExtensionProcessor = algorithmCAToGlobalWrapper(TrigInDetExtensionProcessorCfg,
                                                       flags, name="%sExtensionProcessor%s"%(prefix, signature),
                                                       )
    return trtExtensionProcessor[0]





