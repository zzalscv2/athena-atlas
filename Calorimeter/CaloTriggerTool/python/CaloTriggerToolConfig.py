# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#==============================================================================
# Provides configs for the tools/algorithms used for building/thinning L1Calo related
# object containers and decorations in the DAODs
#==============================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def CaloTriggerTowerServiceCfg(flags, name, **kwargs):
    """
    Configure the CaloTriggerTowerService
    """
    acc = ComponentAccumulator()

    # The calorimeter towers require conditions data
    from  CaloConditions.CaloConditionsConfig import CaloTriggerTowerCfg
    acc.merge(CaloTriggerTowerCfg(flags))

    acc.setPrivateTools(CompFactory.CaloTriggerTowerService(name, **kwargs))

    return acc
