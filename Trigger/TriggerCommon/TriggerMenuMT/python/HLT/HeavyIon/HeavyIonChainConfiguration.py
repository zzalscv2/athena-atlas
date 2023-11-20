# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info('Importing %s', __name__)
log = logging.getLogger(__name__)
from AthenaConfiguration.ComponentFactory import isComponentAccumulatorCfg
from ..Config.ChainConfigurationBase import ChainConfigurationBase
from ..HeavyIon.HeavyIonMenuSequences import HIFwdGapMenuSequenceCfg

#----------------------------------------------------------------
# fragments generating configuration will be functions in New JO,
# so let's make them functions already now
#----------------------------------------------------------------
def HIFwdGapMenuSequence(flags):
  from ..Config.MenuComponents import menuSequenceCAToGlobalWrapper
  return menuSequenceCAToGlobalWrapper(HIFwdGapMenuSequenceCfg, flags)


class HeavyIonChainConfig(ChainConfigurationBase):

  def __init__(self, chainDict):
    ChainConfigurationBase.__init__(self, chainDict)

  # ----------------------
  # Assemble the chain depending on information from chainName
  # ----------------------
  def assembleChainImpl(self, flags):
    log.debug('Assembling chain for %s', self.chainName)
    steps = []
    if isComponentAccumulatorCfg():
      if 'Fgap' in self.chainPart['hypoFgapInfo'][0]:
        steps.append(self.getStep(flags,1, 'Fgap', [HIFwdGapMenuSequenceCfg]))
    else:
      if 'Fgap' in self.chainPart['hypoFgapInfo'][0]:
        steps.append(self.getStep(flags, 1, 'Fgap', [HIFwdGapMenuSequence]))

    return self.buildChain(steps)

  def getHIFwdGapStep(self, flags):
    return self.getStep(flags, 1, 'Fgap', [HIFwdGapMenuSequence])

