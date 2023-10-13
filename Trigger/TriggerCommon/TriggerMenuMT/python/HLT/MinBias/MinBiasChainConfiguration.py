# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger( __name__ )

from TriggerMenuMT.HLT.Config.MenuComponents import EmptyMenuSequence, EmptyMenuSequenceCA
from TriggerMenuMT.HLT.Config.ChainConfigurationBase import ChainConfigurationBase
from AthenaConfiguration.ComponentFactory import isComponentAccumulatorCfg

from TriggerMenuMT.HLT.MinBias.MinBiasMenuSequences import (MinBiasSPSequenceCfg, 
                                                            MinBiasTrkSequenceCfg,
                                                            MinBiasMbtsSequenceCfg,
                                                            MinBiasZVertexFinderSequenceCfg)
from TriggerMenuMT.HLT.MinBias.ALFAMenuSequences import ALFAPerfSequence
from TriggerMenuMT.HLT.MinBias.AFPMenuSequence import AFPTrkSequence, AFPGlobalSequence

#----------------------------------------------------------------
# fragments generating configuration will be functions in New JO,
# so let's make them functions already now
#----------------------------------------------------------------

def MinBiasSPCfgWrap(flags):
    from ..Config.MenuComponents import menuSequenceCAToGlobalWrapper 
    return menuSequenceCAToGlobalWrapper(MinBiasSPSequenceCfg, flags)   

def MinBiasTrkSequenceCfgWrap(flags):
    from ..Config.MenuComponents import menuSequenceCAToGlobalWrapper 
    return menuSequenceCAToGlobalWrapper(MinBiasTrkSequenceCfg, flags)

def MinBiasMbtsEmptySequenceCfg(flags):
    return EmptyMenuSequence("EmptyMbts")

def MinBiasZFindEmptySequenceCfg(flags):
    return EmptyMenuSequence("EmptyZFind")

def AFPGlobalSequenceCfgWrap(flags):
    from ..Config.MenuComponents import menuSequenceCAToGlobalWrapper
    return menuSequenceCAToGlobalWrapper(AFPGlobalSequence, flags)

def AFPTrkSequenceCfgWrap(flags):
    from ..Config.MenuComponents import menuSequenceCAToGlobalWrapper
    return menuSequenceCAToGlobalWrapper(AFPTrkSequence, flags)

def ALFAPerfSequenceCfgWrap(flags):
    from ..Config.MenuComponents import menuSequenceCAToGlobalWrapper
    return menuSequenceCAToGlobalWrapper(ALFAPerfSequence, flags)

def MinBiasZVertexFinderCfgWrap(flags):
    from ..Config.MenuComponents import menuSequenceCAToGlobalWrapper
    return menuSequenceCAToGlobalWrapper(MinBiasZVertexFinderSequenceCfg, flags)

def MinBiasMbtsCfgWrap(flags):
    from ..Config.MenuComponents import menuSequenceCAToGlobalWrapper
    return menuSequenceCAToGlobalWrapper(MinBiasMbtsSequenceCfg, flags)


class MinBiasChainConfig(ChainConfigurationBase):

    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self,chainDict)

    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------
    def assembleChainImpl(self, flags):
        log.debug("Assembling chain for %s", self.chainName)
        steps = []

        if isComponentAccumulatorCfg():
            if "mbts" == self.chainPart['recoAlg'][0] or "mbts" in self.chainName:
                steps.append(self.getStep(flags,1,'Mbts',[MinBiasMbtsSequenceCfg]))
            else:
                steps.append(self.getStep(flags,1,'EmptyMbts',[lambda flags: EmptyMenuSequenceCA("EmptyMbts") ]))

            if self.chainPart['recoAlg'][0] in ['sp', 'sptrk', 'hmt', 'excl']:
                steps.append(self.getStep(flags,2,'SPCount',[MinBiasSPSequenceCfg]))
            if self.chainPart['recoAlg'][0] in ['sptrk', 'hmt', 'excl']:
                steps.append(self.getStep(flags, 3, "ZFind", [MinBiasZVertexFinderSequenceCfg]))
                steps.append(self.getStep(flags, 4, "TrkCount", [MinBiasTrkSequenceCfg]))

        else:
            if "mbts" == self.chainPart['recoAlg'][0] or "mbts" in self.chainName:
                steps.append(self.getMinBiasMbtsStep(flags))
            elif "afprec" == self.chainPart['recoAlg'][0]:
                steps.append(self.getAFPTrkStep(flags))
            else:
                steps.append(self.getMinBiasEmptyMbtsStep(flags))

            if "afptof" in self.chainPart['recoAlg']:
                steps.append(self.getAFPGlobalStep(flags))

            if self.chainPart['recoAlg'][0] in ['sp', 'sptrk', 'hmt', 'excl']:
                steps.append(self.getMinBiasSpStep(flags))

            if self.chainPart['recoAlg'][0] in ['sptrk', 'hmt', 'excl']:
                steps.append(self.getMinBiasZFindStep(flags))
                steps.append(self.getMinBiasTrkStep(flags))

            if "_alfaperf" in self.chainName:
                steps.append(self.getALFAPerfStep(flags))

        return self.buildChain(steps)

    def getMinBiasMbtsStep(self, flags):
        return self.getStep(flags,1,'Mbts',[MinBiasMbtsCfgWrap])

    def getMinBiasEmptyMbtsStep(self, flags):
        return self.getStep(flags,1,'EmptyMbts',[MinBiasMbtsEmptySequenceCfg])

    def getMinBiasSpStep(self, flags):
        return self.getStep(flags,2,'SPCount',[MinBiasSPCfgWrap])

    def getMinBiasZFindStep(self, flags):
        return self.getStep(flags,3,'ZFind',[MinBiasZVertexFinderCfgWrap])

    def getMinBiasEmptyZFindStep(self, flags):
        return self.getStep(flags,3,'EmptyZFind',[MinBiasZFindEmptySequenceCfg])

    def getMinBiasTrkStep(self, flags):
        return self.getStep(flags,4,'TrkCount',[MinBiasTrkSequenceCfgWrap])

    def getAFPTrkStep(self, flags):
         return self.getStep(flags,1,'AFPTrk',[AFPTrkSequenceCfgWrap])

    def getAFPGlobalStep(self, flags):
         return self.getStep(flags,1,'AFPGlobal',[AFPGlobalSequenceCfgWrap])

    def getALFAPerfStep(self, flags):
        return self.getStep(flags,1,'ALFAPerf',[ALFAPerfSequenceCfgWrap])
