# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging

logging.getLogger().info("Importing %s", __name__)
log = logging.getLogger(__name__)

from AthenaConfiguration.ComponentFactory import isComponentAccumulatorCfg
from AthenaCommon.Configurable import ConfigurableCABehavior

from ..Config.ChainConfigurationBase import ChainConfigurationBase
from .ConfigHelpers import recoKeys, AlgConfig
from ..Menu.SignatureDicts import METChainParts_Default
from ..Config.MenuComponents import appendMenuSequenceCAToAthena, RecoFragmentsPool, ChainStep

def extractMETRecoDict(chainDict, fillDefaults=True):
    """ Extract the keys relevant to reconstruction from a provided dictionary

    If fillDefaults is True then any missing keys will be taken from the
    METChainParts_Default dictionary.
    """
    if fillDefaults:
        return {k: chainDict.get(k, METChainParts_Default[k]) for k in recoKeys}
    else:
        return {k: chainDict[k] for k in recoKeys if k in chainDict}


# ----------------------------------------------------------------
# Class to configure chain
# ----------------------------------------------------------------
class METChainConfiguration(ChainConfigurationBase):
    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self, chainDict)
        # Only some subset of keys in the METChainParts dictionary describe
        # reconstruction details - only these keys are passed down into the menu
        # sequence (the actual hypo tool is created later)
        self.recoDict = extractMETRecoDict(self.chainPart)

    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------
    def assembleChainImpl(self, flags):
        log.debug("Assembling chain for %s", self.chainName)
        conf = AlgConfig.fromRecoDict(flags, **self.recoDict)
        if isComponentAccumulatorCfg():
            steps = conf.make_accumulator_steps(flags, self.dict)
        else:
            def get_ca_steps(flags, **dict_via_args):
                with ConfigurableCABehavior():
                    # Avoid setting ComboHypo in the ChainStep here otherwise the cache gets filled
                    # with a GaudiConfig2 that causes problems when generating the CF
                    ca_steps = conf.make_accumulator_steps(flags, dict_via_args, noComboHypo=True)
                return ca_steps
            ca_steps = RecoFragmentsPool.retrieve(get_ca_steps,flags,**self.dict)
            steps = []
            for step in ca_steps:
                steps.append(
                    ChainStep(
                        name=step.name,
                        multiplicity=step.multiplicity,
                        chainDicts=step.stepDicts,
                        Sequences=[appendMenuSequenceCAToAthena(s,flags) for s in step.sequences],
                    )
                )

        return self.buildChain(steps)
