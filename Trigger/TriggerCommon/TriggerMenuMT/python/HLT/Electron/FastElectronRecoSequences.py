#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
from AthenaCommon.CFElements import parOR

#logging
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys
from TriggerMenuMT.HLT.Config.MenuComponents import algorithmCAToGlobalWrapper

def fastElectronRecoSequence(flags, RoIs, variant=''):
    
    import AthenaCommon.CfgMgr as CfgMgr

    TrigEgammaKeys = getTrigEgammaKeys(variant)
    trackParticlesName = TrigEgammaKeys.fastTrackParticleContainer  

    # A simple algorithm to confirm that data has been inherited from parent view
    # Required to satisfy data dependencies
    ViewVerifyTrk = CfgMgr.AthViews__ViewDataVerifier("FastTrackViewDataVerifier_FTF"+variant)
    from TriggerMenuMT.HLT.CommonSequences.CaloSequences import CaloMenuDefs  
    ViewVerifyTrk.DataObjects += [( 'xAOD::TrigEMClusterContainer' , 'StoreGateSvc+%s' % CaloMenuDefs.L2CaloClusters ),
                                  ( 'xAOD::TrackParticleContainer' , 'StoreGateSvc+%s' % trackParticlesName ),
                                  ( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+%s' % RoIs )]
    ViewVerifyTrk.DataObjects += [( 'InDet::TRT_DriftCircleContainerCache' , 'StoreGateSvc+TRT_DriftCircleCache'  )]

    from TrigEgammaRec.TrigEgammaFastElectronConfig import fastElectronFexAlgCfg
    theElectronFex = algorithmCAToGlobalWrapper(fastElectronFexAlgCfg, flags, name="EgammaFastElectronFex_Clean_gen"+variant, rois=RoIs)[0]
    theElectronFex.TrigEMClusterName = CaloMenuDefs.L2CaloClusters
    theElectronFex.TrackParticlesName = trackParticlesName
    theElectronFex.ElectronsName = TrigEgammaKeys.fastElectronContainer

    fastElectronRecoSequence = parOR( "fastElectron"+RoIs)
    fastElectronRecoSequence += ViewVerifyTrk
    fastElectronRecoSequence += theElectronFex
    
    collectionOut = TrigEgammaKeys.fastElectronContainer
    
    return fastElectronRecoSequence, collectionOut
