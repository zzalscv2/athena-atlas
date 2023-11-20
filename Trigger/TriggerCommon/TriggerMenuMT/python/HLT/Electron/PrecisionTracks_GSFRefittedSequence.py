#
#  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
#

from AthenaCommon.CFElements import parOR
import AthenaCommon.CfgMgr as CfgMgr
from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper

#logging
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)
from TriggerMenuMT.HLT.Config.MenuComponents import algorithmCAToGlobalWrapper

def precisionTracks_GSFRefitted(flags, RoIs, ion=False, variant=''):
    """
    Takes precision Tracks as input and applies GSF refits on top
    """
    log.debug('precisionTracks_GSFRefitted(RoIs = %s, variant = %s)',RoIs,variant)

    tag = '_ion' if ion is True else ''
    tag+=variant

    # precision Tracking related data dependencies
    from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import  getTrigEgammaKeys

    TrigEgammaKeys = getTrigEgammaKeys(variant, ion=ion)

    trackParticles = TrigEgammaKeys.precisionTrackingContainer

    ViewVerifyPrecisionTrk   = CfgMgr.AthViews__ViewDataVerifier("PrecisionTrackViewDataVerifier_forGSFRefit"+tag)

    from TrigInDetConfig.InDetTrigCollectionKeys import TrigTRTKeys, TrigPixelKeys
    ViewVerifyPrecisionTrk.DataObjects = [( 'xAOD::TrackParticleContainer','StoreGateSvc+%s' % trackParticles),
                                 # verifier object needed by GSF
                                 ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.averageInteractionsPerCrossing' ), 
                                 ( 'InDet::PixelGangedClusterAmbiguities' , 'StoreGateSvc+%s' % TrigPixelKeys.PixelClusterAmbiguitiesMap ),
                                 ( 'InDet::TRT_DriftCircleContainer' , 'StoreGateSvc+%s' % TrigTRTKeys.DriftCircles ),
                                 ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.AveIntPerXDecor' )]

     # These objects must be loaded from SGIL if not from CondInputLoader
 
    from AthenaCommon.GlobalFlags import globalflags
    if (globalflags.InputFormat.is_bytestream()):
      ViewVerifyPrecisionTrk.DataObjects += [( 'IDCInDetBSErrContainer' , 'StoreGateSvc+PixelByteStreamErrs' )]
      ViewVerifyPrecisionTrk.DataObjects += [( 'TRT_RDO_Cache' , 'StoreGateSvc+TrtRDOCache' )]
    else:
      ViewVerifyPrecisionTrk.DataObjects += [( 'TRT_RDO_Container' , 'StoreGateSvc+TRT_RDOs' )]
    ViewVerifyPrecisionTrk.DataObjects += [( 'InDet::TRT_DriftCircleContainerCache' , 'StoreGateSvc+TRT_DriftCircleCache'  )]

    from TriggerMenuMT.HLT.Electron.TrigEMBremCollectionBuilder import TrigEMBremCollectionBuilderCfg
    
    thesequence_GSF = parOR( "precisionTracking_GSF%s" % RoIs)
    thesequence_GSF += ViewVerifyPrecisionTrk
    
    ## TrigEMBremCollectionBuilder ##
    
    from PixelConditionsAlgorithms.PixelConditionsConfig import PixeldEdxAlgCfg
    CAtoGlobalWrapper(PixeldEdxAlgCfg, flags)
    thesequence_GSF  += algorithmCAToGlobalWrapper(TrigEMBremCollectionBuilderCfg, flags,
                                                   name = "TrigEMBremCollectionBuilder"+variant,
                                                   TrackParticleContainerName=TrigEgammaKeys.precisionTrackingContainer,
                                                   SelectedTrackParticleContainerName=TrigEgammaKeys.precisionTrackingContainer,
                                                   OutputTrkPartContainerName=TrigEgammaKeys.precisionElectronTrackParticleContainerGSF,
                                                   OutputTrackContainerName=TrigEgammaKeys.precisionElectronTrkCollectionGSF)

     ## TrigEMTrackMatchBuilder_GSF ##
    trackParticles_GSF= TrigEgammaKeys.precisionElectronTrackParticleContainerGSF
    return thesequence_GSF, trackParticles_GSF
