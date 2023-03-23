#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

'''@file EGammaPhysValMonitoringConfig.py
@author T. Strebler
@date 2022-07-05
@brief Main CA-based python configuration for EGammaPhysValMonitoring
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def EgammaPhysValMonitoringToolCfg(flags, **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("EnableLumi", False)
    kwargs.setdefault("DetailLevel", 10)
    kwargs.setdefault("isMC", flags.Input.isMC)

    if "MCTruthClassifier" not in kwargs:
        from MCTruthClassifier.MCTruthClassifierConfig import (
            MCTruthClassifierCaloTruthMatchCfg)
        kwargs.setdefault("MCTruthClassifier", acc.popToolsAndMerge(
            MCTruthClassifierCaloTruthMatchCfg(flags)))

    if flags.Tracking.doLargeD0:
        kwargs.setdefault("LRTElectronContainerName", "LRTElectrons")


    from ElectronPhotonSelectorTools.AsgElectronLikelihoodToolsConfig import AsgElectronLikelihoodToolCfg
    from ElectronPhotonSelectorTools.ElectronLikelihoodToolMapping import electronLHmenu
    from ROOT import LikeEnum

    lhMenu = electronLHmenu.offlineMC21

    # VeryLooseNoPix
    ElectronLHSelectorVeryLooseNoPix = acc.popToolsAndMerge(AsgElectronLikelihoodToolCfg(
        flags,
        name="ElectronLHSelectorVeryLooseNoPix",
        quality=LikeEnum.VeryLooseLLP,
        menu=lhMenu)
    )
    ElectronLHSelectorVeryLooseNoPix.primaryVertexContainer = "PrimaryVertices"
    kwargs.setdefault("ElectronLHSelectorVeryLooseNoPix", ElectronLHSelectorVeryLooseNoPix)
    
    # LooseNoPix
    ElectronLHSelectorLooseNoPix = acc.popToolsAndMerge(AsgElectronLikelihoodToolCfg(
        flags,
        name="ElectronLHSelectorLooseNoPix",
        quality=LikeEnum.LooseLLP,
        menu=lhMenu)
    )
    ElectronLHSelectorLooseNoPix.primaryVertexContainer = "PrimaryVertices"
    kwargs.setdefault("ElectronLHSelectorLooseNoPix", ElectronLHSelectorLooseNoPix)

    # MediumNoPix
    ElectronLHSelectorMediumNoPix = acc.popToolsAndMerge(AsgElectronLikelihoodToolCfg(
        flags,
        name="ElectronLHSelectorMediumNoPix",
        quality=LikeEnum.MediumLLP,
        menu=lhMenu)
    )
    ElectronLHSelectorMediumNoPix.primaryVertexContainer = "PrimaryVertices"
    kwargs.setdefault("ElectronLHSelectorMediumNoPix", ElectronLHSelectorMediumNoPix)

    # TightNoPix
    ElectronLHSelectorTightNoPix = acc.popToolsAndMerge(AsgElectronLikelihoodToolCfg(
        flags,
        name="ElectronLHSelectorTightNoPix",
        quality=LikeEnum.TightLLP,
        menu=lhMenu)
    )
    ElectronLHSelectorTightNoPix.primaryVertexContainer = "PrimaryVertices"
    kwargs.setdefault("ElectronLHSelectorTightNoPix", ElectronLHSelectorTightNoPix)

    acc.setPrivateTools(
        CompFactory.EgammaPhysValMonitoring.EgammaPhysValMonitoringTool(**kwargs))


    return acc

