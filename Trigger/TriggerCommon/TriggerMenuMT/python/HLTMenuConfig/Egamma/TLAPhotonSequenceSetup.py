#
#  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
#
from AthenaCommon.CFElements import parOR, seqAND
import AthenaCommon.CfgMgr as CfgMgr
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
from DecisionHandling.DecisionHandlingConf import  ViewCreatorPreviousROITool
#from TrigHypothesis.TrigEgammaHypo import TrigEgammaTLAPhotonFexMT

from TriggerMenuMT.HLTMenuConfig.Menu.MenuComponents import MenuSequence, RecoFragmentsPool



def TLAPhotonSequence(flags, photonsIn, chainDict):
    ''''Create TLA Photon Sequence'''


    # empty reco sequence sequence
    recoSeq = seqAND("PhotonTLASeq_"+photonsIn,  [])
    #this is the name of the output photons
    sequenceOut = photonsIn+"_TLA"
    # initializes and configure the TLA Selector Algorithm

    # this ensures data access to the HLT_egamma_Photons collection previously built in the precisionPhoton View
    ViewVerify = CfgMgr.AthViews__ViewDataVerifier("TLAPhotonViewDataVerifier")
    ViewVerify.DataObjects = [( 'xAOD::PhotonContainer' , 'StoreGateSvc+HLT_egamma_Photons')]
    

    from TrigEgammaHypo import TrigEgammaTLAPhotonFexMTConfig



    # check from chain dictionary the threshold of the photon part
    HLT_threshold = chainDict['chainParts'][0]['threshold']
    # set the Fex TLA threshold to HLT_threshold - 20 GeV, or 0 GeV is HLT_threshold is < 20
    
    # at this point the threshold is in GeV, provide it in MeV to the selector
    TLA_threshold = HLT_threshold - 20 if HLT_threshold - 20 > 0 else 0

    TLAPhotonAlg = TrigEgammaTLAPhotonFexMTConfig.getConfiguredTLAPhotonSelector(photonPtThreshold=TLA_threshold*1000, inputPhotonsKey=photonsIn, TLAPhotonsKey=sequenceOut)

    

    # The OR makes sure that TLAPhotonAlg can access the data dependencies specified by ViewVerify
    photonInViewAlgs = parOR("tlaPhotonInViewAlgs", [ViewVerify, TLAPhotonAlg])

    # adds the selector to the newborn sequence)
    recoSeq +=  photonInViewAlgs

    return ( recoSeq, sequenceOut )

def TLAPhotonAthSequence(flags, photonsIn, chainDict):
    
    
    tlaPhotonViewsMakerAlg = EventViewCreatorAlgorithm("IM_TLAPhotons")
    tlaPhotonViewsMakerAlg.RoIsLink = "initialRoI"
    tlaPhotonViewsMakerAlg.RoITool = ViewCreatorPreviousROITool() 
    # ensure that the sequence runs within a view spawned from the precisionPhoton ROI
    tlaPhotonViewsMakerAlg.InViewRoIs = "precisionPhotonRoIs"
    tlaPhotonViewsMakerAlg.ViewFallThrough = True
    tlaPhotonViewsMakerAlg.RequireParentView = True
    tlaPhotonViewsMakerAlg.Views = "TLAPhotonsViews"
    tlaPhotonViewsMakerAlg.mergeUsingFeature = True

    tlaPhotonViewsMakerAlg.ViewNodeName = "tlaPhotonInViewAlgs"

    

    
    

    (tlaPhotonSequence, sequenceOut) = RecoFragmentsPool.retrieve( TLAPhotonSequence, flags, photonsIn=photonsIn,chainDict=chainDict)
    # the TLAPhoton sequence is now tlaPhotonViewsMakerAlg --> TrigEgammaTLAPhotonFexMT
    tlaPhotonAthSequence = seqAND( "TLAPhotonAthSequence_"+photonsIn, [tlaPhotonViewsMakerAlg, tlaPhotonSequence] )
    
    return (tlaPhotonAthSequence, tlaPhotonViewsMakerAlg, sequenceOut)

def TLAPhotonMenuSequence(flags, photonsIn, chainDict):

    from TrigEgammaHypo.TrigEgammaHypoConf import TrigEgammaTLAPhotonHypoAlgMT
    from TrigEgammaHypo.TrigEgammaTLAPhotonHypoTool import TrigEgammaTLAPhotonHypoToolFromDict # JetTLA calls a function "FromDict" from a python, investigate later

    (tlaPhotonAthSequence, InputMakerAlg, sequenceOut) = RecoFragmentsPool.retrieve(TLAPhotonAthSequence, flags, photonsIn=photonsIn, chainDict=chainDict)
    hypo = TrigEgammaTLAPhotonHypoAlgMT("TrigEgammaTLAPhotonHypoAlgMT_"+photonsIn)
    hypo.Photons = sequenceOut


    return MenuSequence( Sequence    = tlaPhotonAthSequence,
                         Maker       = InputMakerAlg,
                         Hypo        = hypo,
                         HypoToolGen = TrigEgammaTLAPhotonHypoToolFromDict
                         )
