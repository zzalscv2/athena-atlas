# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

########################################################################
#
# SliceDef file for muon chains/signatures
#
#########################################################################
from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from ..Config.ChainConfigurationBase import ChainConfigurationBase
from AthenaConfiguration.ComponentFactory import isComponentAccumulatorCfg

if isComponentAccumulatorCfg():
    from .generateMuon import muFastSequence, muCombSequence, muEFCBSequence,  muCombOvlpRmSequence
else: 
    from .MuonMenuSequences import muFastSequence, muFastCalibSequence, mul2mtSAOvlpRmSequence, muCombSequence, muCombLRTSequence, muCombOvlpRmSequence, mul2mtCBOvlpRmSequence, mul2IOOvlpRmSequence, muEFCBSequence, muEFCBIDperfSequence, muEFCBLRTSequence, muEFCBLRTIDperfSequence, muEFCBFSSequence, muEFIsoSequence, muEFMSIsoSequence,  efLateMuSequence, muEFIDtpSequence

from .MuonMenuSequences import muEFSASequence, muEFSAFSSequence

from .MuonMenuSequences import efLateMuRoISequence, muRoiClusterSequence
from ..Config.MenuComponents import menuSequenceCAToGlobalWrapper
from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFInvMassHypoToolFromDict
from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFIdtpInvMassHypoToolFromDict

#--------------------------------------------------------
# fragments generating config will be functions in new JO
#--------------------------------------------------------
def muFastSequenceCfg(flags,is_probe_leg=False):
    return muFastSequence(flags, is_probe_leg=is_probe_leg)

def muFastCalibSequenceCfg(flags,is_probe_leg=False):
    return muFastCalibSequence(flags, is_probe_leg=is_probe_leg)

def mul2mtSAOvlpRmSequenceCfg(flags,is_probe_leg=False):
    return mul2mtSAOvlpRmSequence(flags, is_probe_leg=is_probe_leg)

def muCombSequenceCfg(flags,is_probe_leg=False):
    return muCombSequence(flags, is_probe_leg=is_probe_leg)

def muCombLRTSequenceCfg(flags,is_probe_leg=False):
    return muCombLRTSequence(flags, is_probe_leg=is_probe_leg)

def muCombOvlpRmSequenceCfg(flags,is_probe_leg=False):
    return muCombOvlpRmSequence(flags, is_probe_leg=is_probe_leg)

def mul2IOOvlpRmSequenceCfg(flags,is_probe_leg=False):
    return mul2IOOvlpRmSequence(flags, is_probe_leg=is_probe_leg)

def mul2mtCBOvlpRmSequenceCfg(flags,is_probe_leg=False):
    return mul2mtCBOvlpRmSequence(flags, is_probe_leg=is_probe_leg)

def muEFSASequenceCfg(flags,is_probe_leg=False):
    if isComponentAccumulatorCfg():
        return muEFSASequence(flags, is_probe_leg=is_probe_leg)
    else:
        return menuSequenceCAToGlobalWrapper(muEFSASequence,flags, is_probe_leg=is_probe_leg)

def muEFCBSequenceCfg(flags,is_probe_leg=False):
    return muEFCBSequence(flags, is_probe_leg=is_probe_leg)

def muEFCBIDperfSequenceCfg(flags,is_probe_leg=False):
    return muEFCBIDperfSequence(flags, is_probe_leg=is_probe_leg)

def muEFIDtpSequenceCfg(flags,is_probe_leg=False):
    return muEFIDtpSequence(flags, is_probe_leg=is_probe_leg)

def muEFCBLRTSequenceCfg(flags,is_probe_leg=False):
    return muEFCBLRTSequence(flags, is_probe_leg=is_probe_leg)

def muEFCBLRTIDperfSequenceCfg(flags,is_probe_leg=False):
    return muEFCBLRTIDperfSequence(flags, is_probe_leg=is_probe_leg)

def FSmuEFSASequenceCfg(flags,is_probe_leg=False):
    if isComponentAccumulatorCfg():
        return muEFSAFSSequence(flags, is_probe_leg=is_probe_leg)
    else:
        return menuSequenceCAToGlobalWrapper(muEFSAFSSequence, flags, is_probe_leg=is_probe_leg)

def FSmuEFCBSequenceCfg(flags,is_probe_leg=False):
    return muEFCBFSSequence(flags, is_probe_leg=is_probe_leg)

def muEFIsoSequenceCfg(flags,is_probe_leg=False):
    return muEFIsoSequence(flags, is_probe_leg=is_probe_leg)

def muEFMSIsoSequenceCfg(flags,is_probe_leg=False):
    return muEFMSIsoSequence(flags, is_probe_leg=is_probe_leg)

def muEFLateRoISequenceCfg(flags,is_probe_leg=False):
    if isComponentAccumulatorCfg():
        return efLateMuRoISequence(flags)
    else:
        return menuSequenceCAToGlobalWrapper(efLateMuRoISequence, flags)

def muEFLateSequenceCfg(flags,is_probe_leg=False):
    return efLateMuSequence(flags)

def muRoiClusterSequenceCfg(flags,is_probe_leg=False):
    if isComponentAccumulatorCfg():
        return muRoiClusterSequence(flags)
    else:
        return menuSequenceCAToGlobalWrapper(muRoiClusterSequence, flags)


#############################################
###  Class/function to configure muon chains
#############################################

class MuonChainConfiguration(ChainConfigurationBase):

    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self,chainDict)

    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------

    def assembleChainImpl(self, flags):
        chainSteps = []

        stepDictionary = self.getStepDictionary()

        is_probe_leg = self.chainPart['tnpInfo']=="probe"
        key = self.chainPart['extra']

        steps=stepDictionary[key]                

        for step in steps:
            if step:
                chainstep = getattr(self, step)(flags, is_probe_leg=is_probe_leg)
                chainSteps+=[chainstep]

        myChain = self.buildChain(chainSteps)
        return myChain

    def getStepDictionary(self):

        # --------------------
        # define here the names of the steps and obtain the chainStep configuration
        # --------------------
        doMSonly = 'msonly' in self.chainPart['msonlyInfo']
        muCombStep = 'getmuComb'
        efCBStep = 'getmuEFCB'
        if self.chainPart['isoInfo']:
            isoStep = 'getmuEFIso'
            if doMSonly:
                isoStep = 'getmuEFMSIso'
                #need to align SA and isolation steps between
                # ms-only and standard iso chains
                muCombStep = 'getmuMSEmpty'
                efCBStep = 'getEFCBEmpty'
        else:
            isoStep=None
            if doMSonly:
                #need to align final SA step between ms-only
                #and standard chains
                muCombStep = 'getmuMSEmpty'
                efCBStep = None

        stepDictionary = {            
            "":['getmuFast', muCombStep, 'getmuEFSA',efCBStep, isoStep], #RoI-based triggers
            "noL1":['getFSmuEFSA'] if doMSonly else ['getFSmuEFSA', 'getFSmuEFCB'], #full scan triggers
            "lateMu":['getLateMuRoI','getLateMu'], #late muon triggers
            "muoncalib":['getmuFast'], #calibration
            "vtx":['getmuRoiClu'], #LLP Trigger
            "mucombTag":['getmuFast', muCombStep], #Trigger for alignment 
        }

        return stepDictionary


    # --------------------
    def getmuFast(self, flags, is_probe_leg=False):

        if 'muoncalib' in self.chainPart['extra']:
           return self.getStep(flags,1,"mufastcalib", [muFastCalibSequenceCfg], is_probe_leg=is_probe_leg )
        elif 'l2mt' in self.chainPart['l2AlgInfo']:
            return self.getStep(flags,1,"mufastl2mt", [mul2mtSAOvlpRmSequenceCfg], is_probe_leg=is_probe_leg )
        else:
           return self.getStep(flags,1,"mufast", [muFastSequenceCfg], is_probe_leg=is_probe_leg )


    # --------------------
    def getmuComb(self, flags, is_probe_leg=False):

        doOvlpRm = False
        if self.chainPart['signature'] == 'Bphysics':
           doOvlpRm = False
        elif self.mult>1:
           doOvlpRm = True
        elif len( self.dict['signatures'] )>1 and not self.chainPart['extra']:
           doOvlpRm = True
        else:
           doOvlpRm = False

        if 'l2mt' in self.chainPart['l2AlgInfo']:
            return self.getStep(flags,2,"muCombl2mt", [mul2mtCBOvlpRmSequenceCfg], is_probe_leg=is_probe_leg )
        elif 'l2io' in self.chainPart['l2AlgInfo']:
            return self.getStep(flags,2, 'muCombIO', [mul2IOOvlpRmSequenceCfg], is_probe_leg=is_probe_leg )
        elif doOvlpRm:
           return self.getStep(flags,2, 'muComb', [muCombOvlpRmSequenceCfg], is_probe_leg=is_probe_leg )
        elif "LRT" in self.chainPart['addInfo']:
           return self.getStep(flags,2, 'muCombLRT', [muCombLRTSequenceCfg], is_probe_leg=is_probe_leg )
        else:
           return self.getStep(flags,2, 'muComb', [muCombSequenceCfg], is_probe_leg=is_probe_leg )

    # --------------------
    def getmuCombIO(self, flags, is_probe_leg=False):
        return self.getStep(flags,2, 'muCombIO', [mul2IOOvlpRmSequenceCfg], is_probe_leg=is_probe_leg )

    # --------------------
    def getmuEFSA(self, flags, is_probe_leg=False):
        return self.getStep(flags,3,'muEFSA',[ muEFSASequenceCfg], is_probe_leg=is_probe_leg)

    # --------------------
    def getmuEFCB(self, flags, is_probe_leg=False):

        if 'invm' in self.chainPart['invMassInfo']: # No T&P support, add if needed
            return self.getStep(flags,4,'EFCB', [muEFCBSequenceCfg], comboTools=[TrigMuonEFInvMassHypoToolFromDict], is_probe_leg=is_probe_leg)
        elif "LRT" in self.chainPart['addInfo']:
            if "idperf" in self.chainPart['addInfo']:
                return self.getStep(flags,4,'EFCBLRTIDPERF', [muEFCBLRTIDperfSequenceCfg], is_probe_leg=is_probe_leg)
            else:
                return self.getStep(flags,4,'EFCBLRT', [muEFCBLRTSequenceCfg], is_probe_leg=is_probe_leg)
        elif "idperf" in self.chainPart['addInfo']:
            return self.getStep(flags,4,'EFCBIDPERF', [muEFCBIDperfSequenceCfg], is_probe_leg=is_probe_leg)
        elif "idtp" in self.chainPart['addInfo']:
            return self.getStep(flags,4,'EFIDTP', [muEFIDtpSequenceCfg], is_probe_leg=is_probe_leg)
        else:
            return self.getStep(flags,4,'EFCB', [muEFCBSequenceCfg], is_probe_leg=is_probe_leg)

    # --------------------
    def getFSmuEFSA(self, flags, is_probe_leg=False):
        return self.getStep(flags,5,'FSmuEFSA', [FSmuEFSASequenceCfg], is_probe_leg=is_probe_leg)

    # --------------------
    def getFSmuEFCB(self, flags, is_probe_leg=False):
        if 'invm' in self.chainPart['invMassInfo']:
            return self.getStep(flags,6,'FSmuEFCB', [FSmuEFCBSequenceCfg],comboTools=[TrigMuonEFInvMassHypoToolFromDict], is_probe_leg=is_probe_leg)
        else:
            return self.getStep(flags,6,'FSmuEFCB', [FSmuEFCBSequenceCfg], is_probe_leg=is_probe_leg)

    #---------------------
    def getmuEFIso(self, flags, is_probe_leg=False):
        if any(x in self.dict['topo'] for x in ['b7invmAB9vtx20', 'b11invmAB60vtx20', 'b11invmAB24vtx20', 'b24invmAB60vtx20']):
            from TrigBphysHypo.TrigMultiTrkComboHypoConfig import DrellYanComboHypoCfg, TrigMultiTrkComboHypoToolFromDict
            return self.getStep(flags,5,'muEFIsoDY', [muEFIsoSequenceCfg], comboHypoCfg=DrellYanComboHypoCfg, comboTools=[TrigMultiTrkComboHypoToolFromDict], is_probe_leg=is_probe_leg)
        else:
            return self.getStep(flags,5,'muEFIso', [muEFIsoSequenceCfg], is_probe_leg=is_probe_leg)

    #---------------------
    def getmuEFMSIso(self, flags, is_probe_leg=False):
        return self.getStep(flags,5,'muEFMSIso',[ muEFMSIsoSequenceCfg], is_probe_leg=is_probe_leg)

    #--------------------
    def getmuMSEmptyAll(self, flags, stepID): # No T&P info needed for empty step?
        return self.getEmptyStep(stepID,'muMS_empty')

    #--------------------
    def getmuMSEmpty(self, flags, is_probe_leg=False): # No T&P info needed for empty step?
        return self.getmuMSEmptyAll(flags, 2)

    #--------------------
    def getmuFastEmpty(self, flags, is_probe_leg=False): # No T&P info needed for empty step?
        return self.getEmptyStep(1,'muFast_empty')

    #--------------------
    def getEFCBEmpty(self, flags, is_probe_leg=False): # No T&P info needed for empty step?
        return self.getEmptyStep(4,'muefCB_Empty')

    #--------------------
    def getLateMuRoI(self, flags, is_probe_leg=False): # No T&P support, add if needed
        return self.getStep(flags,1,'muEFLateRoI',[muEFLateRoISequenceCfg], is_probe_leg=is_probe_leg)

    #--------------------
    def getLateMu(self, flags, is_probe_leg=False): # No T&P support, add if needed
        return self.getStep(flags,2,'muEFLate',[muEFLateSequenceCfg], is_probe_leg=is_probe_leg)

    #--------------------
    def getmuRoiClu(self, flags, is_probe_leg=False):
        return self.getStep(flags,1,'muRoiClu',[muRoiClusterSequenceCfg])


def TrigMuonEFIdtpInvMassHypoToolCfg(flags, chainDict):
    tool = TrigMuonEFIdtpInvMassHypoToolFromDict(flags, chainDict)
    return tool


