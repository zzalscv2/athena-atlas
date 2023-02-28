#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys


def fastTracking(flags, RoIs, variant=''):
    TrigEgammaKeys = getTrigEgammaKeys(variant)
    IDTrigConfig = TrigEgammaKeys.IDTrigConfig
    from TrigInDetConfig.InDetTrigFastTracking import makeInDetTrigFastTracking
    viewAlgs, viewVerify = makeInDetTrigFastTracking( flags, config = IDTrigConfig, rois = RoIs )
    viewVerify.DataObjects += [( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+%s' % RoIs )]

    TrackParticlesName = ""
    for viewAlg in viewAlgs:
        if "InDetTrigTrackParticleCreatorAlg" in viewAlg.name():
            TrackParticlesName = viewAlg.TrackParticlesName
    
    return viewAlgs, TrackParticlesName
