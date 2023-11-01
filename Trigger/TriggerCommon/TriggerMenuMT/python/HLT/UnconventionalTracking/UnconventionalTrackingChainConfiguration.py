# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from TriggerMenuMT.HLT.Config.ChainConfigurationBase import ChainConfigurationBase
from TrigLongLivedParticlesHypo.TrigDJHypoConfig import (TrigDJComboHypoToolFromDict)

from AthenaConfiguration.ComponentFactory import isComponentAccumulatorCfg
from ..Config.MenuComponents import menuSequenceCAToGlobalWrapper


#----------------------------------------------------------------
# Class to configure chain
#----------------------------------------------------------------
class UnconventionalTrackingChainConfiguration(ChainConfigurationBase):

    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self,chainDict)

    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------
    def assembleChainImpl(self, inflags):
        log.debug("Assembling chain %s", self.chainName)
        
        from TrigInDetConfig.utils import getFlagsForActiveConfig
        flags = getFlagsForActiveConfig(inflags, "fullScan", log)
        
        chainSteps = []

        stepDictionary = self.getStepDictionary()

        key = self.chainPart['trigType']
        steps = stepDictionary[key]

        for step in steps:
            chainstep = getattr(self, step)(flags)
            chainSteps += [chainstep]

        myChain = self.buildChain(chainSteps)

        return myChain

    def getStepDictionary(self):

        stepDictionary = {
            "isotrk" : ['getIsoHPtTrackEmpty', 'getRoITrkEmpty', 'getFTFTrackReco', 'getIsoHPtTrackTrigger'],
            "fslrt" : ['getFSLRTEmpty', 'getRoITrkEmpty', 'getFSLRTTrigger'],
            "dedxtrk" : ['getdEdxEmpty', 'getRoITrkEmpty', 'getFTFTrackReco', 'getdEdxTrigger'],
            "hitdvjet" : ['getJetReco', 'getRoITrkEmpty', 'getFTFTrackReco', 'getHitDVTrigger'],
            "fsvsi" : ['getVSIEmpty', 'getRoITrkEmpty', 'getVSITrigger'],
            "distrk" : ['getDisTrkEmpty', 'getRoITrkEmpty', 'getFTFTrackReco', 'getDisTrkTrigger'],
            "dispvtx" : ['getJetReco', 'getRoITrkEmpty', 'getFTFTrackReco', 'getHitDVTrigger', 'getDVRecoStep', 'getDVEDStep'],
            "dispjet" : ['getJetReco', 'getRoITrkEmpty', 'getFTFTrackReco', 'getDJPromptStep', 'getDJDispStep']
        }

        return stepDictionary

    # --------------------
    # Step definitions in alignment order
    # Step 1
    def getJetReco(self, flags):
        return self.getStep(flags,1,'JetRecoOnlyCfg',[JetRecoOnlyCfg])
    # Empty for alignment
    def getIsoHPtTrackEmpty(self, flags):
        return  self.getEmptyStep(1,'EmptyUncTrk')
    def getFSLRTEmpty(self, flags):
        return self.getEmptyStep(1, 'FSLRTEmptyStep')
    def getDisTrkEmpty(self, flags):
        return self.getEmptyStep(1, 'DisTrkEmptyStep')
    def getVSIEmpty(self, flags):
        return self.getEmptyStep(1, 'VSIEmptyStep')
    def getdEdxEmpty(self, flags):
        return self.getEmptyStep(1, 'dEdxEmptyStep')

    # Step 2
    def getFSLRTTrigger(self, flags):
        return self.getStep(flags,2,'FSLRTTrigger',[FSLRTTriggerCfg])
    # Empty for alignment with jets
    def getRoITrkEmpty(self, flags):
        return self.getEmptyStep(2, 'RoITrkEmptyStep')

    # Step 3 -- all FTF tracking here
    def getFTFTrackReco(self, flags):
        return self.getStep(flags,3,'FTFRecoOnlyCfg',[FTFRecoOnlyCfg])

    # Step 4+ -- everything post FTF tracking
    def getIsoHPtTrackTrigger(self, flags):
        return self.getStep(flags,4,'IsoHPtTrackTriggerCfg',[IsoHPtTrackTriggerCfg])
    def getdEdxTrigger(self, flags):
        return self.getStep(flags,4,'dEdxTriggerCfg',[dEdxTriggerCfg])
    def getHitDVTrigger(self, flags):
        return self.getStep(flags,4,'HitDVTriggerCfg',[HitDVTriggerCfg])
    def getDisTrkTrigger(self, flags):
        return self.getStep(flags,4,'DisTrkTriggerCfg',[DisTrkTriggerCfg])
    def getVSITrigger(self, flags):
        return self.getStep(flags,4,'VSITrigger',[VSITriggerCfg])
    def getDJPromptStep(self, flags):
        return self.getStep(flags,3,'DJPromptStepCfg',[DJPromptStepCfg], comboTools = [TrigDJComboHypoToolFromDict])
    def getDJDispStep(self, flags):
        return self.getStep(flags,4,'DJDispStepCfg',[DJDispStepCfg])
    def getDVRecoStep(self, flags):
        return self.getStep(flags,5,'DVRecoStepCfg',[DVRecoStepCfg])
    def getDVEDStep(self, flags):
        return self.getStep(flags,6,'DVEDStepCfg',[DVEDStepCfg])



def IsoHPtTrackTriggerCfg(flags):
    from TriggerMenuMT.HLT.UnconventionalTracking.IsoHighPtTrackTriggerConfiguration import IsoHPtTrackTriggerHypoSequence
    return IsoHPtTrackTriggerHypoSequence(flags)

def FTFRecoOnlyCfg(flags):
    from TriggerMenuMT.HLT.UnconventionalTracking.CommonConfiguration import getFullScanRecoOnlySequence
    if isComponentAccumulatorCfg():
        return getFullScanRecoOnlySequence(flags)
    else:
        return menuSequenceCAToGlobalWrapper(getFullScanRecoOnlySequence,flags)

def FSLRTTriggerCfg(flags):
    from TriggerMenuMT.HLT.UnconventionalTracking.FullScanLRTTrackingConfiguration import FullScanLRTTriggerMenuSequence
    if isComponentAccumulatorCfg():
        return FullScanLRTTriggerMenuSequence(flags)
    else:
        return menuSequenceCAToGlobalWrapper(FullScanLRTTriggerMenuSequence,flags)
    
def VSITriggerCfg(flags):
    from TriggerMenuMT.HLT.UnconventionalTracking.VrtSecInclusiveConfiguration import VrtSecInclusiveMenuSequence
    if isComponentAccumulatorCfg():
        return VrtSecInclusiveMenuSequence(flags)
    else:
        return menuSequenceCAToGlobalWrapper(VrtSecInclusiveMenuSequence,flags)

def dEdxTriggerCfg(flags):
    from TriggerMenuMT.HLT.UnconventionalTracking.dEdxTriggerConfiguration import dEdxTriggerHypoSequence
    if isComponentAccumulatorCfg():
        return dEdxTriggerHypoSequence(flags)
    else:
        return menuSequenceCAToGlobalWrapper(dEdxTriggerHypoSequence, flags)
        
def HitDVTriggerCfg(flags):
    from TriggerMenuMT.HLT.UnconventionalTracking.HitDVConfiguration import HitDVHypoSequence
    if isComponentAccumulatorCfg():
        return HitDVHypoSequence(flags)
    else:
        return menuSequenceCAToGlobalWrapper(HitDVHypoSequence, flags)

def JetRecoOnlyCfg(flags):
    from TriggerMenuMT.HLT.UnconventionalTracking.HitDVConfiguration import UTTJetRecoSequence
    return UTTJetRecoSequence(flags)

def DisTrkTriggerCfg(flags):
    from TriggerMenuMT.HLT.UnconventionalTracking.DisTrkTriggerConfiguration import DisTrkTriggerHypoSequence
    return DisTrkTriggerHypoSequence(flags)

def DJPromptStepCfg(flags):
    from TriggerMenuMT.HLT.UnconventionalTracking.DJTriggerConfiguration import DJPromptStep
    return DJPromptStep(flags)

def DJDispStepCfg(flags):
    from TriggerMenuMT.HLT.UnconventionalTracking.DJTriggerConfiguration import DJDispStep
    return DJDispStep(flags)

def DVRecoStepCfg(flags):
    from TriggerMenuMT.HLT.UnconventionalTracking.DVTriggerConfiguration import DVRecoSequence
    if isComponentAccumulatorCfg():
        return DVRecoSequence(flags)
    else:
        return menuSequenceCAToGlobalWrapper(DVRecoSequence, flags)


def DVEDStepCfg(flags):
    from TriggerMenuMT.HLT.UnconventionalTracking.DVTriggerConfiguration import DVTriggerEDSequence
    if isComponentAccumulatorCfg():
        return DVTriggerEDSequence(flags)
    else:
        return menuSequenceCAToGlobalWrapper(DVTriggerEDSequence, flags)
