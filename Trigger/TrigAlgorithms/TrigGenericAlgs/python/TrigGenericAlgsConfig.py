# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TrigPartialEventBuilding.TrigPartialEventBuildingConfig import getRegSelTools
from AthenaCommon.Logging import logging
_log = logging.getLogger( __name__ )


def TimeBurnerCfg(flags, name="TimeBurner", **kwargs):
    return CompFactory.TimeBurner(name, **kwargs)

def TimeBurnerHypoToolGen(chainDict):
    # Dummy HypoTool (it is not even called by TimeBurner)
    return CompFactory.TrigGenericHypoTool(chainDict['chainName'],
                                           PassString = "")

def EndOfEventROIConfirmerAlgCfg(name):
    return CompFactory.EndOfEventROIConfirmerAlg(name)

def EndOfEventFilterAlgCfg(name, chainName):
    return CompFactory.EndOfEventFilterAlg(name, ChainName=chainName)

def TrigEventInfoRecorderAlgCfg(flags, name, trigEventInfoKey, decoratePFlowInfo, decorateEMTopoInfo, renounceAll=False, primaryVertexInputName="HLT_IDVertex_FS", RhoKey_PFlow = 'HLT_Kt4EMPFlowEventShape', RhoKey_EMTopo = 'HLT_Kt4EMTopoEventShape'):
    acc = ComponentAccumulator()
    alg = CompFactory.TrigEventInfoRecorderAlg(name,
          trigEventInfoKey = trigEventInfoKey,
          decoratePFlowInfo = decoratePFlowInfo,
          decorateEMTopoInfo = decorateEMTopoInfo,
          renounceAll = renounceAll,
          primaryVertexInputName = primaryVertexInputName,
          RhoKey_PFlow = RhoKey_PFlow,
          RhoKey_EMTopo = RhoKey_EMTopo,
          )
    acc.addEventAlgo(alg, primary=True)
    return acc

def L1CorrelationMonitoringCfg(flags, name):
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, 'MonTool')
    monTool.defineHistogram('EF_L1Corr_beforeafterflag', path='EXPERT', type='TH1F', title='beforeafterflag', xbins=4, xmin=-1.5, xmax=2.5)
    monTool.defineHistogram('EF_L1Corr_l1a_type, EF_L1Corr_other_type', path='EXPERT', type='TH2F', title="typeMatrix ; L1A; Other", xbins=8, xmin=-0.5, xmax=7.5, ybins=8, ymin=-0.5, ymax=7.5)
    return monTool

def L1CorrelationAlgCfg(flags, name, **kwargs):
    kwargs.setdefault("MonTool",L1CorrelationMonitoringCfg(flags, "L1CorrelationAlg"))
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

def getChainsForPrefetching(chains):
    from AthenaCommon.Configurable import Configurable
    from TrigConfHLTUtils.HLTUtils import string2hash
    from collections import defaultdict

    def sequenceAlgs(seq):
        algs = []
        # getChildren is only for legacy AthSequencer
        # TODO remove in legacy cleanup
        for alg in seq.getChildren() if hasattr(seq,'getChildren') else seq.Members:
            conf = Configurable.allConfigurables[alg] if type(alg)==str else alg
            if type(conf)==CompFactory.AthSequencer:
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
    return chainFilterMap


# Legacy searches for the algs through the top level sequence
def configurePrefetchingInitialRoI(flags, chains):
    from AthenaCommon.AlgSequence import AlgSequence
    from AthenaCommon.CFElements import findSubSequence, findAlgorithm
    chainFilterMap = getChainsForPrefetching(chains)

    hltBeginSeq = findSubSequence(AlgSequence(), 'HLTBeginSeq')
    for det,chainFilter in chainFilterMap.items():
        prefetchAlg = findAlgorithm(hltBeginSeq, f'ROBPrefetchingAlg_{det}_initialRoI')
        if not chainFilter:
            # Empty filter means unconditional prefetching
            # - prevent this by adding a non-existent hash to the list which effectively disables the prefetching alg
            _log.info('No chains matched to %s - forcing ChainFilter=[0] to disable this alg\'s prefetching', prefetchAlg.getName())
            chainFilter = [0]

        prefetchAlg.ChainFilter = chainFilter


# In CA we have no access to the HLTBeginSeq in here
# Instead generate the prefetching alg configs and append the ChainFilter,
# to be merged into the global config.
def prefetchingInitialRoIConfig(flags, chains):
    chainFilterMap = getChainsForPrefetching(chains)
    configurators = {
        'Si':   ROBPrefetchingAlgCfg_Si,
        'Calo': ROBPrefetchingAlgCfg_Calo,
        'Muon': ROBPrefetchingAlgCfg_Muon,
    }

    prefetchCfg = ComponentAccumulator()

    for det,chainFilter in chainFilterMap.items():
        if not chainFilter:
            # Empty filter means unconditional prefetching
            # - prevent this by adding a non-existent hash to the list which effectively disables the prefetching alg
            _log.info('No chains matched to ROBPrefetchingAlg_%s_initialRoI - forcing ChainFilter=[0] to disable this alg\'s prefetching', det)
            chainFilter = [0]
        prefetchAlg = configurators[det](flags, 'initialRoI', ChainFilter=chainFilter)
        prefetchCfg.merge(prefetchAlg)

    return prefetchCfg
