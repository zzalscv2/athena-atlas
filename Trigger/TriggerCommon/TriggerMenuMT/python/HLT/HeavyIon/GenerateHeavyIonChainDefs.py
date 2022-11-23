# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info('Importing %s', __name__)
log = logging.getLogger(__name__)

from TriggerMenuMT.HLT.Config.Utility.ChainDictTools import splitChainDict
from TriggerMenuMT.HLT.Config.Utility.ChainMerging import mergeChainDefs
from .HeavyIonChainConfiguration import HeavyIonChainConfig

def generateChainConfigs(chainDict):

  listOfChainDicts = splitChainDict(chainDict)
  log.debug('Implement case for heavy ion chain with %d legs', len(listOfChainDicts))

  listOfChainDefs = []

  for subChainDict in listOfChainDicts:
    HeavyIon = HeavyIonChainConfig(subChainDict).assembleChain()

    listOfChainDefs += [HeavyIon]
    log.debug('length of chaindefs: %d', len(listOfChainDefs))

  if len(listOfChainDefs) > 1:
    log.debug('Implement case for multi-leg heavy ion chain')
    theChainDef = mergeChainDefs(listOfChainDefs, chainDict)
  else:
    theChainDef = listOfChainDefs[0]

  log.debug('theChainDef %s', theChainDef)

  return theChainDef

