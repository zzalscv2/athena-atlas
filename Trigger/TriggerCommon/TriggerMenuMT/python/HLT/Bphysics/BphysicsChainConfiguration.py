# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

########################################################################
#
# SliceDef file for bphysics chains/signatures
#
#########################################################################
from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)
from ..Config.MenuComponents import menuSequenceCAToGlobalWrapper
from ..Config.ChainConfigurationBase import ChainConfigurationBase
from ..Muon.MuonChainConfiguration import MuonChainConfiguration
from AthenaConfiguration.ComponentFactory import isComponentAccumulatorCfg

from ..Muon.MuonChainConfiguration import mul2IOOvlpRmSequenceCfg, mul2mtCBOvlpRmSequenceCfg, muEFCBSequenceCfg

if isComponentAccumulatorCfg():
    def StreamerDimuL2ComboHypoCfg(): pass
    def StreamerDimuL2IOComboHypoCfg(): pass
    def StreamerDimuL2MTComboHypoCfg():pass
    def DimuEFComboHypoCfg():pass
    def BmutrkComboHypoCfg():pass
    def StreamerDimuEFComboHypoCfg():pass
    def TrigMultiTrkComboHypoToolFromDict():pass
    def BmumuxComboHypoCfg(): pass
    def BmuxComboHypoCfg(): pass
    def TrigBmumuxComboHypoToolFromDict(): pass
else:
    from .BphysicsMenuSequences import dimuL2Sequence, dimuEFSequence, bmumuxSequence

    from TrigBphysHypo.TrigMultiTrkComboHypoConfig import StreamerDimuL2ComboHypoCfg, StreamerDimuL2IOComboHypoCfg, StreamerDimuL2MTComboHypoCfg, DimuEFComboHypoCfg, BmutrkComboHypoCfg, StreamerDimuEFComboHypoCfg, TrigMultiTrkComboHypoToolFromDict
    from TrigBphysHypo.TrigBmumuxComboHypoConfig import BmumuxComboHypoCfg, TrigBmumuxComboHypoToolFromDict
    from TrigBphysHypo.TrigBmuxComboHypoConfig import BmuxComboHypoCfg

#--------------------------------------------------------
# fragments generating config will be functions in new JO
# I have no idea what the above sentence means - copy/paste from muons...
#--------------------------------------------------------

def dimuL2SequenceCfg(flags):
    if isComponentAccumulatorCfg():
        return dimuL2Sequence(flags)
    else:
        return menuSequenceCAToGlobalWrapper(dimuL2Sequence,flags)

def dimuEFSequenceCfg(flags):
    if isComponentAccumulatorCfg():
        return dimuEFSequence(flags)
    else:
        return menuSequenceCAToGlobalWrapper(dimuEFSequence, flags)

def bmumuxSequenceCfg(flags):
    if isComponentAccumulatorCfg():
        return bmumuxSequence(flags)
    else:
        return menuSequenceCAToGlobalWrapper(bmumuxSequence, flags)

#############################################
###  Class/function to configure muon chains
#############################################

class BphysicsChainConfiguration(MuonChainConfiguration):

    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self,chainDict)

    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------
    def assembleBphysChain(self, flags):

        log.debug("Assembling chain for %s", self.chainName)

        stepDictionary = self.getBphysStepDictionary()
        key = self.getBphysKey()
        steps=stepDictionary[key]

        chainSteps = []
        for step_level in steps:
            for step in step_level:
                chainStep = getattr(self, step)(flags)
                chainSteps+=[chainStep]

        chain = self.buildChain(chainSteps)
        return chain

    def getBphysStepDictionary(self):

        stepDictionary = {
            'dimu'   : [['getmuFast', 'getDimuL2'], ['getmuEFSA', 'getmuEFCB', 'getDimuEF']],
            'bmumux' : [['getmuFast', 'getDimuL2'], ['getmuEFSA', 'getDimuEFCB', 'getBmumux']],
            'bmutrk' : [['getmuFast', 'getmuCombIO'], ['getmuEFSA', 'getmuEFCB', 'getBmutrk']],
            'bmux'   : [['getmuFast', 'getmuCombIO'], ['getmuEFSA', 'getmuEFCB', 'getBmux']],
        }
        return stepDictionary

    def getBphysKey(self):

        the_topo = self.dict['topo'][0]

        topo_dict = {
            'bJpsimumu'  : 'dimu',
            'bJpsi'      : 'dimu',
            'bJpsimutrk' : 'bmutrk',
            'bUpsimumu'  : 'dimu',
            'bUpsi'      : 'dimu',
            'bBmumu'     : 'dimu',
            'bDimu'      : 'dimu',
            'bDimu2700'  : 'dimu',
            'bDimu6000'  : 'dimu',
            'bPhi'       : 'dimu',
            'bTau'       : 'dimu',
            'b3mu'       : 'dimu',
            'bBmux'      : 'bmux',
            'bBmumux'    : 'bmumux',
            'b0dRAB12vtx20' : 'dimu',
            'b0dRAB207invmAB22vtx20' : 'dimu',
            'b0dRAB127invmAB22vtx20' : 'dimu',
            'b7invmAB22vtx20' : 'dimu',
        }

        return topo_dict[the_topo]

    def getDimuL2(self, flags):
        if 'noL2Comb' in self.chainPart['extra']:
            return self.getStep(flags, 2, 'dimuL2', [dimuL2SequenceCfg], comboHypoCfg=StreamerDimuL2ComboHypoCfg)
        elif 'l2mt' in self.chainPart['l2AlgInfo']:
            return self.getStep(flags, 2, 'dimuL2MT', [mul2mtCBOvlpRmSequenceCfg], comboHypoCfg=StreamerDimuL2MTComboHypoCfg)
        else:
            return self.getStep(flags, 2, 'dimuL2IO', [mul2IOOvlpRmSequenceCfg], comboHypoCfg=StreamerDimuL2IOComboHypoCfg)

    def getDimuEF(self, flags):
        return self.getStep(flags, 5, 'dimuEF', [dimuEFSequenceCfg], comboHypoCfg=DimuEFComboHypoCfg, comboTools=[TrigMultiTrkComboHypoToolFromDict])

    def getDimuEFCB(self, flags):
        return self.getStep(flags, 4, 'dimuEFCB', [muEFCBSequenceCfg], comboHypoCfg=StreamerDimuEFComboHypoCfg)

    def getBmux(self, flags):
        return self.getStep(flags, 5, 'bmux', [bmumuxSequenceCfg], comboHypoCfg=BmuxComboHypoCfg, comboTools=[TrigBmumuxComboHypoToolFromDict])

    def getBmumux(self, flags):
        return self.getStep(flags, 5, 'bmumux', [bmumuxSequenceCfg], comboHypoCfg=BmumuxComboHypoCfg, comboTools=[TrigBmumuxComboHypoToolFromDict])

    def getBmutrk(self, flags):
        return self.getStep(flags, 5, 'bmutrk', [bmumuxSequenceCfg], comboHypoCfg=BmutrkComboHypoCfg, comboTools=[TrigMultiTrkComboHypoToolFromDict])
