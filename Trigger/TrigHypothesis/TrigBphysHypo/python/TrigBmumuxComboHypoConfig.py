# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from TrigBphysHypo.TrigBmumuxComboHypoMonitoringConfig import TrigBmumuxComboHypoMonitoring, TrigBmumuxComboHypoToolMonitoring
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TriggerMenuMT.HLT.Config.MenuComponents import algorithmCAToGlobalWrapper
from AthenaConfiguration.ComponentFactory import isComponentAccumulatorCfg

from AthenaCommon.Logging import logging
log = logging.getLogger('TrigBmumuxComboHypoConfig')

def BmumuxComboHypoInternalCfg(flags):
    suffix = 'Bmumux'
    acc = ComponentAccumulator()
    from TrigBphysHypo.TrigBPhyCommonConfig import TrigBPHY_TrkVKalVrtFitterCfg
    from InDetConfig.InDetConversionFinderToolsConfig import BPHY_VertexPointEstimatorCfg
    from TrackToVertex.TrackToVertexConfig import TrackToVertexCfg

    hypo = CompFactory.TrigBmumuxComboHypo(
        name = 'BmumuxComboHypo',
        VertexFitter = acc.popToolsAndMerge(TrigBPHY_TrkVKalVrtFitterCfg(flags, suffix)),
        VertexPointEstimator = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, 'VertexPointEstimator_'+suffix)),
        TrackToVertexTool = acc.popToolsAndMerge(TrackToVertexCfg(flags,'TrackToVertexTool_'+suffix)),
        CheckMultiplicityMap = False,
        TrigBphysCollectionKey = 'HLT_Bmumux',
        MuonCollectionKey = 'HLT_Muons_Bmumux',
        TrackCollectionKey = 'HLT_IDTrack_Bmumux_IDTrig',
        DeltaR = 0.01,
        TrkZ0 = 50.,
        FitAttemptsWarningThreshold = 200,
        FitAttemptsBreakThreshold = 1000,
        # dimuon properties
        Dimuon_rejectSameChargeTracks = True,
        Dimuon_massRange = (100., 5500.),
        Dimuon_chi2 = 20.,
        # B+ -> mu+ mu- K+
        BplusToMuMuKaon = True,
        BplusToMuMuKaon_minKaonPt = 100.,
        BplusToMuMuKaon_massRange = (4500., 5900.),
        BplusToMuMuKaon_chi2 = 50.,
        # B_c+ -> J/psi(-> mu+ mu-) pi+
        BcToMuMuPion = True,
        BcToMuMuPion_minPionPt = 2000.,
        BcToMuMuPion_dimuonMassRange = (2500., 4300.),
        BcToMuMuPion_massRange = (5500., 7300.),
        BcToMuMuPion_chi2 = 50.,
        # B_s0 -> mu+ mu- phi(-> K+ K-)
        BsToMuMuPhi1020 = True,
        BsToMuMuPhi1020_rejectSameChargeTracks = True,
        BsToMuMuPhi1020_minKaonPt = 100.,
        BsToMuMuPhi1020_massRange = (4800., 5800.),
        BsToMuMuPhi1020_phiMassRange = (940., 1100.),
        BsToMuMuPhi1020_chi2 = 60.,
        # B0 -> mu+ mu- K*0(-> K+ pi-)
        BdToMuMuKstar0 = True,
        BdToMuMuKstar0_rejectSameChargeTracks = True,
        BdToMuMuKstar0_minKaonPt = 100.,
        BdToMuMuKstar0_minPionPt = 100.,
        BdToMuMuKstar0_massRange = (4600., 5900.),
        BdToMuMuKstar0_KstarMassRange = (700., 1100.),
        BdToMuMuKstar0_chi2 = 60.,
        # Lambda_b0 -> J/psi(-> mu+ mu-) p K-
        LambdaBToMuMuProtonKaon = True,
        LambdaBToMuMuProtonKaon_rejectSameChargeTracks = False,
        LambdaBToMuMuProtonKaon_minProtonPt = 1000.,
        LambdaBToMuMuProtonKaon_minKaonPt = 1000.,
        LambdaBToMuMuProtonKaon_minKstarMass = 1300.,
        LambdaBToMuMuProtonKaon_dimuonMassRange = (2500., 4300.),
        LambdaBToMuMuProtonKaon_massRange = (4800., 6400.),
        LambdaBToMuMuProtonKaon_chi2 = 60.,
        # B_c+ -> J/psi(-> mu+ mu-) D_s+(->phi(-> K+ K-) pi+)
        BcToDsMuMu = True,
        BcToDsMuMu_minKaonPt = 1000.,
        BcToDsMuMu_minPionPt = 1000.,
        BcToDsMuMu_massRange = (5500., 7300.),
        BcToDsMuMu_dimuonMassRange = (2500., 4300.),
        BcToDsMuMu_phiMassRange = (940., 1100.),
        BcToDsMuMu_DsMassRange = (1750., 2100.),
        BcToDsMuMu_chi2 = 60.,
        # B_c+ -> J/psi(-> mu+ mu-) D+(-> K- pi+ pi+)
        BcToDplusMuMu = True,
        BcToDplusMuMu_minKaonPt = 1000.,
        BcToDplusMuMu_minPionPt = 1000.,
        BcToDplusMuMu_massRange = (5500., 7300.),
        BcToDplusMuMu_dimuonMassRange = (2500., 4300.),
        BcToDplusMuMu_DplusMassRange = (1750., 2000.),
        BcToDplusMuMu_chi2 = 60.,
        # B_c+ -> J/psi(-> mu+ mu-) D*+(-> D0(-> K- pi+) pi+)
        BcToDstarMuMu = True,
        BcToDstarMuMu_makeDstar = True,
        BcToDstarMuMu_minD0KaonPt = 1000.,
        BcToDstarMuMu_minD0PionPt = 1000.,
        BcToDstarMuMu_minDstarPionPt = 500.,
        BcToDstarMuMu_maxDstarPionZ0 = 5.,
        BcToDstarMuMu_massRange = (5500., 7300.),
        BcToDstarMuMu_dimuonMassRange = (2500., 4300.),
        BcToDstarMuMu_D0MassRange = (1750., 2000.),
        BcToDstarMuMu_DstarMassRange = (-1., 2110.),
        BcToDstarMuMu_chi2 = 60.,
        MonTool = TrigBmumuxComboHypoMonitoring(flags, 'TrigBmumuxComboHypoMonitoring'))

    acc.addEventAlgo(hypo)
    return acc

def BmumuxComboHypoCfg(flags, name):
    log.debug('BmumuxComboHypoCfg.name = %s ', name)
    if not isComponentAccumulatorCfg():
        return algorithmCAToGlobalWrapper(BmumuxComboHypoInternalCfg, flags)[0]
    else :
        return BmumuxComboHypoInternalCfg(flags)


def TrigBmumuxComboHypoToolFromDict(flags, chainDict):

    topoAlgs = chainDict['chainName']
    log.debug("Set for algorithm %s", topoAlgs)
    tool = CompFactory.TrigBmumuxComboHypoTool(topoAlgs)
    decay = chainDict['topo'][-1]
    trigDecayDict = {             # xAOD::TrigBphys::pType
        'Bidperf':            6,  # MULTIMU
        'BpmumuKp':           7,  # BKMUMU
        'BcmumuPi':          21,  # BCPIMUMU
        'BsmumuPhi':          9,  # BSPHIMUMU
        'BdmumuKst':          8,  # BDKSTMUMU
        'LbPqKm':            22,  # LBPQMUMU
        'BcmumuDploose' :    13,  # BCDPMUMU
        'BcmumuDsloose' :    11,  # BCDSMUMU
        'BcmumuD0Xloose' :   19,  # DZKPI
        'BcmumuDstarloose' : 14,  # BCDSTMUMU
        'BpmuD0X' :          23,  # B2D0MUX
        'BdmuDpX' :          24,  # BD2DMMUX
        'BdmuDstarX' :       25,  # BD2DSTMUX
        'BsmuDsX' :          26,  # BS2DSMUX
        'LbmuLcX' :          27   # LB2LCMUX
    }
    tool.Decay = trigDecayDict[decay]
    tool.isBmux = True if 'bBmux' in chainDict['topo'] else False
    monGroups = ['bphysMon:online']
    if any(group in monGroups for group in chainDict['monGroups']):
        tool.MonTool = TrigBmumuxComboHypoToolMonitoring(flags, 'MonTool')
    if isComponentAccumulatorCfg():
        acc = ComponentAccumulator()
        acc.setPrivateTools(tool)
        return acc
    else:  return tool
