#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
#           Setup of precision tracking

from AthenaCommon.Logging import logging
log = logging.getLogger("InDetTrigPrecisionTracking")

from TriggerMenuMT.HLT.Config.MenuComponents import extractAlgorithmsAndAppendCA

def makeInDetTrigPrecisionTracking( inflags, config=None, verifier=False, rois='EMViewRoIs', prefix="InDetTrigMT" ) :
    
    log.info( "makeInDetTrigPrecisionTracking:: {} {} doTRT: {} ".format(  config.input_name, config.name, config.doTRT ) )
    ptAlgs = []
    
    # Expects configuration  
    if config is None:
      raise ValueError('PrecisionTracking No configuration provided!')

    from TrigInDetConfig.utils import getFlagsForActiveConfig
    flags = getFlagsForActiveConfig(inflags, config.input_name, log)

    if verifier:
        from .InDetTrigCollectionKeys import TrigPixelKeys
        inputTracks = flags.Tracking.ActiveConfig.trkTracks_FTF 
        verifier.DataObjects += [( 'InDet::PixelGangedClusterAmbiguities' , 'StoreGateSvc+' + TrigPixelKeys.PixelClusterAmbiguitiesMap ),
                                 ( 'TrackCollection' , 'StoreGateSvc+' +inputTracks ),
                                 ( 'TRT_RDO_Cache' , 'StoreGateSvc+TrtRDOCache' ),
                                 ( 'InDet::TRT_DriftCircleContainerCache' , 'StoreGateSvc+TRT_DriftCircleCache'  ),
                                 ( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+%s' % rois) ]


    from TrigInDetConfig.InDetTrigSequence import InDetTrigSequence
    seq = InDetTrigSequence(flags, 
                            flags.Tracking.ActiveConfig.input_name, 
                            rois = rois, 
                            inView = verifier.getName() if verifier else '')
    
    ca = seq.sequenceAfterPattern()
    sequence = extractAlgorithmsAndAppendCA(ca)
    ptAlgs.extend(sequence)

    
    outTrkTracks        = flags.Tracking.ActiveConfig.trkTracks_IDTrig # Final output Track collection
    outTrackParticles   = flags.Tracking.ActiveConfig.tracks_IDTrig
    nameTrackCollections =[ outTrkTracks ]
    nameTrackParticles =  [ outTrackParticles ]

    return  nameTrackCollections, nameTrackParticles, ptAlgs






