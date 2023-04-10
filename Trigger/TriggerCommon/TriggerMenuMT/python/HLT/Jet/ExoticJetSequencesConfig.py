# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
log = logging.getLogger("TriggerMenuMT.HLT.Jet.JetChainSequences")

from ..Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA

from AthenaConfiguration.ComponentFactory import CompFactory

def jetEJsMenuSequence(flags, jetsIn, name):
    
    from TrigHLTJetHypo.TrigJetHypoToolConfig import trigJetEJsHypoToolFromDict

    # Get track sequence name
    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    IDTrigConfig = getInDetTrigConfig( 'jet' )
    sequenceOut  = IDTrigConfig.tracks_FTF()
    vertices     = IDTrigConfig.vertex_jet
    
    reco = InEventRecoCA(
        f"EmergingJets_HypoOnlyStep_{jetsIn}Reco",
        inputMaker=CompFactory.InputMakerForRoI(
            "IM_EmergingJets_HypoOnlyStep",
            RoITool = CompFactory.ViewCreatorInitialROITool(),
            mergeUsingFeature = True
        )
    )

    selAcc = SelectionCA(f"EmergingJets_HypoOnlyStep_{jetsIn}")
    selAcc.mergeReco(reco)

    selAcc.addHypoAlgo(
        CompFactory.TrigJetEJsHypoAlg(
            "L2EmergingJets",
            Tracks = sequenceOut,
            PV     = vertices
        )
    )

    return MenuSequenceCA(flags, selAcc, HypoToolGen=trigJetEJsHypoToolFromDict)

                                                                                                                            
def jetCRMenuSequence(flags, jetsIn, name):

    from TrigHLTJetHypo.TrigJetHypoToolConfig import trigJetCRHypoToolFromDict

    # Get track sequence name
    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    from ..CommonSequences.FullScanDefs import fs_cells, trkFSRoI
    IDTrigConfig = getInDetTrigConfig( 'jet' )
    sequenceOut  = IDTrigConfig.tracks_FTF()
    cellsin=fs_cells

    from .JetMenuSequencesConfig import getTrackingInputMaker
    from .JetTrackingConfig import JetFSTrackingCfg
    trk_acc = JetFSTrackingCfg(flags, trkopt='ftf', RoIs=trkFSRoI)

    reco = InEventRecoCA(f"EmergingJets_HypoOnlyStep_{jetsIn}Reco", inputMaker=getTrackingInputMaker('ftf'))
    reco.mergeReco(trk_acc)

    selAcc = SelectionCA(f"EmergingJets_HypoOnlyStep_{jetsIn}")
    selAcc.mergeReco(reco)
    selAcc.addHypoAlgo(
        CompFactory.TrigJetCRHypoAlg(
            "L2CalRatio",
            Tracks = sequenceOut,
            Cells  = cellsin
        )
    )

    return MenuSequenceCA(flags, selAcc, HypoToolGen=trigJetCRHypoToolFromDict)
