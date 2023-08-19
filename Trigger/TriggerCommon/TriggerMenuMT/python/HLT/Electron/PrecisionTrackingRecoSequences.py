#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaCommon.CFElements import parOR
import AthenaCommon.CfgMgr as CfgMgr

#logging
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys

def precisionTracking(flags, RoIs, ion=False, variant=''):

    ## Taking Fast Track information computed in 2nd step ##
    TrigEgammaKeys = getTrigEgammaKeys(variant, ion=ion)

    IDTrigConfig = TrigEgammaKeys.IDTrigConfig

    tag = '_ion' if ion is True else ''

    # TrackCollection="TrigFastTrackFinder_Tracks_Electron"
    ViewVerifyTrk = CfgMgr.AthViews__ViewDataVerifier("FastTrackViewDataVerifier"+ variant + tag)
    
    ViewVerifyTrk.DataObjects = [( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+%s' % RoIs ),
                                 ( 'CaloCellContainer' , 'StoreGateSvc+CaloCells' ),
                                 ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.averageInteractionsPerCrossing' ),
                                 ]

    """ Precision Track Related Setup.... """
    PTAlgs = []
    PTTracks = []
    PTTrackParticles = []
    
    from TrigInDetConfig.InDetTrigPrecisionTracking import makeInDetTrigPrecisionTracking

    PTTracks, PTTrackParticles, PTAlgs = makeInDetTrigPrecisionTracking( flags, config = IDTrigConfig, verifier = ViewVerifyTrk, rois= RoIs )
    PTSeq = parOR("precisionTrackingInElectrons" + variant + tag, PTAlgs)
    #trackParticles = PTTrackParticles[-1]    
    trackParticles = TrigEgammaKeys.precisionTrackingContainer

    electronPrecisionTrack = parOR("electronPrecisionTrack" + variant + tag)
    electronPrecisionTrack += ViewVerifyTrk
    electronPrecisionTrack += PTSeq

    return electronPrecisionTrack, trackParticles

   
