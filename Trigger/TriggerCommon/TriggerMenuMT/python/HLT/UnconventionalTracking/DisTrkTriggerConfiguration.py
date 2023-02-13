# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaCommon.CFElements import seqAND
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence
from AthenaCommon.Logging import logging

logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)


def DisTrkTriggerHypoSequence(flags):

        from TrigLongLivedParticlesHypo.TrigDisappearingTrackHypoConfig import TrigDisappearingTrackHypoToolFromDict
        from TrigLongLivedParticlesHypo.TrigDisappearingTrackHypoConfig import createTrigDisappearingTrackHypoAlg

        theDisTrkHypo = createTrigDisappearingTrackHypoAlg(flags, "DisTrkTrack")

        from AthenaConfiguration.ComponentAccumulator import conf2toConfigurable
        from AthenaConfiguration.ComponentFactory import CompFactory
        DummyInputMakerAlg = conf2toConfigurable(CompFactory.InputMakerForRoI( "IM_DisTrkTrack_HypoOnlyStep" ))
        DummyInputMakerAlg.RoITool = conf2toConfigurable(CompFactory.ViewCreatorInitialROITool())

        log.debug("Building the Step dictinary for DisTrk")
        return MenuSequence( flags,
                             Sequence    = seqAND("DisTrkEmptyStep",[DummyInputMakerAlg]),
                             Maker       = DummyInputMakerAlg,
                             Hypo        = theDisTrkHypo,
                             HypoToolGen = TrigDisappearingTrackHypoToolFromDict,
                         )
