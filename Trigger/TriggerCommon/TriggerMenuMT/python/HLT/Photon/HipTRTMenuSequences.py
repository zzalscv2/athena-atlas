#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

# menu components
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, RecoFragmentsPool
from AthenaCommon.CFElements import parOR, seqAND
import AthenaCommon.CfgMgr as CfgMgr
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
from DecisionHandling.DecisionHandlingConf import ViewCreatorCentredOnClusterROITool
from TrigTRTHighTHitCounter.TrigTRTHTHCounterConfig import TrigTRTHTHCounterFex

def TRTHitGeneratorSequence(flags):

    from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys
    TrigEgammaKeys = getTrigEgammaKeys()

    """ hipTRT step ....."""
    InViewRoIs = "TRTHitGenerator"
    # EVCreator:
    roiTool = ViewCreatorCentredOnClusterROITool()
    roiTool.AllowMultipleClusters = False 
    roiTool.RoisWriteHandleKey = "HLT_Roi_TRTHit"
    roiTool.RoIEtaWidth = 0.10
    roiTool.RoIPhiWidth = 0.10
    trtViewsMaker = EventViewCreatorAlgorithm("IMTRTHitGenerator")
    trtViewsMaker.RoIsLink = "initialRoI" # Merge inputs based on their initial L1 ROI
    trtViewsMaker.RoITool = roiTool
    trtViewsMaker.InViewRoIs = InViewRoIs
    trtViewsMaker.Views = "TRTHitGeneratorViews"
    trtViewsMaker.ViewFallThrough = True
    trtViewsMaker.RequireParentView = True
   
    # view data verifier
    ViewVerifyName = "TRTHitGeneratorViewDataVerifier"
    ViewVerify = CfgMgr.AthViews__ViewDataVerifier(ViewVerifyName)
    ViewVerify.DataObjects = [('TrigRoiDescriptorCollection' , 'StoreGateSvc+TRTHitGenerator'),
                             ]
    if flags.Input.isMC:
        ViewVerify.DataObjects += [( 'TRT_RDO_Container' , 'StoreGateSvc+TRT_RDOs' )]
    else:
        ViewVerify.DataObjects += [( 'TRT_RDO_Cache' , 'StoreGateSvc+TrtRDOCache' )]

    ViewVerify.DataObjects += [( 'InDet::TRT_DriftCircleContainerCache' , 'StoreGateSvc+TRT_DriftCircleCache'  )]

    from AthenaCommon.Logging import logging
    log = logging.getLogger(__name__)
    from TrigInDetConfig.utils import getFlagsForActiveConfig
    flagsWithTrk = getFlagsForActiveConfig(flags, 'photon', log)

    from TriggerMenuMT.HLT.Config.MenuComponents import extractAlgorithmsAndAppendCA
    from TrigInDetConfig.InDetTrigSequence import InDetTrigSequence

    seq = InDetTrigSequence(flagsWithTrk, flagsWithTrk.Tracking.ActiveConfig.input_name, 
                            rois = trtViewsMaker.InViewRoIs, inView = ViewVerifyName)

    ca = seq.dataPreparationTRT()
    trtInviewAlgs = extractAlgorithmsAndAppendCA(ca)
    
    trtHTHFex = TrigTRTHTHCounterFex(flags, name="TrigTRTH_fex",
                                     RoIs = trtViewsMaker.InViewRoIs,
                                     containerName = "TRT_TrigDriftCircles",
                                     RNNOutputName = TrigEgammaKeys.TrigTRTHTCountsContainer)

    sequenceOut = TrigEgammaKeys.TrigTRTHTCountsContainer
    
    trtInviewAlgs = parOR("trtInviewAlgs", trtInviewAlgs + [ViewVerify,trtHTHFex])
    trtViewsMaker.ViewNodeName = "trtInviewAlgs"


    trtDataSequence = seqAND("trtDataSequence", [trtViewsMaker, trtInviewAlgs ] )
    return (sequenceOut, trtDataSequence, trtViewsMaker)



def hipTRTMenuSequence(flags, is_probe_leg=False):
    """ Creates TRTDataGenerator MENU sequence """
    (sequenceOut, trtDataSequence, trtViewsMaker) = RecoFragmentsPool.retrieve(TRTHitGeneratorSequence, flags)

    #Hypo
    from TrigTRTHighTHitCounter.TrigTRTHighTHitCounterConf import TrigTRTHTHhypoAlg
    from TrigTRTHighTHitCounter.TrigTRTHTHhypoTool import TrigTRTHTHhypoToolFromDict

    trtHTHhypo = TrigTRTHTHhypoAlg("TrigTRTHTHhypo")
    trtHTHhypo.RNNOutputName = sequenceOut 

    return MenuSequence( flags,
                         Sequence    = trtDataSequence,
                         Maker       = trtViewsMaker, 
                         Hypo        = trtHTHhypo,
                         HypoToolGen = TrigTRTHTHhypoToolFromDict,
                         IsProbe     = is_probe_leg)


