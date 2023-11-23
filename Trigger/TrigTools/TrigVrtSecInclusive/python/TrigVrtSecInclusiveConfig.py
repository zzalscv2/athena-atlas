# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def TrigVrtSecInclusiveMonitoring(flags, name):

    def addTrackPairHistograms(montool):
        montool.defineHistogram('pair_dphi',     path='EXPERT',type='TH1F',title="Delta phi of track pair",        xbins = 132, xmin=-0.1, xmax=3.2)
        montool.defineHistogram('pair_dr',       path='EXPERT',type='TH1F',title="Delta r of track pair",          xbins = 132, xmin=-0.1, xmax=3.2)
        montool.defineHistogram('intersect_r',   path='EXPERT',type='TH1F',title="R of two circles intersections (mm)", xbins = 200, xmin=0, xmax=600)
        montool.defineHistogram('init_r',        path='EXPERT',type='TH1F',title="R of initial vertex (mm)",       xbins = 200, xmin=0, xmax=600)
        montool.defineHistogram('init_trkd0',    path='EXPERT',type='TH1F',title="Trk d0 wrt initial vertex",      xbins = 200, xmin=0, xmax=200)
        montool.defineHistogram('init_trkz0',    path='EXPERT',type='TH1F',title="Trk z0 wrt initial vertex",      xbins = 100, xmin=0, xmax=100)
        montool.defineHistogram('vtxfit_chi2',   path='EXPERT',type='TH1F',title="Chi2 of fitted vertex",          xbins = 100, xmin=0, xmax=5)
        montool.defineHistogram('vtxfit_r',      path='EXPERT',type='TH1F',title="R of fitted vertex (mm)",        xbins = 200, xmin=0, xmax=600)
        montool.defineHistogram('vtx_chi2',      path='EXPERT',type='TH1F',title="Chi2 of vertex",                 xbins = 100, xmin=0, xmax=5)
        montool.defineHistogram('vtx_mass',      path='EXPERT',type='TH1F',title="Mass of vertex (GeV)",           xbins = 200, xmin=0, xmax=5)
        montool.defineHistogram('vtx_mass_high', path='EXPERT',type='TH1F',title="Mass of vertex (GeV, high)",     xbins = 200, xmin=0, xmax=100)
        montool.defineHistogram('vtx_pt',        path='EXPERT',type='TH1F',title="Pt of vertex (GeV)",             xbins = 100, xmin=0, xmax=100)
        montool.defineHistogram('vtx_charge',    path='EXPERT',type='TH1F',title="Charge of vertex",               xbins =   5, xmin=-2.5,xmax=2.5)
        montool.defineHistogram('vtx_r',         path='EXPERT',type='TH1F',title="R of vertex (mm)",               xbins = 200, xmin=0, xmax=600)

    def addTimingHistograms(montool):
        montool.defineHistogram('TIME_TrackSel',       path='EXPERT',type='TH1F',title="Track selection time (ns)",                xbins = 200, xmin=0.0, xmax=1000.0)
        montool.defineHistogram('TIME_TwoTrackVertex', path='EXPERT',type='TH1F',title="Two track vertexing time (ms)",            xbins = 200, xmin=0.0, xmax=1000.0)
        montool.defineHistogram('TIME_MapClustering',  path='EXPERT',type='TH1F',title="Time spent in vertex map clustering (us)", xbins = 200, xmin=0.0, xmax=10000.0)
        montool.defineHistogram('TIME_NTrackVertex',   path='EXPERT',type='TH1F',title="Time spent in Ntrack vertexing (ms)",      xbins = 200, xmin=0.0, xmax=50.0)
        montool.defineHistogram('TIME_NTrackVtxOffVSI',path='EXPERT',type='TH1F',title="Time spent in Ntrack vertexing (ms)",      xbins = 200, xmin=0.0, xmax=1000.0)
        montool.defineHistogram('TIME_CleanUp',        path='EXPERT',type='TH1F',title="Time spent in vertex cleaning (ns)",       xbins = 200, xmin=0.0, xmax=10000.0)
        montool.defineHistogram('TIME_WriteVertices',  path='EXPERT',type='TH1F',title="Time spent in writing vertex out (us)",    xbins = 200, xmin=0.0, xmax=100.0)
        montool.defineHistogram('TIME_Overall',        path='EXPERT',type='TH1F',title="Time spent over all vertexing (ms)",       xbins = 200, xmin=0.0, xmax=1000.0)

        montool.defineHistogram('TIME_TwoCircIntsect', path='EXPERT',type='TH1F',title="Two circles intersection time (ns) / Di-track vertexing", xbins = 200, xmin=0.0, xmax=1000.0)
        montool.defineHistogram('TIME_VrtFitFast',     path='EXPERT',type='TH1F',title="Two track vertexing fast time (ns) / Di-track vertexing", xbins = 200, xmin=0.0, xmax=10000.0)
        montool.defineHistogram('TIME_VrtFit',         path='EXPERT',type='TH1F',title="Two track vertexing time (us) / Di-track vertexing",      xbins = 200, xmin=0.0, xmax=2000.0)

        montool.defineHistogram('TIME_RetrvFromMap',   path='EXPERT',type='TH1F',title="Time spent in track list retrieving from the map (ns) / N-track vertexing", xbins = 200, xmin=0.0, xmax=10000.0)
        montool.defineHistogram('TIME_MergeParGraph',  path='EXPERT',type='TH1F',title="Time spent in vertex merging using partial graph (us) / N-track vertexing", xbins = 200, xmin=0.0, xmax=1000.0)
        montool.defineHistogram('TIME_MergeSimple',    path='EXPERT',type='TH1F',title="Time spent in simple vertex merging (us) / N-track vertexing",              xbins = 200, xmin=0.0, xmax=1000.0)


    montool = GenericMonitoringTool(flags, HistPath = f"TrigVrtSecInclusive_{name}")
    addTrackPairHistograms(montool)
    addTimingHistograms(montool)

    return montool

def TrigVrtSecInclusive_VSIMonitoring(flags, name):

    def addTrackPairHistograms(montool):
        montool.defineHistogram('pair_dphi',     path='EXPERT',type='TH1F',title="Delta phi of track pair",        xbins = 132, xmin=-0.1, xmax=3.2)
        montool.defineHistogram('pair_dr',       path='EXPERT',type='TH1F',title="Delta r of track pair",          xbins = 132, xmin=-0.1, xmax=3.2)
        montool.defineHistogram('intersect_r',   path='EXPERT',type='TH1F',title="R of two circles intersections (mm)", xbins = 200, xmin=0, xmax=600)
        montool.defineHistogram('init_r',        path='EXPERT',type='TH1F',title="R of initial vertex (mm)",       xbins = 200, xmin=0, xmax=600)
        montool.defineHistogram('init_trkd0',    path='EXPERT',type='TH1F',title="Trk d0 wrt initial vertex",      xbins = 200, xmin=0, xmax=200)
        montool.defineHistogram('init_trkz0',    path='EXPERT',type='TH1F',title="Trk z0 wrt initial vertex",      xbins = 100, xmin=0, xmax=100)
        montool.defineHistogram('vtxfit_chi2',   path='EXPERT',type='TH1F',title="Chi2 of fitted vertex",          xbins = 100, xmin=0, xmax=5)
        montool.defineHistogram('vtxfit_r',      path='EXPERT',type='TH1F',title="R of fitted vertex (mm)",        xbins = 200, xmin=0, xmax=600)
        montool.defineHistogram('vtx_chi2',      path='EXPERT',type='TH1F',title="Chi2 of vertex",                 xbins = 100, xmin=0, xmax=5)
        montool.defineHistogram('vtx_mass',      path='EXPERT',type='TH1F',title="Mass of vertex (GeV)",           xbins = 200, xmin=0, xmax=5)
        montool.defineHistogram('vtx_mass_high', path='EXPERT',type='TH1F',title="Mass of vertex (GeV, high)",     xbins = 200, xmin=0, xmax=100)
        montool.defineHistogram('vtx_pt',        path='EXPERT',type='TH1F',title="Pt of vertex (GeV)",             xbins = 100, xmin=0, xmax=100)
        montool.defineHistogram('vtx_charge',    path='EXPERT',type='TH1F',title="Charge of vertex",               xbins =   5, xmin=-2.5,xmax=2.5)
        montool.defineHistogram('vtx_r',         path='EXPERT',type='TH1F',title="R of vertex (mm)",               xbins = 200, xmin=0, xmax=600)

    def addTimingHistograms(montool):
        montool.defineHistogram('TIME_TrackSel',       path='EXPERT',type='TH1F',title="Track selection time (ns)",                xbins = 200, xmin=0.0, xmax=1000.0)
        montool.defineHistogram('TIME_TwoTrackVertex', path='EXPERT',type='TH1F',title="Two track vertexing time (ms)",            xbins = 200, xmin=0.0, xmax=10000.0)
        montool.defineHistogram('TIME_MapClustering',  path='EXPERT',type='TH1F',title="Time spent in vertex map clustering (us)", xbins = 200, xmin=0.0, xmax=10000.0)
        montool.defineHistogram('TIME_NTrackVertex',   path='EXPERT',type='TH1F',title="Time spent in Ntrack vertexing (ms)",      xbins = 200, xmin=0.0, xmax=500.0)
        montool.defineHistogram('TIME_NTrackVtxOffVSI',path='EXPERT',type='TH1F',title="Time spent in Ntrack vertexing (ms)",      xbins = 200, xmin=0.0, xmax=1000.0)
        montool.defineHistogram('TIME_CleanUp',        path='EXPERT',type='TH1F',title="Time spent in vertex cleaning (ns)",       xbins = 200, xmin=0.0, xmax=10000.0)
        montool.defineHistogram('TIME_WriteVertices',  path='EXPERT',type='TH1F',title="Time spent in writing vertex out (us)",    xbins = 200, xmin=0.0, xmax=100.0)
        montool.defineHistogram('TIME_Overall',        path='EXPERT',type='TH1F',title="Time spent over all vertexing (ms)",       xbins = 200, xmin=0.0, xmax=10000.0)

        montool.defineHistogram('TIME_TwoCircIntsect', path='EXPERT',type='TH1F',title="Two circles intersection time (ns) / Di-track vertexing", xbins = 200, xmin=0.0, xmax=1000.0)
        montool.defineHistogram('TIME_VrtFitFast',     path='EXPERT',type='TH1F',title="Two track vertexing fast time (ns) / Di-track vertexing", xbins = 200, xmin=0.0, xmax=10000.0)
        montool.defineHistogram('TIME_VrtFit',         path='EXPERT',type='TH1F',title="Two track vertexing time (us) / Di-track vertexing",      xbins = 200, xmin=0.0, xmax=2000.0)

        montool.defineHistogram('TIME_RetrvFromMap',   path='EXPERT',type='TH1F',title="Time spent in track list retrieving from the map (ns) / N-track vertexing", xbins = 200, xmin=0.0, xmax=10000.0)
        montool.defineHistogram('TIME_MergeParGraph',  path='EXPERT',type='TH1F',title="Time spent in vertex merging using partial graph (us) / N-track vertexing", xbins = 200, xmin=0.0, xmax=1000.0)
        montool.defineHistogram('TIME_MergeSimple',    path='EXPERT',type='TH1F',title="Time spent in simple vertex merging (us) / N-track vertexing",              xbins = 200, xmin=0.0, xmax=1000.0)


    montool = GenericMonitoringTool(flags, HistPath = f"TrigVrtSecInclusive_{name}")
    addTrackPairHistograms(montool)
    addTimingHistograms(montool)

    return montool


def TrigVrtSecInclusiveCfg(flags, name, FirstPassTracksName, SecondPassTracksName, PrimaryVertexInputName, VxCandidatesOutputName, TrkPairOutputName, recordTrkPair=True):

    acc = ComponentAccumulator()

    from TrkConfig.TrkVKalVrtFitterConfig import TrigVKalVrtFitterCfg
    VertexFitter = acc.popToolsAndMerge(
        TrigVKalVrtFitterCfg(flags, name = 'VKalVrtFitter_'+name))

    from InDetConfig.InDetConversionFinderToolsConfig import BPHY_VertexPointEstimatorCfg
    VertexPointEstimator = acc.popToolsAndMerge(
        BPHY_VertexPointEstimatorCfg(flags, name = 'VertexPointEstimator_'+name))

    alg = CompFactory.TrigVSI.TrigVrtSecInclusive(
        name = name,
        FirstPassTracksName    = FirstPassTracksName,
        SecondPassTracksName   = SecondPassTracksName,
        PrimaryVertexInputName = PrimaryVertexInputName,
        VxCandidatesOutputName = VxCandidatesOutputName,
        TrkPairOutputName      = TrkPairOutputName,
        VertexFitter           = VertexFitter,
        VertexPointEstimator   = VertexPointEstimator,
        recordTrkPair          = recordTrkPair,
        )

    # monitoring
    alg.MonTool = TrigVrtSecInclusive_VSIMonitoring(flags, 'jet')

    acc.addEventAlgo(alg)
    return acc
