# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
log = logging.getLogger("TriggerMenuMT.HLT.Jet.JetChainSequences")

from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, RecoFragmentsPool
from AthenaCommon.CFElements import seqAND

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import conf2toConfigurable

def returnEJSequence(flags, algo):

    return seqAND("EmergingJetsExoticSeq", [algo])

def jetEJsMenuSequence(flags, jetsIn, name):
    
    from TrigHLTJetHypo.TrigHLTJetHypoConf import TrigJetEJsHypoAlg
    from TrigHLTJetHypo.TrigJetHypoToolConfig import trigJetEJsHypoToolFromDict

    # Get track sequence name
    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    IDTrigConfig = getInDetTrigConfig( 'jet' )
    sequenceOut  = IDTrigConfig.tracks_FTF()
    vertices     = IDTrigConfig.vertex_jet
    
    #Setup the hypothesis algorithm
    theEmergingJetsTriggerHypo           = TrigJetEJsHypoAlg("L2EmergingJets")
    theEmergingJetsTriggerHypo.Tracks    = sequenceOut
    theEmergingJetsTriggerHypo.PV        = vertices

    DummyInputMakerAlg = conf2toConfigurable(CompFactory.InputMakerForRoI( "IM_EmergingJets_HypoOnlyStep" ) )
    DummyInputMakerAlg.RoITool = conf2toConfigurable(CompFactory.ViewCreatorInitialROITool())
    DummyInputMakerAlg.mergeUsingFeature = True

    sequence = RecoFragmentsPool.retrieve( returnEJSequence , flags, algo=DummyInputMakerAlg)


    return MenuSequence( flags,
                         Sequence    = sequence,
                         Maker       = DummyInputMakerAlg,
                         Hypo        = theEmergingJetsTriggerHypo,
                         HypoToolGen = trigJetEJsHypoToolFromDict,
    )
                                                                                                                            
def jetCRMenuSequence(flags, jetsIn, name):

    from TrigHLTJetHypo.TrigHLTJetHypoConf import TrigJetCRHypoAlg
    from TrigHLTJetHypo.TrigJetHypoToolConfig import trigJetCRHypoToolFromDict

    # Get track sequence name
    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    from ..CommonSequences.FullScanDefs import fs_cells, trkFSRoI
    IDTrigConfig = getInDetTrigConfig( 'jet' )
    sequenceOut  = IDTrigConfig.tracks_FTF()
    cellsin=fs_cells

    from .JetRecoSequences import JetFSTrackingSequence
    from .JetMenuSequences import getTrackingInputMaker
    trkSeq = RecoFragmentsPool.retrieve(
        JetFSTrackingSequence, flags, trkopt='ftf', RoIs=trkFSRoI
    )

    IMAlg = getTrackingInputMaker('ftf')
    sequence = seqAND( 'CalRatioExoticSeq', [IMAlg, trkSeq] )

    #Setup the hypothesis algorithm
    theCalRatioTriggerHypo           = TrigJetCRHypoAlg("L2CalRatio")
    theCalRatioTriggerHypo.Tracks    = sequenceOut
    theCalRatioTriggerHypo.Cells        = cellsin

    return MenuSequence( flags,
                         Sequence    = sequence,
                         Maker       = IMAlg,
                         Hypo        = theCalRatioTriggerHypo,
                         HypoToolGen = trigJetCRHypoToolFromDict,
    )
