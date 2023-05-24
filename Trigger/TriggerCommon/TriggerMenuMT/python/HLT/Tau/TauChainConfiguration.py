# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

########################################################################
#
# SliceDef file for muon chains/signatures
#
#########################################################################
from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from TriggerMenuMT.HLT.Config.ChainConfigurationBase import ChainConfigurationBase
from AthenaConfiguration.ComponentFactory import isComponentAccumulatorCfg
from ..Config.MenuComponents import menuSequenceCAToGlobalWrapper

from .generateTau import tauCaloMVAMenuSeq, tauFTFTauCoreSeq

if isComponentAccumulatorCfg():
    from .generateTau import tauFTFTauIsoSeq, tauFTFTauIsoBDTSeq
else:
    from .TauMenuSequences import tauFTFTauLRTSeq, tauFTFTauIsoSeq, tauFTFTauIsoBDTSeq, tauTrackTwoMVASeq, tauTrackTwoLLPSeq, tauTrackLRTSeq, tauPrecTrackIsoSeq, tauPrecTrackLRTSeq

#--------------------------------------------------------
# fragments generating config will be functions in new JO
#--------------------------------------------------------
def getTauCaloMVACfg(flags, is_probe_leg=False):
    if isComponentAccumulatorCfg():
       return tauCaloMVAMenuSeq(flags, "Tau", is_probe_leg=is_probe_leg)
    else:
       return menuSequenceCAToGlobalWrapper(tauCaloMVAMenuSeq,flags, "Tau", is_probe_leg=is_probe_leg)

def getFTFCoreCfg(flags, is_probe_leg=False):
    if isComponentAccumulatorCfg():
       return tauFTFTauCoreSeq(flags, is_probe_leg=is_probe_leg)
    else:
       return menuSequenceCAToGlobalWrapper(tauFTFTauCoreSeq,flags, is_probe_leg=is_probe_leg)

def getFTFLRTCfg(flags, is_probe_leg=False):
    return tauFTFTauLRTSeq(flags, is_probe_leg=is_probe_leg)

def getFTFIsoCfg(flags, is_probe_leg=False):
    return tauFTFTauIsoSeq(flags, is_probe_leg=is_probe_leg)

def getFTFIsoBDTCfg(flags, is_probe_leg=False):
    return tauFTFTauIsoBDTSeq(flags, is_probe_leg=is_probe_leg)

def getTrackTwoMVACfg(flags, is_probe_leg=False):
    return tauTrackTwoMVASeq(flags, is_probe_leg=is_probe_leg)

def getTrackTwoLLPCfg(flags, is_probe_leg=False):
    return tauTrackTwoLLPSeq(flags, is_probe_leg=is_probe_leg)

def getTrackLRTCfg(flags, is_probe_leg=False):
    return tauTrackLRTSeq(flags, is_probe_leg=is_probe_leg)

def getPrecTrackIsoCfg(flags, is_probe_leg=False):
    return tauPrecTrackIsoSeq(flags, is_probe_leg=is_probe_leg)

def getPrecTrackLRTCfg(flags, is_probe_leg=False):
    return tauPrecTrackLRTSeq(flags, is_probe_leg=is_probe_leg)

############################################# 
###  Class/function to configure muon chains 
#############################################

class TauChainConfiguration(ChainConfigurationBase):

    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self,chainDict)
        
    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------
    def assembleChainImpl(self, flags):                            
        chainSteps = []
        log.debug("Assembling chain for %s", self.chainName)

        # --------------------
        # define here the names of the steps and obtain the chainStep configuration 
        # --------------------
        stepDictionary = {
            "ptonly"        :['getCaloMVASeq', 'getFTFEmpty', 'getTrkEmpty' , 'getPTEmpty'      , 'getIDEmpty'     ],
            "tracktwoMVA"   :['getCaloMVASeq', 'getFTFCore' , 'getFTFIso'   , 'getPrecTrackIso' , 'getTrackTwoMVA' ],
            "tracktwoMVABDT":['getCaloMVASeq', 'getFTFCore' , 'getFTFIsoBDT', 'getPrecTrackIso' , 'getTrackTwoMVA' ],
            "tracktwoLLP"   :['getCaloMVASeq', 'getFTFCore' , 'getFTFIso'   , 'getPrecTrackIso' , 'getTrackTwoLLP' ],
            "trackLRT"      :['getCaloMVASeq', 'getFTFLRT'  , 'getTrkEmpty' , 'getPrecTrackLRT' , 'getTrackLRT'    ],
        }

        # this should be extended by the signature expert to make full use of the dictionary!
        key = self.chainPart['preselection']
        steps=stepDictionary[key]
        for step in steps:
            is_probe_leg = self.chainPart['tnpInfo']=='probe'
            if 'Empty' in step:
                chainstep = getattr(self, step)(flags)
            else:
                chainstep = getattr(self, step)(flags, is_probe_leg=is_probe_leg)
            chainSteps+=[chainstep]
    
        myChain = self.buildChain(chainSteps)
        return myChain


    # --------------------
    def getCaloMVASeq(self, flags, is_probe_leg=False):
        stepName = 'MVA_tau'
        return self.getStep(flags,1,stepName, [getTauCaloMVACfg], is_probe_leg=is_probe_leg)
        
    # --------------------
    def getFTFCore(self, flags, is_probe_leg=False):
        stepName = 'FTFCore_tau'
        return self.getStep(flags,2,stepName, [getFTFCoreCfg], is_probe_leg=is_probe_leg)

    # --------------------
    def getFTFLRT(self, flags, is_probe_leg=False):
        stepName = 'FTFLRT_tau'
        return self.getStep(flags,2,stepName, [getFTFLRTCfg], is_probe_leg=is_probe_leg)

    # --------------------

    def getFTFEmpty(self, flags):
        stepName = 'FTFEmpty_tau'
        return self.getEmptyStep(2,stepName)

    # --------------------

    def getFTFIso(self, flags, is_probe_leg=False):
        stepName = 'FTFIso_tau'
        return self.getStep(flags,3,stepName, [getFTFIsoCfg], is_probe_leg=is_probe_leg)

    # --------------------

    def getFTFIsoBDT(self, flags, is_probe_leg=False):
        stepName = 'FTFIsoBDT_tau'
        return self.getStep(flags,3,stepName, [getFTFIsoBDTCfg], is_probe_leg=is_probe_leg)

    # --------------------

    def getTrkEmpty(self, flags):
        stepName = 'TrkEmpty_tau'
        return self.getEmptyStep(3,stepName)

    # --------------------

    def getPrecTrackIso(self, flags, is_probe_leg=False):
        stepName = 'PrecTrkIso_tau'
        return self.getStep(flags,4,stepName,[getPrecTrackIsoCfg],is_probe_leg=is_probe_leg)

    # --------------------
    def getPrecTrackLRT(self, flags, is_probe_leg=False):
        stepName = 'PrecTrkLRT_tau'
        return self.getStep(flags,4,stepName,[getPrecTrackLRTCfg],is_probe_leg=is_probe_leg)

    # --------------------

    def getPTEmpty(self, flags):
        stepName = 'PTEmpty_tau'
        return self.getEmptyStep(4,stepName)

    # --------------------

    def getTrackTwoMVA(self, flags, is_probe_leg=False):
        stepName = "TrkTwoMVA_tau"
        return self.getStep(flags,5,stepName,[getTrackTwoMVACfg],is_probe_leg=is_probe_leg)

    # --------------------

    def getTrackTwoLLP(self, flags, is_probe_leg=False):
        stepName = "TrkTwoLLP_tau"
        return self.getStep(flags,5,stepName,[getTrackTwoLLPCfg],is_probe_leg=is_probe_leg)

    # --------------------
    def getTrackLRT(self, flags, is_probe_leg=False):
        stepName = "TrkLRT_tau"
        return self.getStep(flags,5,stepName,[getTrackLRTCfg],is_probe_leg=is_probe_leg)

    # --------------------

    def getIDEmpty(self, flags):
        stepName = 'IDEmpty_tau'
        return self.getEmptyStep(5,stepName)
