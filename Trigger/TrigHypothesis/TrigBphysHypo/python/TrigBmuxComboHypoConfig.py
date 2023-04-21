# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from TrigBphysHypo.TrigBmuxComboHypoMonitoringConfig import TrigBmuxComboHypoMonitoring
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TriggerMenuMT.HLT.Config.MenuComponents import algorithmCAToGlobalWrapper
from AthenaConfiguration.ComponentFactory import isComponentAccumulatorCfg

from AthenaCommon.Logging import logging
log = logging.getLogger('TrigBmuxComboHypoConfig')

def BmuxComboHypoInternalCfg(flags):
    suffix = 'Bmux'
    acc = ComponentAccumulator()
    from TrigBphysHypo.TrigBPhyCommonConfig import TrigBPHY_TrkVKalVrtFitterCfg
    from TrackToVertex.TrackToVertexConfig import TrackToVertexCfg
    from InDetConfig.InDetConversionFinderToolsConfig import BPHY_VertexPointEstimatorCfg

    hypo = CompFactory.TrigBmuxComboHypo(
        name = 'BmuxComboHypo',
        VertexFitter = acc.popToolsAndMerge(TrigBPHY_TrkVKalVrtFitterCfg(flags, suffix)),
        VertexPointEstimator = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, 'VertexPointEstimator_'+suffix)),
        TrackToVertexTool = acc.popToolsAndMerge(TrackToVertexCfg(flags,'TrackToVertexTool_'+suffix)),
        CheckMultiplicityMap = False,
        TrigBphysCollectionKey = 'HLT_Bmux',
        MuonCollectionKey = 'HLT_Muons_Bmumux',
        TrackCollectionKey = 'HLT_IDTrack_Bmumux_IDTrig',
        MakeCascadeFit = True,
        CascadeChi2 = 50.,
        DeltaR = 0.01,
        TrkZ0 = 10.,
        RoiEtaWidth = 0.75,
        RoiPhiWidth = 0.75,
        FitAttemptsWarningThreshold = 200,
        FitAttemptsBreakThreshold = 1000,
        # B+ -> mu+ nu_mu anti-D0(-> K+ pi-) and B0 -> mu+ nu_mu D*-(-> anti-D0(-> K+ pi-) pi-)
        BToD0 = True,
        BToD0_makeDstar = True,
        BToD0_minD0KaonPt = 1000.,
        BToD0_minD0PionPt = 1000.,
        BToD0_minD0Pt = -1.,
        BToD0_minDstarPt = 5500.,
        BToD0_minDstarPionPt = 1000.,
        BToD0_maxDstarPionZ0 = 5.,
        BToD0_massRange = (-1., 10000.),
        BToD0_D0MassRange = (1750., 2000.),
        BToD0_DstarMassRange = (-1., 2110.),
        BToD0_chi2 = 20.,
        BToD0_LxyB = 0.1,
        BToD0_LxyBd = 0.05,
        BToD0_LxyD0 = 0.,
        # B0 -> mu+ nu_mu D-(-> K+ pi- pi-)
        BdToD = True,
        BdToD_minKaonPt = 1250.,
        BdToD_minPionPt = 1000.,
        BdToD_minDPt = 5500.,
        BdToD_massRange = (-1., 10000.),
        BdToD_DMassRange = (1750., 2000.),
        BdToD_chi2 = 27.,
        BdToD_LxyBd = 0.1,
        BdToD_LxyD = 0.05,
        # B_s0 -> mu+ nu_mu D_s-(->phi(-> K+ K-) pi-)
        BsToDs = True,
        BsToDs_minKaonPt = 1000.,
        BsToDs_minPionPt = 1000.,
        BsToDs_minDsPt = 5500.,
        BsToDs_massRange = (-1., 10000.),
        BsToDs_phiMassRange = (940., 1100.),
        BsToDs_DsMassRange = (1750., 2100.),
        BsToDs_chi2 = 27.,
        BsToDs_LxyBs = 0.1,
        BsToDs_LxyDs = 0.02,
        # Lambda_b0 -> mu+ nu_mu anti-Lambda_c-(-> anti-p K+ pi-)
        LambdaBToLambdaC = True,
        LambdaBToLambdaC_minProtonPt = 2750.,
        LambdaBToLambdaC_minKaonPt = 1250.,
        LambdaBToLambdaC_minPionPt = 1000.,
        LambdaBToLambdaC_minLambdaCPt = 5500.,
        LambdaBToLambdaC_massRange = (-1., 10000.),
        LambdaBToLambdaC_LambdaCMassRange = (2190., 2390.),
        LambdaBToLambdaC_chi2 = 27.,
        LambdaBToLambdaC_LxyLb = 0.1,
        LambdaBToLambdaC_LxyLc = 0.02,
        MonTool = TrigBmuxComboHypoMonitoring(flags, 'TrigBmuxComboHypoMonitoring'))
    acc.addEventAlgo(hypo)
    return acc

def BmuxComboHypoCfg(flags, name):
    log.debug('BmuxComboHypoCfg.name = %s ', name)
    if not isComponentAccumulatorCfg():
        return algorithmCAToGlobalWrapper(BmuxComboHypoInternalCfg, flags)[0]
    else:
        return BmuxComboHypoInternalCfg(flags)
