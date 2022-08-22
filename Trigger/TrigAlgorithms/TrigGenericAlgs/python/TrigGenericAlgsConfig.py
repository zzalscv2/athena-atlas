# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from TrigPartialEventBuilding.TrigPartialEventBuildingConfig import getRegSelTools

class TimeBurnerCfg(CompFactory.TimeBurner):
    def __init__(self, name="TimeBurner", **kwargs):
        super(TimeBurnerCfg, self).__init__(name, **kwargs)
        # Decorate the Configurable with a HypoTools property which is only required
        # by the menu and python configuration framework, but has no use in C++ TimeBurner
        self.HypoTools = []


def TimeBurnerHypoToolGen(chainDict):
    # Dummy HypoTool implementing only the functions used by the menu and python configuration framework
    class DummyHypo:
        def __init__(self, name):
            self.name = name
        def getName(self):
            return self.name

    return DummyHypo(chainDict['chainName'])


def EndOfEventROIConfirmerAlgCfg(name):
    return CompFactory.EndOfEventROIConfirmerAlg(name)

def EndOfEventFilterAlgCfg(name, chainName):
    return CompFactory.EndOfEventFilterAlg(name, ChainName=chainName)

def TrigEventInfoRecorderAlgCfg(name):
    return CompFactory.TrigEventInfoRecorderAlg(name)

def L1CorrelationMonitoringCfg(name):
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool('MonTool')
    monTool.defineHistogram('EF_L1Corr_beforeafterflag', path='EXPERT', type='TH1F', title='beforeafterflag', xbins=4, xmin=-1.5, xmax=2.5)
    monTool.defineHistogram('EF_L1Corr_l1a_type, EF_L1Corr_other_type', path='EXPERT', type='TH2F', title="typeMatrix ; L1A; Other", xbins=8, xmin=-0.5, xmax=7.5, ybins=8, ymin=-0.5, ymax=7.5)
    return monTool

def L1CorrelationAlgCfg(name, **kwargs):
    kwargs.setdefault("MonTool",L1CorrelationMonitoringCfg("L1CorrelationAlg"))
    return CompFactory.L1CorrelationAlg(name, **kwargs)

def ROBPrefetchingAlgCfg(flags, name, inputMaker=None, regSelDets=[]):
    alg = CompFactory.ROBPrefetchingAlg(name)
    alg.RegionSelectorTools = getRegSelTools(flags, regSelDets)

    return alg

def ROBPrefetchingAlgCfg_Si(flags, nameSuffix, inputMaker=None):
    return ROBPrefetchingAlgCfg(flags, 'ROBPrefetchingAlg_Si_'+nameSuffix, inputMaker, ['Pixel', 'SCT'])

def ROBPrefetchingAlgCfg_Calo(flags, nameSuffix, inputMaker=None):
    return ROBPrefetchingAlgCfg(flags, 'ROBPrefetchingAlg_Calo_'+nameSuffix, inputMaker, ['TTEM', 'TTHEC', 'FCALEM', 'FCALHAD', 'TILE'])

def ROBPrefetchingAlgCfg_Muon(flags, nameSuffix, inputMaker=None):
    return ROBPrefetchingAlgCfg(flags, 'ROBPrefetchingAlg_Muon_'+nameSuffix, inputMaker, ['MDT', 'RPC', 'TGC', 'CSC', 'MM', 'sTGC'])
