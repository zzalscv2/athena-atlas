# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CFElements import seqAND
from AthenaCommon.Configurable import ConfigurableCABehavior
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence
from AthenaCommon.Logging import logging

logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from TrigCaloRec.TrigCaloRecConfig import jetmetTopoClusteringCfg
from TriggerMenuMT.HLT.Config.MenuComponents import algorithmCAToGlobalWrapper, extractAlgorithmsAndAppendCA

from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA
from AthenaConfiguration.ComponentFactory import CompFactory



def UTTJetRecoSequence(flags):

        topoClusterSequence = algorithmCAToGlobalWrapper(jetmetTopoClusteringCfg,
                                                 flags = flags,
                                                 RoIs = '')
        clustersKey = "HLT_TopoCaloClustersFS"
 
        from TrigStreamerHypo.TrigStreamerHypoConf   import TrigStreamerHypoAlg
        from TrigStreamerHypo.TrigStreamerHypoConfig import StreamerHypoToolGenerator

        from ..Jet.JetRecoSequencesConfig  import JetRecoCfg
        from ..Jet.JetRecoCommon     import extractRecoDict
        from ..Menu.SignatureDicts   import JetChainParts_Default
        
        jetRecoDict = extractRecoDict([JetChainParts_Default])
        jetRecoDict.update( 
                {'recoAlg': 'a4', 'constitType': 'tc', 'clusterCalib': 'em', 'constitMod': '', 'trkopt': 'notrk'}
        )

        with ConfigurableCABehavior():
                JetCA, jetName, jetDef = JetRecoCfg(flags, clustersKey, **jetRecoDict)
                JetSeq = extractAlgorithmsAndAppendCA(JetCA)

        HypoAlg = TrigStreamerHypoAlg("UTTJetRecDummyStream")

        from TrigT2CaloCommon.CaloDef import clusterFSInputMaker
        IMalg = clusterFSInputMaker()

        return MenuSequence( flags,
                             Sequence    = seqAND("UTTJetRecoSeq", [IMalg,topoClusterSequence,JetSeq]),
                             Maker       = IMalg,
                             Hypo        = HypoAlg,
                             HypoToolGen = StreamerHypoToolGenerator
                     )


def HitDVHypoSequence(flags):
        from TrigLongLivedParticlesHypo.TrigHitDVHypoConfig import TrigHitDVHypoToolFromDict
        from TrigLongLivedParticlesHypo.TrigHitDVHypoConfig import TrigHitDVHypoAlgCfg

#                                     Sequence    = seqAND("HitDVEmptyStep",[DummyInputMakerAlg]),

        selAcc = SelectionCA('HitDVSeq')


        theHitDVHypo = TrigHitDVHypoAlgCfg(flags, "HitDV")



        DummyInputMakerAlg = CompFactory.InputMakerForRoI( "IM_HitDV_HypoOnlyStep" )
        DummyInputMakerAlg.RoITool = CompFactory.ViewCreatorInitialROITool()
        reco = InEventRecoCA('HitDVEmptyStep',inputMaker=DummyInputMakerAlg)

        selAcc.mergeReco(reco)
        selAcc.mergeHypo(theHitDVHypo)


        return MenuSequenceCA( flags,
                               selAcc,
                               HypoToolGen = TrigHitDVHypoToolFromDict
                              )
