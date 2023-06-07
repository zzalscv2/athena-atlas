#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

# menu components   
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, RecoFragmentsPool
from AthenaCommon.CFElements import seqAND
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
from DecisionHandling.DecisionHandlingConf import ViewCreatorPreviousROITool
from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys	  import getTrigEgammaKeys
from TriggerMenuMT.HLT.Config.MenuComponents import algorithmCAToGlobalWrapper
from AthenaCommon.CFElements import parOR
from AthenaConfiguration.ComponentFactory import CompFactory

def tag(ion):
    return 'precision' + ('HI' if ion is True else '') + 'CaloPhoton'


def precisionCaloSequence(flags, ion=False):
    """ Creates PrecisionCalo sequence """
    TrigEgammaKeys = getTrigEgammaKeys(ion=ion)
    # EV creator
    InViewRoIs="PrecisionCaloRoIs"    
    precisionCaloViewsMaker = EventViewCreatorAlgorithm('IM' + tag(ion))
    precisionCaloViewsMaker.ViewFallThrough = True
    precisionCaloViewsMaker.RoIsLink = "initialRoI" # Merge inputs based on their initial L1 ROI
    roiTool = ViewCreatorPreviousROITool()
    # Note: This step processes Decision Objects which have followed either Electron reco, Photon reco, or both.
    # For Decision Object which have followed both, there is an ambiguity about which ROI should be used in this
    # merged step. In such cases we break the ambiguity by specifying that the Electron ROI is to be used.
    roiTool.RoISGKey = "HLT_Roi_FastElectron"
    precisionCaloViewsMaker.RoITool = roiTool
    precisionCaloViewsMaker.InViewRoIs = InViewRoIs
    precisionCaloViewsMaker.Views = tag(ion) + 'Views'
    precisionCaloViewsMaker.RequireParentView = True

    # reco sequence
    hiInfo = 'HI' if ion is True else ''
    from TriggerMenuMT.HLT.Photon.PrecisionCaloRecoSequences import precisionCaloRecoSequence
    precisionCaloSequence = algorithmCAToGlobalWrapper(precisionCaloRecoSequence,flags, InViewRoIs,'gPrecisionCaloRecoSequence'+hiInfo, ion)
   
    precisionCaloInViewSequence = parOR("photonRoITopoRecoSequence"+hiInfo, [precisionCaloSequence])

    precisionCaloViewsMaker.ViewNodeName = precisionCaloInViewSequence.name()

    theSequence = seqAND(tag(ion) + 'Sequence', [])

    sequenceOut = TrigEgammaKeys.precisionPhotonCaloClusterContainer
    if ion is True:
        # add UE subtraction for heavy ion e/gamma triggers
        # NOTE: UE subtraction requires an average pedestal to be calculated
        # using the full event (FS info), and has to be done outside of the
        # event views in this sequence. the egammaFSRecoSequence is thus placed
        # before the precisionCaloInViewSequence.
        from TriggerMenuMT.HLT.Egamma.TrigEgammaFactoriesCfg import egammaFSCaloRecoSequenceCfg
        egammaFSRecoSequence = algorithmCAToGlobalWrapper(egammaFSCaloRecoSequenceCfg,flags)
        theSequence += egammaFSRecoSequence

    # connect EVC and reco
    theSequence += [precisionCaloViewsMaker, precisionCaloInViewSequence]
    return (theSequence, precisionCaloViewsMaker, sequenceOut)



def precisionCaloMenuSequence(flags, name, is_probe_leg=False, ion=False):
    """ Creates precisionCalo MENU sequence """

    (sequence, precisionCaloViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(precisionCaloSequence, flags, ion=ion)

    #Hypo
    from TrigEgammaHypo.TrigEgammaPrecisionCaloHypoTool import TrigEgammaPrecisionCaloHypoToolFromDict

    thePrecisionCaloHypo = CompFactory.TrigEgammaPrecisionCaloHypoAlg(name+tag(ion) + 'Hypo')
    thePrecisionCaloHypo.CaloClusters = sequenceOut

    return MenuSequence( flags,
                         Sequence    = sequence,
                         Maker       = precisionCaloViewsMaker, 
                         Hypo        = thePrecisionCaloHypo,
                         HypoToolGen = TrigEgammaPrecisionCaloHypoToolFromDict,
                         IsProbe     = is_probe_leg)

