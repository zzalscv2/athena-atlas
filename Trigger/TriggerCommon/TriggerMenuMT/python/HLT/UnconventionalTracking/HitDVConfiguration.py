# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CFElements import seqAND
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, RecoFragmentsPool
from AthenaCommon.Logging import logging

logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from TrigCaloRec.TrigCaloRecConfig import jetmetTopoClusteringCfg
from TriggerMenuMT.HLT.Config.MenuComponents import algorithmCAToGlobalWrapper

def UTTJetRecoSequence(flags):

        topoClusterSequence = algorithmCAToGlobalWrapper(jetmetTopoClusteringCfg,
                                                 flags = flags,
                                                 RoIs = '')
        clustersKey = "HLT_TopoCaloClustersFS"
 
        from TrigStreamerHypo.TrigStreamerHypoConf   import TrigStreamerHypoAlg
        from TrigStreamerHypo.TrigStreamerHypoConfig import StreamerHypoToolGenerator

        from ..Jet.JetRecoSequences  import jetRecoSequence
        from ..Jet.JetRecoCommon     import extractRecoDict
        from ..Menu.SignatureDicts   import JetChainParts_Default
        
        jetRecoDict = extractRecoDict([JetChainParts_Default])
        jetRecoDict.update( 
                {'recoAlg': 'a4', 'constitType': 'tc', 'clusterCalib': 'em', 'constitMod': '', 'trkopt': 'notrk'}
        )

        trkcolls = {}

        JetSeq, jetName, jetDef = RecoFragmentsPool.retrieve(
            jetRecoSequence,
            flags,
            clustersKey=clustersKey,
            **trkcolls,
            **jetRecoDict,
        )

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
        from TrigLongLivedParticlesHypo.TrigHitDVHypoConfig import createTrigHitDVHypoAlg

        theHitDVHypo = createTrigHitDVHypoAlg(flags, "HitDV")

        from AthenaConfiguration.ComponentAccumulator import conf2toConfigurable
        from AthenaConfiguration.ComponentFactory import CompFactory
        DummyInputMakerAlg = conf2toConfigurable(CompFactory.InputMakerForRoI( "IM_HitDV_HypoOnlyStep" ))
        DummyInputMakerAlg.RoITool = conf2toConfigurable(CompFactory.ViewCreatorInitialROITool())

        return MenuSequence( flags,
                             Sequence    = seqAND("HitDVEmptyStep",[DummyInputMakerAlg]),
                             Maker       = DummyInputMakerAlg,
                             Hypo        = theHitDVHypo,
                             HypoToolGen = TrigHitDVHypoToolFromDict,
                         )
