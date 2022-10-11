# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TrigPartialEventBuilding.TrigPartialEventBuildingConfig import getRegSelTools
from AthenaCommon.Logging import logging
_log = logging.getLogger( __name__ )

# The hack below works only for old configurables, because Conf2 uses __slots__ and cannot be extended
from TrigGenericAlgs.TrigGenericAlgsConf import TimeBurner
class TimeBurnerCfg(TimeBurner):
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

def ROBPrefetchingAlgCfg(flags, name, regSelDets=[], **kwargs):
    acc = ComponentAccumulator()
    alg = CompFactory.ROBPrefetchingAlg(name, **kwargs)
    alg.RegionSelectorTools = acc.popToolsAndMerge(getRegSelTools(flags, regSelDets))
    acc.addEventAlgo(alg, primary=True)

    return acc

def ROBPrefetchingAlgCfg_Si(flags, nameSuffix, **kwargs):
    return ROBPrefetchingAlgCfg(flags, 'ROBPrefetchingAlg_Si_'+nameSuffix, ['Pixel', 'SCT'], **kwargs)

def ROBPrefetchingAlgCfg_Calo(flags, nameSuffix, **kwargs):
    return ROBPrefetchingAlgCfg(flags, 'ROBPrefetchingAlg_Calo_'+nameSuffix, ['TTEM', 'TTHEC', 'FCALEM', 'FCALHAD', 'TILE'], **kwargs)

def ROBPrefetchingAlgCfg_Muon(flags, nameSuffix, **kwargs):
    return ROBPrefetchingAlgCfg(flags, 'ROBPrefetchingAlg_Muon_'+nameSuffix, ['MDT', 'RPC', 'TGC', 'CSC', 'MM', 'sTGC'], **kwargs)

def configurePrefetchingInitialRoI(flags, chains):
    from AthenaCommon.AlgSequence import AthSequencer, AlgSequence
    from AthenaCommon.CFElements import findSubSequence, findAlgorithm
    from AthenaCommon.Configurable import Configurable
    from TrigConfHLTUtils.HLTUtils import string2hash
    from collections import defaultdict

    def sequenceAlgs(seq):
        algs = []
        for alg in seq.getChildren():
            conf = Configurable.allConfigurables[alg] if type(alg)==str else alg
            if type(conf)==AthSequencer:
                algs.extend(sequenceAlgs(conf))
            elif conf.getName().startswith('IMEmpty'):
                # skip empty probe step in tag&probe chains
                continue
            else:
                algs.append(conf.getName())
        return algs

    def firstNonEmptyStepAlgs(chainConfig):
        algsMap = defaultdict(list) # {chainLegName, algsInFirstNonEmptyStep}
        for step in chainConfig.steps:
            for legName,menuSeq in zip(step.getChainLegs(), step.sequences):
                algsMap[legName] = sequenceAlgs(menuSeq.sequence.Alg)
                if algsMap[legName]:
                    # only consider the first non-empty sequence across all legs - once found, break the loop
                    return algsMap
        return algsMap

    detGroupIdentifierAlgs = {
        'Si' : ['PixelRawDataProvider','SCTRawDataProvider'],
        'Calo' : ['HLTCaloCellMaker','FastCaloL2EgammaAlg'],
        'Muon' : ['RpcRawDataProvider','TgcRawDataProvider','MdtRawDataProvider','sTgcRawDataProvider','MMRawDataProvider']
    }

    def algsToDetGroup(algs):
        groups = []
        for group,idAlgs in detGroupIdentifierAlgs.items():
            if any([ida in algName for algName in algs for ida in idAlgs]):
                groups.append(group)
        if len(groups)>1:
            raise RuntimeError(f'Multiple detector groups: {groups:s} matched to the list of algs: {algs:s}')
        return groups[0] if groups else None

    chainFilterMap = { # {DetGroup, list of chain leg hashes}
        'Si': [],
        'Calo': [],
        'Muon': []
    }

    for chain in chains:
        algsMap = firstNonEmptyStepAlgs(chain)
        for legName,algs in algsMap.items():
            det = algsToDetGroup(algs)
            if not det:
                continue
            _log.debug("%s initialRoI will prefetch %s", legName, det)
            chainFilterMap[det].append(string2hash(legName))

    hltBeginSeq = findSubSequence(AlgSequence(), 'HLTBeginSeq')
    for det,chainFilter in chainFilterMap.items():
        prefetchAlg = findAlgorithm(hltBeginSeq, f'ROBPrefetchingAlg_{det}_initialRoI')
        if not chainFilter:
            # Empty filter means unconditional prefetching
            # - prevent this by adding a non-existent hash to the list which effectively disables the prefetching alg
            _log.info('No chains matched to %s - forcing ChainFilter=[0] to disable this alg\'s prefetching', prefetchAlg.getName())
            chainFilter = [0]

        prefetchAlg.ChainFilter = chainFilter
