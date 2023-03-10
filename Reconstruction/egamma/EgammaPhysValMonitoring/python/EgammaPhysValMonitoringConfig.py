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

    # VeryLooseNoPix
    ElectronLHSelectorVeryLooseNoPix = CompFactory.AsgElectronLikelihoodTool(
        "ElectronLHSelectorVeryLooseNoPix",
        ConfigFile = "ElectronPhotonSelectorTools/trigger/rel22_20210611/ElectronLikelihoodVeryLooseTriggerConfig_NoPix.conf",
        primaryVertexContainer = "PrimaryVertices")
    kwargs.setdefault("ElectronLHSelectorVeryLooseNoPix", ElectronLHSelectorVeryLooseNoPix)

    # LooseNoPix
    ElectronLHSelectorLooseNoPix = CompFactory.AsgElectronLikelihoodTool(
        "ElectronLHSelectorLooseNoPix",
        ConfigFile = "ElectronPhotonSelectorTools/trigger/rel22_20210611/ElectronLikelihoodLooseTriggerConfig_NoPix.conf",
        primaryVertexContainer = "PrimaryVertices")
    kwargs.setdefault("ElectronLHSelectorLooseNoPix", ElectronLHSelectorLooseNoPix)

    # MediumNoPix
    ElectronLHSelectorMediumNoPix = CompFactory.AsgElectronLikelihoodTool(
        "ElectronLHSelectorMediumNoPix",
        ConfigFile = "ElectronPhotonSelectorTools/trigger/rel22_20210611/ElectronLikelihoodMediumTriggerConfig_NoPix.conf",
        primaryVertexContainer = "PrimaryVertices")
    kwargs.setdefault("ElectronLHSelectorMediumNoPix", ElectronLHSelectorMediumNoPix)

    # TightNoPix
    ElectronLHSelectorTightNoPix = CompFactory.AsgElectronLikelihoodTool(
        "ElectronLHSelectorTightNoPix",
        ConfigFile = "ElectronPhotonSelectorTools/trigger/rel22_20210611/ElectronLikelihoodTightTriggerConfig_NoPix.conf",
        primaryVertexContainer = "PrimaryVertices")
    kwargs.setdefault("ElectronLHSelectorTightNoPix", ElectronLHSelectorTightNoPix)

    acc.setPrivateTools(
        CompFactory.EgammaPhysValMonitoring.EgammaPhysValMonitoringTool(**kwargs))


    return acc

