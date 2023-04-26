#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

# menu components
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, RecoFragmentsPool
from AthenaCommon.CFElements import parOR, seqAND
import AthenaCommon.CfgMgr as CfgMgr
from AthenaCommon.GlobalFlags import globalflags
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
from DecisionHandling.DecisionHandlingConf import ViewCreatorCentredOnClusterROITool
from TrigTRTHighTHitCounter.TrigTRTHTHCounterConfig import TrigTRTHTHCounterFex

def TRTHitGeneratorSequence(flags):

    from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys
    TrigEgammaKeys = getTrigEgammaKeys()

    from InDetTrigRecExample import InDetTrigCA
    InDetTrigCA.InDetTrigConfigFlags = flags

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
    ViewVerify = CfgMgr.AthViews__ViewDataVerifier("TRTHitGeneratorViewDataVerifier")
    ViewVerify.DataObjects = [('TrigRoiDescriptorCollection' , 'StoreGateSvc+TRTHitGenerator'),
                             ]
    if not globalflags.InputFormat.is_bytestream():
        ViewVerify.DataObjects += [( 'TRT_RDO_Container' , 'StoreGateSvc+TRT_RDOs' )]

    # calling trtRIOMaker
    from TrigInDetConfig.InDetTrigPrecisionTracking import trtRIOMaker_builder
    trtInviewAlgs = trtRIOMaker_builder(flags, signature = "electrontrt", config = None, rois=InViewRoIs)
    
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


