# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# menu components   
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, RecoFragmentsPool
from AthenaCommon.CFElements import seqAND
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
from DecisionHandling.DecisionHandlingConf import ViewCreatorPreviousROITool

# logger
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

def tag(ion):
    return 'precision' + ('HI' if ion is True else '') + 'PhotonCaloIso'

def precisionPhotonCaloIsoSequence(flags, ion=False):
    """ This function creates the PrecisionPhotonCaloIso sequence"""
    # Prepare first the EventView
    InViewRoIs="PrecisionPhotonCaloIsoRoIs"                                          
    precisionPhotonCaloIsoViewsMaker = EventViewCreatorAlgorithm( "IM" + tag(ion))
    precisionPhotonCaloIsoViewsMaker.ViewFallThrough = True                          
    precisionPhotonCaloIsoViewsMaker.RequireParentView = True
    precisionPhotonCaloIsoViewsMaker.RoIsLink = "initialRoI"            # ROI link used to merge inputs
    precisionPhotonCaloIsoViewsMaker.RoITool = ViewCreatorPreviousROITool() # Tool used to supply ROIs for EventViews
    precisionPhotonCaloIsoViewsMaker.InViewRoIs = InViewRoIs            # names to use for the collection of which the RoIs are picked up
    precisionPhotonCaloIsoViewsMaker.Views = tag(ion) + "Views"     # Output container which has the view objects

    # Configure the reconstruction algorithm sequence
    from TriggerMenuMT.HLT.Photon.PrecisionPhotonCaloIsoRecoSequences import precisionPhotonCaloIsoRecoSequence
    (precisionPhotonCaloIsoInViewSequence, sequenceOut) = precisionPhotonCaloIsoRecoSequence(flags, RoIs=InViewRoIs, ion=ion)

    precisionPhotonCaloIsoViewsMaker.ViewNodeName = precisionPhotonCaloIsoInViewSequence.name()

    theSequence = seqAND(tag(ion)+"Sequence", [])
    # Add first the sequence part that is FS, so to run outside the view
    from TriggerMenuMT.HLT.Egamma.TrigEgammaFactories import egammaFSEventDensitySequence
    theSequence += egammaFSEventDensitySequence(flags)

    # And now add the the rest which is run isnide the EventView:
    theSequence += [precisionPhotonCaloIsoViewsMaker,precisionPhotonCaloIsoInViewSequence]

    return (theSequence, precisionPhotonCaloIsoViewsMaker, sequenceOut)



def precisionPhotonCaloIsoMenuSequence(flags, name,ion=False, is_probe_leg=False):

    # This will be executed after pricisionPhoton

    """Creates precisionPhotonCaloIso  sequence"""
    (sequence, precisionPhotonCaloIsoViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(precisionPhotonCaloIsoSequence,flags,ion=ion)

    from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys
    TrigEgammaKeys = getTrigEgammaKeys()

    # Hypo 
    from TrigEgammaHypo.TrigEgammaPrecisionPhotonCaloIsoHypoTool import createTrigEgammaPrecisionPhotonCaloIsoHypoAlg
    thePrecisionPhotonCaloIsoHypo = createTrigEgammaPrecisionPhotonCaloIsoHypoAlg(name+ tag(ion) +"Hypo", sequenceOut, TrigEgammaKeys.precisionPhotonContainer )


    
    from TrigEgammaHypo.TrigEgammaPrecisionPhotonCaloIsoHypoTool import TrigEgammaPrecisionPhotonCaloIsoHypoToolFromDict

    return MenuSequence( flags,
                         Sequence    = sequence,
                         Maker       = precisionPhotonCaloIsoViewsMaker, 
                         Hypo        = thePrecisionPhotonCaloIsoHypo,
                         HypoToolGen = TrigEgammaPrecisionPhotonCaloIsoHypoToolFromDict,
                         IsProbe     = is_probe_leg)

