# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# menu components   
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, RecoFragmentsPool
from AthenaCommon.CFElements import seqAND, parOR
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
from DecisionHandling.DecisionHandlingConf import ViewCreatorPreviousROITool
from TriggerMenuMT.HLT.Config.MenuComponents import algorithmCAToGlobalWrapper
from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys

# logger
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

def tag(ion):
    return 'precision' + ('HI' if ion is True else '') + 'Photon'

def precisionPhotonSequence(flags, ion=False):
    """ This function creates the PrecisionPhoton sequence"""
    # Prepare first the EventView
    InViewRoIs="PrecisionPhotonRoIs"                                          
    precisionPhotonViewsMaker = EventViewCreatorAlgorithm( "IM" + tag(ion))
    precisionPhotonViewsMaker.ViewFallThrough = True                          
    precisionPhotonViewsMaker.RequireParentView = True
    precisionPhotonViewsMaker.RoIsLink = "initialRoI"            # ROI link used to merge inputs
    precisionPhotonViewsMaker.RoITool = ViewCreatorPreviousROITool() # Tool used to supply ROIs for EventViews
    precisionPhotonViewsMaker.InViewRoIs = InViewRoIs            # names to use for the collection of which the RoIs are picked up
    precisionPhotonViewsMaker.Views = tag(ion) + "Views"     # Output container which has the view objects

    # Configure the reconstruction algorithm sequence
    import AthenaCommon.CfgMgr as CfgMgr
    TrigEgammaKeys = getTrigEgammaKeys(ion = ion)    
    caloClusters = TrigEgammaKeys.precisionPhotonCaloClusterContainer

    ViewVerify = CfgMgr.AthViews__ViewDataVerifier("PrecisionPhotonViewDataVerifier" + tag(ion))
    dataObjects = [( 'xAOD::CaloClusterContainer' , 'StoreGateSvc+%s' % caloClusters ),
                              ( 'EgammaRecContainer', 'StoreGateSvc+%s' % TrigEgammaKeys.precisionPhotonSuperClusterCollection),
                              ( 'CaloCellContainer' , 'StoreGateSvc+CaloCells' ),
                              ( 'xAOD::EventInfo' , 'StoreGateSvc+EventInfo' ),
                              ]
    if ion:
        dataObjects += [( 'CaloCellContainer' , 'StoreGateSvc+CorrectedRoICaloCells' )]
 
    ViewVerify.DataObjects = dataObjects

    hiInfo = 'HI' if ion is True else ''
    from TriggerMenuMT.HLT.Photon.PrecisionPhotonRecoSequences import precisionPhotonRecoSequence
    precisionPhotonSequence = algorithmCAToGlobalWrapper(precisionPhotonRecoSequence,flags, InViewRoIs,'PrecisionPhotonRecoSequence'+hiInfo, ion)

    precisionPhotonInViewSequence = parOR("precisionPhotonInViewSequence"+hiInfo, [ViewVerify, precisionPhotonSequence])
    precisionPhotonViewsMaker.ViewNodeName = precisionPhotonInViewSequence.name()

    theSequence = seqAND(tag(ion)+"Sequence", [])

    # And now add the the rest which is run isnide the EventView:
    theSequence += [precisionPhotonViewsMaker,precisionPhotonInViewSequence]

    sequenceOut = TrigEgammaKeys.precisionPhotonContainer

    return (theSequence, precisionPhotonViewsMaker, sequenceOut)



def precisionPhotonMenuSequence(flags, name,ion=False, is_probe_leg=False):
    """Creates precisionPhoton  sequence"""

    # This will be executed after pricisionCalo, so we need to pickup indeed the topoClusters by precisionCalo and add them here as requirements

    (sequence, precisionPhotonViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(precisionPhotonSequence,flags,ion=ion)

    # Hypo 
    from TrigEgammaHypo.TrigEgammaPrecisionPhotonHypoTool import createTrigEgammaPrecisionPhotonHypoAlg
    thePrecisionPhotonHypo = createTrigEgammaPrecisionPhotonHypoAlg(flags, name+ tag(ion) +"Hypo", sequenceOut)
    
    from TrigEgammaHypo.TrigEgammaPrecisionPhotonHypoTool import TrigEgammaPrecisionPhotonHypoToolFromDict

    return MenuSequence( flags,
                         Sequence    = sequence,
                         Maker       = precisionPhotonViewsMaker, 
                         Hypo        = thePrecisionPhotonHypo,
                         HypoToolGen = TrigEgammaPrecisionPhotonHypoToolFromDict,
                         IsProbe     = is_probe_leg)


