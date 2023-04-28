# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from ..Config.MenuComponents import MenuSequence, MenuSequenceCA, RecoFragmentsPool, algorithmCAToGlobalWrapper, SelectionCA, InEventRecoCA
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.CFElements import seqAND
from TrigEDMConfig.TriggerEDMRun3 import recordable
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)


def bmumuxAlgSequence(flags):
    from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
    from DecisionHandling.DecisionHandlingConf import  ViewCreatorCentredOnIParticleROITool, ViewCreatorMuonSuperROITool 

    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    IDConfig = getInDetTrigConfig( "bphysics" )

    log.debug("TrigBPhysMenuSequence: eta half width %s ", IDConfig.etaHalfWidth)
    log.debug("TrigBPhysMenuSequence: phi half width %s ", IDConfig.phiHalfWidth)
    log.debug("TrigBPhysMenuSequence: zed half width %s ", IDConfig.zedHalfWidth)
    log.debug("TrigBphysMenuSequence: superRoI %s ", IDConfig.SuperRoI)

    if IDConfig.SuperRoI:
        viewCreatorROITool = ViewCreatorMuonSuperROITool(
            RoIEtaWidth = IDConfig.etaHalfWidth,
            RoIPhiWidth = IDConfig.phiHalfWidth,
            RoIZedWidth = IDConfig.zedHalfWidth,
            RoisWriteHandleKey = recordable(IDConfig.roi))
    else:
        viewCreatorROITool = ViewCreatorCentredOnIParticleROITool(
            RoIEtaWidth = IDConfig.etaHalfWidth,
            RoIPhiWidth = IDConfig.phiHalfWidth,
            RoIZedWidth = IDConfig.zedHalfWidth,
            RoisWriteHandleKey = recordable(IDConfig.roi))


    viewMaker = EventViewCreatorAlgorithm(
        name = 'IMbmumux',
        mergeUsingFeature = True,
        RoITool = viewCreatorROITool,
        Views = 'BmumuxViews',
        InViewRoIs = 'BmumuxViewRoIs',
        ViewFallThrough = True,
        PlaceMuonInView = True,
        InViewMuonCandidates = 'BmumuxMuonCandidates',
        InViewMuons = 'HLT_Muons_Bmumux')

    from .BphysicsRecoSequences import bmumuxRecoSequence
    recoSequence = bmumuxRecoSequence(flags, viewMaker.InViewRoIs, viewMaker.InViewMuons)

    viewMaker.ViewNodeName = recoSequence.name()

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
    robPrefetchAlg = algorithmCAToGlobalWrapper(ROBPrefetchingAlgCfg_Si, flags, nameSuffix=viewMaker.name())[0]

    sequence = seqAND('bmumuxSequence', [viewMaker, robPrefetchAlg, recoSequence])

    return (sequence, viewMaker)


def bmumuxSequence(flags):
    from TrigBphysHypo.TrigBphysHypoConf import TrigBphysStreamerHypo
    from TrigBphysHypo.TrigBphysStreamerHypoConfig import TrigBphysStreamerHypoToolFromDict

    sequence, viewMaker = RecoFragmentsPool.retrieve(bmumuxAlgSequence, flags)
    hypo = TrigBphysStreamerHypo('BmumuxStreamerHypoAlg')

    return MenuSequence(flags,
        Sequence = sequence,
        Maker = viewMaker,
        Hypo = hypo,
        HypoToolGen = TrigBphysStreamerHypoToolFromDict)


def dimuL2Sequence(flags):
    from ..Muon.MuonMenuSequences import muCombAlgSequence
    from TrigBphysHypo.TrigBphysHypoConf import TrigBphysStreamerHypo
    from TrigBphysHypo.TrigBphysStreamerHypoConfig import TrigBphysStreamerHypoToolFromDict

    sequence, viewMaker, combinedMuonContainerName = RecoFragmentsPool.retrieve(muCombAlgSequence, flags)

    hypo = TrigBphysStreamerHypo(
        name = 'DimuL2StreamerHypoAlg',
        triggerList = getNoL2CombChainNames(),
        triggerLevel = 'L2')

    return MenuSequence(flags,
        Sequence = sequence,
        Maker = viewMaker,
        Hypo = hypo,
        HypoToolGen = TrigBphysStreamerHypoToolFromDict)


def dimuEFSequence(flags):
    selAcc = SelectionCA('dimuSequence')

    inputMakerAlg = CompFactory.InputMakerForRoI('IM_bphysStreamerDimuEF',
        RoITool = CompFactory.ViewCreatorPreviousROITool(),
        mergeUsingFeature = True)

    reco = InEventRecoCA('bphysStreamerDimuEFReco', inputMaker=inputMakerAlg)
    selAcc.mergeReco(reco)

    hypoAlg = CompFactory.TrigBphysStreamerHypo('DimuEFStreamerHypoAlg', triggerLevel = 'EF')
    selAcc.addHypoAlgo(hypoAlg)

    from TrigBphysHypo.TrigBphysStreamerHypoConfig import TrigBphysStreamerHypoToolFromDict
    return MenuSequenceCA(flags, selAcc, HypoToolGen=TrigBphysStreamerHypoToolFromDict)


def getNoL2CombChainNames():
    from ..Config.GenerateMenuMT import GenerateMenuMT
    menu = GenerateMenuMT()
    chains = [chain.name for chain in menu.chainsInMenu['Bphysics'] if 'noL2Comb' in chain.name]
    return chains
