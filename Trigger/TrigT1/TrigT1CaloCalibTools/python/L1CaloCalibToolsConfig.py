# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#==============================================================================
# Provides configs for the tools/algorithms used for building/thinning L1Calo related
# object containers and decorations in the DAODs
#==============================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def LegacyTriggerTowerThinningCfg(flags, name, **kwargs):
    """
    Configure the Legacy Trigger Tower Thinning tool
    """
    acc = ComponentAccumulator()

    acc.addPublicTool(CompFactory.DerivationFramework.TriggerTowerThinningAlg(name, **kwargs), primary=True)

    return acc

def L1CaloMatchCell2TowerCfg(flags, name, **kwargs):
    """
    Configure the Legacy L1Calo match cell 2 tower tool
    """
    acc = ComponentAccumulator()

    from CaloTriggerTool.CaloTriggerToolConfig import CaloTriggerTowerServiceCfg
    kwargs.setdefault('CaloTriggerTowerService', acc.popToolsAndMerge(CaloTriggerTowerServiceCfg(flags,'CaloTriggerTowerService')))
    acc.setPrivateTools(CompFactory.LVL1.L1CaloMatchCell2Tower(name, **kwargs))

    return acc


def L1CaloCells2TriggerTowersCfg(flags, name, **kwargs):
    """
    Configure the Legacy Calo to Trigger Towers tool
    """
    acc = ComponentAccumulator()

    kwargs.setdefault('L1CaloMatchCell2Tower', acc.popToolsAndMerge(L1CaloMatchCell2TowerCfg(flags,'L1CaloMatchCell2Tower')))
    acc.setPrivateTools(CompFactory.LVL1.L1CaloCells2TriggerTowers(name, **kwargs))

    return acc


def L1CaloxAODOfflineTriggerTowerToolsCfg(flags, name, **kwargs):
    """
    Configure the tools required by L1CaloxAODOfflineTriggerTowerTools
    """
    acc = ComponentAccumulator()

    from CaloTriggerTool import L1CaloCells2TriggerTowersCfg
    kwargs.setdefault("L1CaloCells2TriggerTowers", acc.popToolsAndMerge(L1CaloCells2TriggerTowersCfg(flags,'L1CaloCells2TriggerTowers')))

    acc.setPrivateTools(CompFactory.LVL1.L1CaloxAODOfflineTriggerTowerTools(name, **kwargs))

    return acc


def LegacyTriggerTowerDecoratorCfg(flags, name, **kwargs):
    """
    Configure the legacy trigger tower decorator algorithm
    """
    acc = ComponentAccumulator()

    # get the trigger tower tools
    triggerTowerTools = acc.popToolsAndMerge(L1CaloxAODOfflineTriggerTowerToolsCfg(flags, name, **kwargs))

    acc.addEventAlgo(CompFactory.LVL1.L1CaloTriggerTowerDecoratorAlg(name, TriggerTowerTools = triggerTowerTools, **kwargs), primary=True)

    return acc

