# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CFElements import seqAND, parOR
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)


def bmumuxRecoSequence(flags, rois, muons):

    # ATR-20453, until such time as FS and RoI collections do not interfere, a hacky fix
    #recoSequence = parOR('bmumuxViewNode')
    recoSequence = seqAND('bmumuxViewNode')

    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    config = getInDetTrigConfig('bmumux')

    from TrigInDetConfig.InDetTrigFastTracking import makeInDetTrigFastTracking
    viewAlgs, viewDataVerifier = makeInDetTrigFastTracking(flags, config, rois)
    viewDataVerifier.DataObjects += [('TrigRoiDescriptorCollection', 'StoreGateSvc+%s' % rois),
                                     ('xAOD::MuonContainer', 'StoreGateSvc+%s' % muons)]

    # Make sure required objects are still available at whole-event level
    if flags.Input.isMC:
        from AthenaCommon.AlgSequence import AlgSequence
        topSequence = AlgSequence()
        viewDataVerifier.DataObjects += [('TRT_RDO_Container', 'StoreGateSvc+TRT_RDOs')]
        topSequence.SGInputLoader.Load += [('TRT_RDO_Container', 'StoreGateSvc+TRT_RDOs')]
    else:
        viewDataVerifier.DataObjects += [( 'TRT_RDO_Cache' , 'StoreGateSvc+TrtRDOCache' )]


    for viewAlg in viewAlgs:
        recoSequence += viewAlg

    # Precision Tracking is requested in the same view as FTF, so viewDataVerifier must not be provided
    from TrigInDetConfig.InDetTrigPrecisionTracking import makeInDetTrigPrecisionTracking
    ptTracks, ptTrackParticles, ptAlgs = makeInDetTrigPrecisionTracking(flags, config, None, rois)

    precisionTrackingSequence = parOR('precisionTrackingInBmumux', ptAlgs)
    recoSequence += precisionTrackingSequence

    return recoSequence
