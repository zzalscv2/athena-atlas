# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info('Importing %s', __name__)
log = logging.getLogger(__name__)

from ..Config.ChainConfigurationBase import ChainConfigurationBase
from AthenaConfiguration.ComponentFactory import isRun3Cfg

if isRun3Cfg():
  pass
else:
  from ..HeavyIon.HeavyIonMenuSequences import HIFwdGapMenuSequence

#----------------------------------------------------------------
# fragments generating configuration will be functions in New JO,
# so let's make them functions already now
#----------------------------------------------------------------
def HIFwdGapMenuSequenceCfg(flags):
  return HIFwdGapMenuSequence()

class HeavyIonChainConfig(ChainConfigurationBase):

  def __init__(self, chainDict):
    ChainConfigurationBase.__init__(self, chainDict)

  # ----------------------
  # Assemble the chain depending on information from chainName
  # ----------------------
  def assembleChainImpl(self):
    log.debug('Assembling chain for %s', self.chainName)
    steps = []

    if 'Fgap' in self.chainPart['hypoFgapInfo'][0]:
      steps.append(self.getHIFwdGapStep())

    return self.buildChain(steps)

  def getHIFwdGapStep(self):
    return self.getStep(1, 'Fgap', [HIFwdGapMenuSequenceCfg])

