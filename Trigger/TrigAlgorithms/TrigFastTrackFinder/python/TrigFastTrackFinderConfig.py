# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool

def TrigFastTrackFinderMonitoring(flags):
    name =    "trigfasttrackfinder_" + flags.InDet.Tracking.ActiveConfig.name
    doResMon= flags.InDet.Tracking.ActiveConfig.doResMon
    return TrigFastTrackFinderMonitoringArg(flags, name, doResMon)

    
def TrigFastTrackFinderMonitoringArg(flags, name, doResMon):


    def addSPHistograms(montool, name):
        if name in ['FS', 'JetFS', 'FullScan', 'fullScan', 'fullScanUTT', 'jet']:
            montool.defineHistogram('roi_nSPsPIX', path='EXPERT',type='TH1F',title="Number of Pixel SPs", xbins = 500, xmin=-0.5, xmax=49999.5)
            montool.defineHistogram('roi_nSPsSCT', path='EXPERT',type='TH1F',title="Number of SCT SPs", xbins = 500, xmin=-0.5, xmax=99999.5)
            montool.defineHistogram('roi_phiWidth',path='EXPERT',type='TH1F',title="Phi width of the input RoI",xbins = 100, xmin=0, xmax=6.4)
            montool.defineHistogram('roi_etaWidth',path='EXPERT',type='TH1F',title="Eta width of the input RoI",xbins = 100, xmin=0, xmax=5)
        else:
            montool.defineHistogram('roi_nSPsPIX', path='EXPERT',type='TH1F',title="Number of Pixel SPs", xbins = 50, xmin=-0.5, xmax=4999.5)
            montool.defineHistogram('roi_nSPsSCT', path='EXPERT',type='TH1F',title="Number of SCT SPs", xbins = 50, xmin=-0.5, xmax=4999.5)
            montool.defineHistogram('roi_phiWidth',path='EXPERT',type='TH1F',title="Phi width of the input RoI",xbins = 100, xmin=0, xmax=1.0)
            montool.defineHistogram('roi_etaWidth',path='EXPERT',type='TH1F',title="Eta width of the input RoI",xbins = 100, xmin=0, xmax=1.0)

        montool.defineHistogram('roi_eta',     path='EXPERT',type='TH1F',title='Eta of the input RoI;;Entries', xbins=100, xmin=-5, xmax=5)
        montool.defineHistogram('roi_phi',     path='EXPERT',type='TH1F',title="Phi of the input RoI",xbins = 100, xmin=-3.2, xmax=3.2)
        montool.defineHistogram('roi_z',       path='EXPERT',type='TH1F',title="z of the input RoI",xbins = 200, xmin=-400, xmax=400)
        montool.defineHistogram('roi_zWidth',  path='EXPERT',type='TH1F',title="z width of the input RoI",xbins = 100, xmin=0, xmax=500)

    def addDataErrorHistograms(montool):
        montool.defineHistogram('roi_lastStageExecuted',path='EXPERT',type='TH1F',title="Last Step Successfully Executed", xbins = 8 , xmin=-0.5, xmax=7.5,
                             xlabels=["Start","GetRoI","GetSPs","ZFinder","Triplets","TrackMaker","TrackFitter","TrackConverter"])

    def addTimingHistograms(montool, name):
        if name in ['FS', 'JetFS', 'FullScan', 'fullScan', 'fullScanUTT', 'jet']:
            montool.defineHistogram('roi_nSPs, TIME_PattReco',   path='EXPERT',type='TH2F',title="PattReco time; nSPs",    xbins = 200, xmin=0.0, xmax=200000.0, ybins = 100, ymin=0.0, ymax=5000.0)
            montool.defineHistogram('roi_nTracks, TIME_PattReco',path='EXPERT',type='TH2F',title="PattReco time; nTracks", xbins = 50,  xmin=0.0, xmax=5000.0,   ybins = 100, ymin=0.0, ymax=5000.0)
            montool.defineHistogram('TIME_Total',                path='EXPERT',type='TH1F',title="Total time (ms)",             xbins = 200, xmin=0.0, xmax=5000.0)
            montool.defineHistogram('TIME_PattReco',             path='EXPERT',type='TH1F',title="Pure PattReco time (ms)",     xbins = 200, xmin=0.0, xmax=5000.0)
            montool.defineHistogram('TIME_SpacePointConversion', path='EXPERT',type='TH1F',title="SP Conversion time (ms)",     xbins = 100, xmin=0.0, xmax=100.0)
            montool.defineHistogram('TIME_ZFinder',              path='EXPERT',type='TH1F',title="ZFinder time (ms)",           xbins = 200, xmin=0.0, xmax=5000.0)
            montool.defineHistogram('TIME_Triplets',             path='EXPERT',type='TH1F',title="Triplets Making time (ms)",   xbins = 200, xmin=0.0, xmax=5000.0)
            montool.defineHistogram('TIME_CmbTrack',             path='EXPERT',type='TH1F',title="Combined Tracking time (ms)", xbins = 200, xmin=0.0, xmax=5000.0)
            montool.defineHistogram('TIME_TrackFitter',          path='EXPERT',type='TH1F',title="Track Fitter time (ms)",      xbins = 200, xmin=0.0, xmax=1000.0)
        elif name=='jetSuper':
            montool.defineHistogram('roi_nSPs, TIME_PattR1eco',   path='EXPERT',type='TH2F',title="PattReco time; nSPs",   xbins = 200, xmin=0.0, xmax=3000.0, ybins = 100, ymin=0.0, ymax=1000.0)
            montool.defineHistogram('roi_nTracks, TIME_PattReco',path='EXPERT',type='TH2F',title="PattReco time; nTracks", xbins = 50,  xmin=0.0, xmax=200.0,  ybins = 100, ymin=0.0, ymax=1000.0)
            montool.defineHistogram('TIME_Total',                path='EXPERT',type='TH1F',title="Total time (ms)",             xbins = 200, xmin=0.0, xmax=2000.0)
            montool.defineHistogram('TIME_PattReco',             path='EXPERT',type='TH1F',title="Pure PattReco time (ms)",     xbins = 200, xmin=0.0, xmax=1000.0)
            montool.defineHistogram('TIME_SpacePointConversion', path='EXPERT',type='TH1F',title="SP Conversion time (ms)",     xbins = 100, xmin=0.0, xmax=100.0)
            montool.defineHistogram('TIME_Triplets',             path='EXPERT',type='TH1F',title="Triplets Making time (ms)",   xbins = 200, xmin=0.0, xmax=1000.0)
            montool.defineHistogram('TIME_CmbTrack',             path='EXPERT',type='TH1F',title="Combined Tracking time (ms)", xbins = 200, xmin=0.0, xmax=1000.0)
            montool.defineHistogram('TIME_TrackFitter',          path='EXPERT',type='TH1F',title="Track Fitter time (ms)",      xbins = 200, xmin=0.0, xmax=200.0)
        elif name in ['beamSpot','beamSpotFS','bphysics','bmumux']:
            montool.defineHistogram('roi_nSPs, TIME_PattReco',   path='EXPERT',type='TH2F',title="PattReco time; nSPs",    xbins = 200, xmin=0.0, xmax=3000.0, ybins = 100, ymin=0.0, ymax=2000.0)
            montool.defineHistogram('roi_nTracks, TIME_PattReco',path='EXPERT',type='TH2F',title="PattReco time; nTracks", xbins = 50,  xmin=0.0, xmax=200.0,  ybins = 100, ymin=0.0, ymax=2000.0)
            montool.defineHistogram('TIME_Total',                path='EXPERT',type='TH1F',title="Total time (ms)",             xbins = 200, xmin=0.0, xmax=5000.0)
            montool.defineHistogram('TIME_PattReco',             path='EXPERT',type='TH1F',title="Pure PattReco time (ms)",     xbins = 200, xmin=0.0, xmax=2000.0)
            montool.defineHistogram('TIME_SpacePointConversion', path='EXPERT',type='TH1F',title="SP Conversion time (ms)",     xbins =  50, xmin=0.0, xmax=50.0)
            montool.defineHistogram('TIME_Triplets',             path='EXPERT',type='TH1F',title="Triplets Making time (ms)",   xbins = 200, xmin=0.0, xmax=2000.0)
            montool.defineHistogram('TIME_CmbTrack',             path='EXPERT',type='TH1F',title="Combined Tracking time (ms)", xbins = 200, xmin=0.0, xmax=2000.0)
            montool.defineHistogram('TIME_TrackFitter',          path='EXPERT',type='TH1F',title="Track Fitter time (ms)",      xbins = 200, xmin=0.0, xmax=200.0)
        elif name=='fullScanLRT':
            montool.defineHistogram('roi_nSPs, TIME_PattReco',   path='EXPERT',type='TH2F',title="PattReco time; nSPs",    xbins = 200, xmin=0.0, xmax=3000.0, ybins = 100, ymin=0.0, ymax=500.0)
            montool.defineHistogram('roi_nTracks, TIME_PattReco',path='EXPERT',type='TH2F',title="PattReco time; nTracks", xbins = 50,  xmin=0.0, xmax=200.0,  ybins = 100, ymin=0.0, ymax=500.0)
            montool.defineHistogram('TIME_Total',                path='EXPERT',type='TH1F',title="Total time (ms)",             xbins = 200, xmin=0.0, xmax=5000.0)
            montool.defineHistogram('TIME_PattReco',             path='EXPERT',type='TH1F',title="Pure PattReco time (ms)",     xbins = 200, xmin=0.0, xmax=2000.0)
            montool.defineHistogram('TIME_SpacePointConversion', path='EXPERT',type='TH1F',title="SP Conversion time (ms)",     xbins = 200, xmin=0.0, xmax=200.0)
            montool.defineHistogram('TIME_Triplets',             path='EXPERT',type='TH1F',title="Triplets Making time (ms)",   xbins = 200, xmin=0.0, xmax=400.0)
            montool.defineHistogram('TIME_CmbTrack',             path='EXPERT',type='TH1F',title="Combined Tracking time (ms)", xbins = 200, xmin=0.0, xmax=2000.0)
            montool.defineHistogram('TIME_TrackFitter',          path='EXPERT',type='TH1F',title="Track Fitter time (ms)",      xbins = 200, xmin=0.0, xmax=200.0)
        else:
            montool.defineHistogram('roi_nSPs, TIME_PattReco',   path='EXPERT',type='TH2F',title="PattReco time; nSPs",    xbins = 200, xmin=0.0, xmax=3000.0, ybins = 100, ymin=0.0, ymax=400.0)
            montool.defineHistogram('roi_nTracks, TIME_PattReco',path='EXPERT',type='TH2F',title="PattReco time; nTracks", xbins = 50,  xmin=0.0, xmax=200.0,  ybins = 100, ymin=0.0, ymax=400.0)
            montool.defineHistogram('TIME_Total',                path='EXPERT',type='TH1F',title="Total time (ms)",             xbins = 200, xmin=0.0, xmax=1000.0)
            montool.defineHistogram('TIME_PattReco',             path='EXPERT',type='TH1F',title="Pure PattReco time (ms)",     xbins = 200, xmin=0.0, xmax=400.0)
            montool.defineHistogram('TIME_SpacePointConversion', path='EXPERT',type='TH1F',title="SP Conversion time (ms)",     xbins =  20, xmin=0.0, xmax=20.0)
            montool.defineHistogram('TIME_Triplets',             path='EXPERT',type='TH1F',title="Triplets Making time (ms)",   xbins = 100, xmin=0.0, xmax=100.0)
            montool.defineHistogram('TIME_CmbTrack',             path='EXPERT',type='TH1F',title="Combined Tracking time (ms)", xbins = 200, xmin=0.0, xmax=400.0)
            montool.defineHistogram('TIME_TrackFitter',          path='EXPERT',type='TH1F',title="Track Fitter time (ms)",      xbins =  50, xmin=0.0, xmax=50.0)



    def addTrackHistograms(montool, name):
        if name in ['FS', 'JetFS', 'FullScan', 'fullScan', 'fullScanUTT', 'jet']:
            montool.defineHistogram('roi_nSeeds',     path='EXPERT',type='TH1F',title="Number of seeds",xbins = 1000, xmin=-0.5, xmax=99999.5)
            montool.defineHistogram('roi_nTracks',    path='EXPERT',type='TH1F',title="Number of Tracks",xbins = 100, xmin=-0.5, xmax=9999.5)
        elif name=='fullScanLRT':
            montool.defineHistogram('roi_nSeeds',     path='EXPERT',type='TH1F',title="Number of seeds",xbins = 1000, xmin=-0.5, xmax=99999.5)
            montool.defineHistogram('roi_nTracks',    path='EXPERT',type='TH1F',title="Number of Tracks",xbins = 100, xmin=-0.5, xmax=5000.5)
        else:
            montool.defineHistogram('roi_nSeeds',     path='EXPERT',type='TH1F',title="Number of seeds",xbins =  100, xmin=-0.5, xmax=4999.5)
            montool.defineHistogram('roi_nTracks',    path='EXPERT',type='TH1F',title="Number of Tracks",xbins =  50, xmin=-0.5, xmax=199.5)

        montool.defineHistogram('roi_nZvertices', path='EXPERT',type='TH1F',title="Number of z vertices",xbins = 60 ,  xmin=-0.5, xmax=49.5)
        montool.defineHistogram('roi_zVertices',  path='EXPERT',type='TH1F',title="ZFinder Vertices",xbins = 501, xmin=-250, xmax=250)
        montool.defineHistogram('roi_nTrk_zVtx',  path='EXPERT',type='TH1F',title="Ntrk ZFinder Vertices",xbins = 100, xmin=-0.5, xmax=49.5)
        montool.defineHistogram('trk_nSiHits',    path='EXPERT',type='TH1F',title="Total number of Silicon Hits per Track",xbins = 20, xmin=-0.5, xmax=19.5)
        montool.defineHistogram('trk_nPIXHits',   path='EXPERT',type='TH1F',title="Number of Pixel Hits per Track",xbins = 10, xmin=-0.5, xmax=9.5)
        montool.defineHistogram('trk_nSCTHits',   path='EXPERT',type='TH1F',title="Number of SCT Hits per Track",xbins = 10, xmin=-0.5, xmax=9.5)
        montool.defineHistogram('trk_chi2dof',    path='EXPERT',type='TH1F',title="ChiSqd / nDoF",xbins = 100, xmin=0.0, xmax=5)
        montool.defineHistogram('trk_pt',         path='EXPERT',type='TH1F',title="pt",xbins = 100, xmin=-2.5e5, xmax=2.5e5)
        montool.defineHistogram('trk_phi0',       path='EXPERT',type='TH1F',title="phi",xbins = 100, xmin=-3.2, xmax=3.2)
        montool.defineHistogram('trk_eta',        path='EXPERT',type='TH1F',title="eta",xbins = 100, xmin=-5, xmax=5)
        montool.defineHistogram('trk_dPhi0',      path='EXPERT',type='TH1F',title="dphi",xbins = 160, xmin=-0.8, xmax=0.8)
        montool.defineHistogram('trk_dEta',       path='EXPERT',type='TH1F',title="deta",xbins = 160, xmin=-0.8, xmax=0.8)
        if name=="Cosmic":
            montool.defineHistogram('trk_a0',     path='EXPERT',type='TH1F',title="a0",xbins = 100, xmin=-300, xmax=300)
            montool.defineHistogram('trk_a0beam', path='EXPERT',type='TH1F',title="a0beam",xbins = 100, xmin=-300, xmax=300)
            montool.defineHistogram('trk_z0',     path='EXPERT',type='TH1F',title="z0",xbins = 100, xmin=-800, xmax=800)
            montool.defineHistogram('trk_z0beam', path='EXPERT',type='TH1F',title="z0beam",xbins = 100, xmin=-800, xmax=800)
        elif name=='fullScanLRT':
            montool.defineHistogram('trk_a0',     path='EXPERT',type='TH1F',title="a0",xbins = 100, xmin=-300, xmax=300)
            montool.defineHistogram('trk_a0beam', path='EXPERT',type='TH1F',title="a0beam",xbins = 100, xmin=-300, xmax=300)
            montool.defineHistogram('trk_z0',     path='EXPERT',type='TH1F',title="z0",xbins = 100, xmin=-550, xmax=550)
            montool.defineHistogram('trk_z0beam', path='EXPERT',type='TH1F',title="z0beam",xbins = 100, xmin=-550, xmax=550)
        else:
            montool.defineHistogram('trk_a0',     path='EXPERT',type='TH1F',title="a0",xbins = 200, xmin=-10, xmax=10)
            montool.defineHistogram('trk_a0beam', path='EXPERT',type='TH1F',title="a0beam",xbins = 200, xmin=-10, xmax=10)
            montool.defineHistogram('trk_z0',     path='EXPERT',type='TH1F',title="z0",xbins = 200, xmin=-400, xmax=400)
            montool.defineHistogram('trk_z0beam', path='EXPERT',type='TH1F',title="z0beam",xbins = 200, xmin=-400, xmax=400)

    def addResidualHistograms(self):
        montool.defineHistogram('layer_IBL',                 path='EXPERT',type='TH1F',title="IBL layer",xbins = 10, xmin=0., xmax=10.)
        montool.defineHistogram('layer_PixB',                path='EXPERT',type='TH1F',title="Pixel Barrel layer",xbins = 10, xmin=0., xmax=10.)
        montool.defineHistogram('layer_PixE',                path='EXPERT',type='TH1F',title="Pixel Endcap layer",xbins = 10, xmin=0., xmax=10.)
        montool.defineHistogram('layer_SCTB',                path='EXPERT',type='TH1F',title="SCT Barrel layer",xbins = 10, xmin=0., xmax=10.)
        montool.defineHistogram('layer_SCTE',                path='EXPERT',type='TH1F',title="SCT Endcap layer",xbins = 10, xmin=0., xmax=10.)
        #
        montool.defineHistogram('hit_IBLPhiResidual',        path='EXPERT',type='TH1F',title="IBL hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_IBLEtaResidual',        path='EXPERT',type='TH1F',title="IBL hit-track eta residual",xbins = 100, xmin=-1.0, xmax=1.0)
        montool.defineHistogram('hit_IBLPhiPull',            path='EXPERT',type='TH1F',title="IBL hit-track phi pull",xbins = 100, xmin=-5., xmax=5.)
        montool.defineHistogram('hit_IBLEtaPull',            path='EXPERT',type='TH1F',title="IBL hit-track eta pull",xbins = 100, xmin=-5., xmax=5.)
        #
        montool.defineHistogram('hit_PIXBarrelPhiResidual',  path='EXPERT',type='TH1F',title="Pixel Barrel hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_PIXBarrelEtaResidual',  path='EXPERT',type='TH1F',title="Pixel Barrel hit-track eta residual",xbins = 100, xmin=-1.0, xmax=1.0)
        montool.defineHistogram('hit_PIXBarrelL1PhiResidual',path='EXPERT',type='TH1F',title="Pixel Barrel L1 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_PIXBarrelL1EtaResidual',path='EXPERT',type='TH1F',title="Pixel Barrel L1 hit-track eta residual",xbins = 100, xmin=-1.0, xmax=1.0)
        montool.defineHistogram('hit_PIXBarrelL2PhiResidual',path='EXPERT',type='TH1F',title="Pixel Barrel L2 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_PIXBarrelL2EtaResidual',path='EXPERT',type='TH1F',title="Pixel Barrel L2 hit-track eta residual",xbins = 100, xmin=-1.0, xmax=1.0)
        montool.defineHistogram('hit_PIXBarrelL3PhiResidual',path='EXPERT',type='TH1F',title="Pixel Barrel L3 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_PIXBarrelL3EtaResidual',path='EXPERT',type='TH1F',title="Pixel Barrel L3 hit-track eta residual",xbins = 100, xmin=-1.0, xmax=1.0)
        montool.defineHistogram('hit_PIXBarrelPhiPull',      path='EXPERT',type='TH1F',title="Pixel Barrel hit-track phi pull",xbins = 100, xmin=-5., xmax=5.)
        montool.defineHistogram('hit_PIXBarrelEtaPull',      path='EXPERT',type='TH1F',title="Pixel Barrel hit-track eta pull",xbins = 100, xmin=-5., xmax=5.)
        #
        montool.defineHistogram('hit_PIXEndcapPhiResidual',  path='EXPERT',type='TH1F',title="Pixel EC hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_PIXEndcapEtaResidual',  path='EXPERT',type='TH1F',title="Pixel EC hit-track eta residual",xbins = 100, xmin=-1.0, xmax=1.0)
        montool.defineHistogram('hit_PIXEndcapL1PhiResidual',path='EXPERT',type='TH1F',title="Pixel EC L1 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_PIXEndcapL1EtaResidual',path='EXPERT',type='TH1F',title="Pixel EC L1 hit-track eta residual",xbins = 100, xmin=-1.0, xmax=1.0)
        montool.defineHistogram('hit_PIXEndcapL2PhiResidual',path='EXPERT',type='TH1F',title="Pixel EC L2 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_PIXEndcapL2EtaResidual',path='EXPERT',type='TH1F',title="Pixel EC L2 hit-track eta residual",xbins = 100, xmin=-1.0, xmax=1.0)
        montool.defineHistogram('hit_PIXEndcapL3PhiResidual',path='EXPERT',type='TH1F',title="Pixel EC L3 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_PIXEndcapL3EtaResidual',path='EXPERT',type='TH1F',title="Pixel EC L3 hit-track eta residual",xbins = 100, xmin=-1.0, xmax=1.0)
        montool.defineHistogram('hit_PIXEndcapPhiPull',      path='EXPERT',type='TH1F',title="Pixel EC hit-track phi pull",xbins = 100, xmin=-5., xmax=5.)
        montool.defineHistogram('hit_PIXEndcapEtaPull',      path='EXPERT',type='TH1F',title="Pixel EC hit-track eta pull",xbins = 100, xmin=-5., xmax=5.)
        #
        montool.defineHistogram('hit_SCTBarrelResidual',     path='EXPERT',type='TH1F',title="SCT Barrel hit-track residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTBarrelL1PhiResidual',path='EXPERT',type='TH1F',title="SCT Barrel L1 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTBarrelL2PhiResidual',path='EXPERT',type='TH1F',title="SCT Barrel L2 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTBarrelL3PhiResidual',path='EXPERT',type='TH1F',title="SCT Barrel L3 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTBarrelL4PhiResidual',path='EXPERT',type='TH1F',title="SCT Barrel L4 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTBarrelPull',         path='EXPERT',type='TH1F',title="SCT Barrel hit-track pull",xbins = 100, xmin=-5., xmax=5.)
        #
        montool.defineHistogram('hit_SCTEndcapResidual',     path='EXPERT',type='TH1F',title="SCT EC hit-track residual",xbins = 100, xmin=-1.0, xmax=1.0)
        montool.defineHistogram('hit_SCTEndcapL1PhiResidual',path='EXPERT',type='TH1F',title="SCT Endcap L1 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTEndcapL2PhiResidual',path='EXPERT',type='TH1F',title="SCT Endcap L2 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTEndcapL3PhiResidual',path='EXPERT',type='TH1F',title="SCT Endcap L3 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTEndcapL4PhiResidual',path='EXPERT',type='TH1F',title="SCT Endcap L4 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTEndcapL5PhiResidual',path='EXPERT',type='TH1F',title="SCT Endcap L5 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTEndcapL6PhiResidual',path='EXPERT',type='TH1F',title="SCT Endcap L6 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTEndcapL7PhiResidual',path='EXPERT',type='TH1F',title="SCT Endcap L7 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTEndcapL8PhiResidual',path='EXPERT',type='TH1F',title="SCT Endcap L8 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTEndcapL9PhiResidual',path='EXPERT',type='TH1F',title="SCT Endcap L9 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTEndcapPull',         path='EXPERT',type='TH1F',title="SCT EC hit-track pull",xbins = 100, xmin=-5., xmax=5.)

    def addUTTHistograms(montool):
        montool.defineHistogram('trk_dedx',           path='EXPERT',type='TH1F',title="Track dEdx (pT > 3 GeV)", xbins = 140, xmin=-0.5, xmax=6.5)
        montool.defineHistogram('trk_dedx_nusedhits', path='EXPERT',type='TH1F',title="Nr of used hits for dEdx",xbins =  11, xmin=-0.5, xmax=10.5)
        #
        montool.defineHistogram('disTrk_nVtx',        path='EXPERT',type='TH1F',title="Nr of Vertex for disTrk",xbins =  11, xmin=-0.5, xmax=10.5)
        montool.defineHistogram('disTrk_xVtx',        path='EXPERT',type='TH1F',title="X position of primary vertex for disTrk", xbins = 120, xmin=-1.2, xmax=1.2)
        montool.defineHistogram('disTrk_yVtx',        path='EXPERT',type='TH1F',title="Y position of primary vertex for disTrk", xbins = 120, xmin=-1.2, xmax=1.2)
        montool.defineHistogram('disTrk_zVtx',        path='EXPERT',type='TH1F',title="Z position of primary vertex for disTrk", xbins = 150, xmin=-150, xmax=150)
        #
        montool.defineHistogram('disFailTrk_n',       path='EXPERT',type='TH1F',title="Nr of disFailTrk", xbins = 50, xmin=0, xmax=3000)
        montool.defineHistogram('disFailTrk_nclone',  path='EXPERT',type='TH1F',title="Nr of disFailTrk (after clone removal)", xbins = 50, xmin=0, xmax=3000)
        montool.defineHistogram('disFailTrk_ncand',   path='EXPERT',type='TH1F',title="Nr of disFailTrk (after pre-selection)", xbins = 50, xmin=0, xmax=3000)
        montool.defineHistogram('disCombTrk_n',       path='EXPERT',type='TH1F',title="Nr of disCombTrk", xbins = 20, xmin=0, xmax=100)
        montool.defineHistogram('disCombTrk_nclone',  path='EXPERT',type='TH1F',title="Nr of disCombTrk (after clone removal)", xbins = 20, xmin=0, xmax=100)
        montool.defineHistogram('disCombTrk_ncand',   path='EXPERT',type='TH1F',title="Nr of disCombTrk (after pre-selection)", xbins = 20, xmin=0, xmax=100)
        #
        montool.defineHistogram('TIME_HitDV',             path='EXPERT',type='TH1F',title="Hit-based DV search (ms)",          xbins = 100, xmin=0.0, xmax=400.0)
        montool.defineHistogram('TIME_dEdxTrk',           path='EXPERT',type='TH1F',title="Large dEdx search (ms)",            xbins =  20, xmin=0.0, xmax=20.0)
        montool.defineHistogram('TIME_disTrkZVertex',     path='EXPERT',type='TH1F',title="UTT z-vertexing time (ms)",         xbins =  10, xmin=0.0, xmax=10.0)
        montool.defineHistogram('TIME_disappearingTrack', path='EXPERT',type='TH1F',title="Disappearing track reco time (ms)", xbins = 100, xmin=0.0, xmax=300.0)


    montool = GenericMonitoringTool(flags, HistPath = f"TrigFastTrackFinder_{name}")
    addSPHistograms(montool, name)
    addDataErrorHistograms(montool)
    addTimingHistograms(montool, name)
    addTrackHistograms(montool, name)
    if doResMon:
        addResidualHistograms(montool)
    if name=='jet':
        addUTTHistograms(montool)

    return montool


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AthConfigFlags import AthConfigFlags

def TrigZFinderCfg(flags : AthConfigFlags, numberingTool) -> ComponentAccumulator:
  acc = ComponentAccumulator()
  zfargs = {}
  if flags.InDet.Tracking.ActiveConfig.name == "beamSpot" :
    zfargs = {
        'TripletMode'   : 1,
        'TripletDZ'     : 1,
        'PhiBinSize'    : 0.1,
        'UseOnlyPixels' : True,
        'MaxLayer'      : 3
    }
    
    acc.setPrivateTools(
        CompFactory.TrigZFinder( name="TrigZFinder",
                                 NumberOfPeaks = 3,
                                 LayerNumberTool=numberingTool,
                                 FullScanMode = True, #TODO: know this from the RoI anyway - should set for every event
                                 **zfargs
                                )
    )
    return acc                


def TrigFastTrackFinderCfg(flags: AthConfigFlags, name: str, slice_name: str, RoIs: str, inputTracksName:str = None) -> ComponentAccumulator:
  acc = ComponentAccumulator()

  from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
  config = getInDetTrigConfig( slice_name )

  remapped_type = config.name
  isCosmicConfig = (remapped_type=="cosmics")

  #Global keys/names for collections
  from TrigInDetConfig.InDetTrigCollectionKeys import TrigPixelKeys, TrigSCTKeys


  useNewLayerNumberScheme = True
  acc.addPublicTool(CompFactory.TrigL2LayerNumberTool(name="TrigL2LayerNumberTool_FTF",
                                                      UseNewLayerScheme = useNewLayerNumberScheme))
  numberingTool = acc.getPublicTool("TrigL2LayerNumberTool_FTF")
  
  # GPU offloading config begins - perhaps set from configure
  useGPU = False
  if useGPU :
    acc.addPublicTool(CompFactory.TrigInDetAccelerationTool(name = "TrigInDetAccelerationTool_FTF"))
  # GPU offloading config ends


  # Spacepoint conversion
  from RegionSelector.RegSelToolConfig import regSelTool_SCT_Cfg, regSelTool_Pixel_Cfg
  
  acc.addPublicTool(
      CompFactory.TrigSpacePointConversionTool(name = 'TrigSpacePointConversionTool_' + remapped_type,
                                               DoPhiFiltering        = config.DoPhiFiltering,
                                               UseNewLayerScheme     = useNewLayerNumberScheme,
                                               UseBeamTilt           = False,
                                               PixelSP_ContainerName = TrigPixelKeys.SpacePoints,
                                               SCT_SP_ContainerName  = TrigSCTKeys.SpacePoints,
                                               layerNumberTool       = numberingTool,
                                               UsePixelSpacePoints   = config.UsePixelSpacePoints,
                                               RegSelTool_Pixel = acc.popToolsAndMerge( regSelTool_Pixel_Cfg( flags) ),
                                               RegSelTool_SCT = acc.popToolsAndMerge( regSelTool_SCT_Cfg( flags) ),
                                               )
  )

  spTool = acc.getPublicTool('TrigSpacePointConversionTool_' + remapped_type)

  from TrkConfig.TrkExRungeKuttaPropagatorConfig import RungeKuttaPropagatorCfg
  InDetTrigPatternPropagator = acc.popToolsAndMerge(RungeKuttaPropagatorCfg(flags,name="InDetTrigRKPropagator"))
  acc.addPublicTool(InDetTrigPatternPropagator)
  
  acc.addPublicTool(CompFactory.InDet.SiDetElementsRoadMaker_xk(name='InDetTrigSiDetElementsRoadMaker'+remapped_type,
                                                                PropagatorTool = InDetTrigPatternPropagator,
                                                                usePixel     = flags.Detector.EnablePixel, 
                                                                useSCT       = flags.Detector.EnableSCT,
                                                                RoadWidth    = config.RoadWidth
                                                                )
                          )
  InDetTrigSiDetElementsRoadMaker_FTF = acc.getPublicTool('InDetTrigSiDetElementsRoadMaker'+remapped_type)
                          
  from InDetConfig.SiTrackMakerConfig import TrigSiTrackMaker_xkCfg
  TrackMaker_FTF = acc.popToolsAndMerge(
      TrigSiTrackMaker_xkCfg(flags, name = 'InDetTrigSiTrackMaker_FTF_'+slice_name,
                             RoadTool       = InDetTrigSiDetElementsRoadMaker_FTF,
                             )
  )
  acc.addPublicTool(TrackMaker_FTF)

  from TrkConfig.TrkRIO_OnTrackCreatorConfig import TrigRotCreatorCfg
  TrigRotCreator = acc.popToolsAndMerge(TrigRotCreatorCfg(flags))
  acc.addPublicTool(TrigRotCreator)
  
  acc.addPublicTool(
      CompFactory.TrigInDetTrackFitter(
          name = "TrigInDetTrackFitter_"+remapped_type,
          doBremmCorrection = '2023fix' in flags.InDet.Tracking.ActiveConfig.name,
          correctClusterPos = True,  #improved err(z0) estimates in Run 2
          ROTcreator = TrigRotCreator,
      )
  )
  theTrigInDetTrackFitter = acc.getPublicTool("TrigInDetTrackFitter_"+remapped_type)
  
  if (config.doZFinder):
    theTrigZFinder = acc.popToolsAndMerge(TrigZFinderCfg(flags,numberingTool))
      
  if not config.doZFinderOnly:
    
    from TrkConfig.TrkTrackSummaryToolConfig import InDetTrigTrackSummaryToolCfg, InDetTrigFastTrackSummaryToolCfg
    if config.holeSearch_FTF :
      trackSummaryTool = acc.popToolsAndMerge(InDetTrigTrackSummaryToolCfg(flags,name="InDetTrigTrackSummaryTool"))
    else:
      trackSummaryTool = acc.popToolsAndMerge(InDetTrigFastTrackSummaryToolCfg(flags,name="InDetTrigFastTrackSummaryTool"))

    acc.addPublicTool(trackSummaryTool)

    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    
    ftf = CompFactory.TrigFastTrackFinder(name = name,
                                          useNewLayerNumberScheme = useNewLayerNumberScheme,
                                          LayerNumberTool = numberingTool,
                                          useGPU = useGPU,
                                          SpacePointProviderTool=spTool,
                                          MinHits = 5, #Only process RoI with more than 5 spacepoints
                                          Triplet_MinPtFrac = 1,
                                          Triplet_nMaxPhiSlice = 53 if "cosmics" not in flags.InDet.Tracking.ActiveConfig.name else 2,
                                          LRT_Mode = flags.InDet.Tracking.ActiveConfig.isLRT,
                                          dodEdxTrk = flags.InDet.Tracking.ActiveConfig.dodEdxTrk,
                                          doHitDV = flags.InDet.Tracking.ActiveConfig.doHitDV,
                                          doDisappearingTrk = flags.InDet.Tracking.ActiveConfig.doDisappearingTrk,
                                          Triplet_MaxBufferLength = 3,
                                          doSeedRedundancyCheck = flags.InDet.Tracking.ActiveConfig.doSeedRedundancyCheck,
                                          Triplet_D0Max         = flags.InDet.Tracking.ActiveConfig.Triplet_D0Max,
                                          Triplet_D0_PPS_Max    = flags.InDet.Tracking.ActiveConfig.Triplet_D0_PPS_Max,
                                          TrackInitialD0Max     = flags.InDet.Tracking.ActiveConfig.TrackInitialD0Max,
                                          TrackZ0Max            = flags.InDet.Tracking.ActiveConfig.TrackZ0Max,
                                          TripletDoPPS    = flags.InDet.Tracking.ActiveConfig.TripletDoPPS,
                                          TripletDoPSS    = False,
                                          pTmin           = flags.InDet.Tracking.ActiveConfig.pTmin,
                                          DoubletDR_Max   = flags.InDet.Tracking.ActiveConfig.DoubletDR_Max,
                                          SeedRadBinWidth = flags.InDet.Tracking.ActiveConfig.SeedRadBinWidth,
                                          initialTrackMaker = TrackMaker_FTF,
                                          trigInDetTrackFitter = theTrigInDetTrackFitter,
                                          doZFinder = flags.InDet.Tracking.ActiveConfig.doZFinder,
                                          TrackSummaryTool = trackSummaryTool,
                                          doCloneRemoval = flags.InDet.Tracking.ActiveConfig.doCloneRemoval,
                                          TracksName     = flags.InDet.Tracking.ActiveConfig.trkTracks_FTF,
                                          doResMon = flags.InDet.Tracking.ActiveConfig.doResMon,
                                          MonTool = TrigFastTrackFinderMonitoring(flags),
                                          Extrapolator = acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)),
                                          RoIs = RoIs,
                                          )
    
  if config.LRT_D0Min is not None:
    ftf.LRT_D0Min = config.LRT_D0Min

  if config.LRT_HardMinPt is not None:
    ftf.LRT_HardMinPt = config.LRT_HardMinPt
  
  ftf.UseTrigSeedML = config.UseTrigSeedML

  if isCosmicConfig:
    ftf.Doublet_FilterRZ = False

  from TrigEDMConfig.TriggerEDMRun3 import recordable
  if config.dodEdxTrk:
    ftf.dEdxTrk = recordable("HLT_dEdxTrk")
    ftf.dEdxHit = recordable("HLT_dEdxHit")

  if config.doHitDV:
    ftf.doHitDV_Seeding = True
    ftf.RecJetRoI = "HLT_RecJETRoIs"
    ftf.HitDVSeed = "HLT_HitDVSeed" # not 'recordable' due to HLT truncation (ATR-23958)
    ftf.HitDVTrk  = "HLT_HitDVTrk"  # not 'recordable' due to HLT truncation (ATR-23958)
    ftf.HitDVSP   = "HLT_HitDVSP"   # not 'recordable' due to HLT truncation (ATR-23958)

  if config.doDisappearingTrk:
    ftf.DisTrkCand = recordable("HLT_DisTrkCand")
    from TrkConfig.TrkGlobalChi2FitterConfig import InDetTrigGlobalChi2FitterCfg
    InDetTrigTrackFitter = acc.popToolsAndMerge(InDetTrigGlobalChi2FitterCfg(flags))
    acc.addPublicTool(InDetTrigTrackFitter)
    ftf.DisTrackFitter = InDetTrigTrackFitter

  if config.doZFinder:
    ftf.doZFinderOnly = config.doZFinderOnly
    ftf.trigZFinder = theTrigZFinder
    ftf.zVertexResolution = 1
    ftf.doFastZVertexSeeding = True

  if inputTracksName:
    ftf.inputTracksName = inputTracksName
    
  acc.addEventAlgo(ftf)
  return acc




  from TrkConfig.TrkExRungeKuttaPropagatorConfig import RungeKuttaPropagatorCfg
  InDetTrigPatternPropagator = acc.popToolsAndMerge(RungeKuttaPropagatorCfg(flags,name="InDetTrigRKPropagator"))

  acc.addPublicTool(CompFactory.InDet.SiDetElementsRoadMaker_xk(name='InDetTrigSiDetElementsRoadMaker'+remapped_type,
                                                                PropagatorTool = InDetTrigPatternPropagator,
                                                                usePixel     = flags.Detector.EnablePixel, 
                                                                useSCT       = flags.Detector.EnableSCT,
                                                                RoadWidth    = config.RoadWidth
                                                                )
                          )
  InDetTrigSiDetElementsRoadMaker_FTF = acc.getPublicTool('InDetTrigSiDetElementsRoadMaker'+remapped_type)
                          
  from InDetConfig.SiTrackMakerConfig import TrigSiTrackMaker_xkCfg
  TrackMaker_FTF = acc.popToolsAndMerge(
      TrigSiTrackMaker_xkCfg(flags, name = 'InDetTrigSiTrackMaker_FTF_'+slice_name,
                             RoadTool       = InDetTrigSiDetElementsRoadMaker_FTF,
                             )
  )
  acc.addPublicTool(TrackMaker_FTF)
                          
  from TrkConfig.TrkRIO_OnTrackCreatorConfig import TrigRotCreatorCfg
  acc.addPublicTool(
      CompFactory.TrigInDetTrackFitter(
          name = "TrigInDetTrackFitter_"+remapped_type,
          doBremmCorrection = '2023fix' in flags.InDet.Tracking.ActiveConfig.name,
          correctClusterPos = True,  #improved err(z0) estimates in Run 2
          ROTcreator = acc.popToolsAndMerge(TrigRotCreatorCfg(flags)),
      )
  )
  theTrigInDetTrackFitter = acc.getPublicTool("TrigInDetTrackFitter_"+remapped_type)
  
  if (config.doZFinder):
    theTrigZFinder = acc.popToolsAndMerge(TrigZFinderCfg(flags,numberingTool))
      
  if not config.doZFinderOnly:
    
    from TrkConfig.TrkTrackSummaryToolConfig import InDetTrigTrackSummaryToolCfg, InDetTrigFastTrackSummaryToolCfg
    if config.holeSearch_FTF :
      trackSummaryTool = acc.popToolsAndMerge(InDetTrigTrackSummaryToolCfg(flags,name="InDetTrigTrackSummaryTool"))
    else:
      trackSummaryTool = acc.popToolsAndMerge(InDetTrigFastTrackSummaryToolCfg(flags,name="InDetTrigFastTrackSummaryTool"))

    acc.addPublicTool(trackSummaryTool)

    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    
    ftf = CompFactory.TrigFastTrackFinder(name = name,
                                          useNewLayerNumberScheme = useNewLayerNumberScheme,
                                          LayerNumberTool = numberingTool,
                                          useGPU = useGPU,
                                          SpacePointProviderTool=spTool,
                                          MinHits = 5, #Only process RoI with more than 5 spacepoints
                                          Triplet_MinPtFrac = 1,
                                          Triplet_nMaxPhiSlice = 53 if "cosmics" not in flags.InDet.Tracking.ActiveConfig.name else 2,
                                          LRT_Mode = flags.InDet.Tracking.ActiveConfig.isLRT,
                                          dodEdxTrk = flags.InDet.Tracking.ActiveConfig.dodEdxTrk,
                                          doHitDV = flags.InDet.Tracking.ActiveConfig.doHitDV,
                                          doDisappearingTrk = flags.InDet.Tracking.ActiveConfig.doDisappearingTrk,
                                          Triplet_MaxBufferLength = 3,
                                          doSeedRedundancyCheck = flags.InDet.Tracking.ActiveConfig.doSeedRedundancyCheck,
                                          Triplet_D0Max         = flags.InDet.Tracking.ActiveConfig.Triplet_D0Max,
                                          Triplet_D0_PPS_Max    = flags.InDet.Tracking.ActiveConfig.Triplet_D0_PPS_Max,
                                          TrackInitialD0Max     = flags.InDet.Tracking.ActiveConfig.TrackInitialD0Max,
                                          TrackZ0Max            = flags.InDet.Tracking.ActiveConfig.TrackZ0Max,
                                          TripletDoPPS    = flags.InDet.Tracking.ActiveConfig.TripletDoPPS,
                                          TripletDoPSS    = False,
                                          pTmin           = flags.InDet.Tracking.ActiveConfig.pTmin,
                                          DoubletDR_Max   = flags.InDet.Tracking.ActiveConfig.DoubletDR_Max,
                                          SeedRadBinWidth = flags.InDet.Tracking.ActiveConfig.SeedRadBinWidth,
                                          initialTrackMaker = TrackMaker_FTF,
                                          trigInDetTrackFitter = theTrigInDetTrackFitter,
                                          doZFinder = flags.InDet.Tracking.ActiveConfig.doZFinder,
                                          TrackSummaryTool = trackSummaryTool,
                                          doCloneRemoval = flags.InDet.Tracking.ActiveConfig.doCloneRemoval,
                                          TracksName     = flags.InDet.Tracking.ActiveConfig.trkTracks_FTF,
                                          doResMon = flags.InDet.Tracking.ActiveConfig.doResMon,
                                          MonTool = TrigFastTrackFinderMonitoring(flags),
                                          Extrapolator = acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)),
                                          RoIs = RoIs,
                                          )
    
  if config.LRT_D0Min is not None:
    ftf.LRT_D0Min = config.LRT_D0Min

  if config.LRT_HardMinPt is not None:
    ftf.LRT_HardMinPt = config.LRT_HardMinPt
  
  ftf.UseTrigSeedML = config.UseTrigSeedML

  if isCosmicConfig:
    ftf.Doublet_FilterRZ = False

  from TrigEDMConfig.TriggerEDMRun3 import recordable
  if config.dodEdxTrk:
    ftf.dEdxTrk = recordable("HLT_dEdxTrk")
    ftf.dEdxHit = recordable("HLT_dEdxHit")

  if config.doHitDV:
    ftf.doHitDV_Seeding = True
    ftf.RecJetRoI = "HLT_RecJETRoIs"
    ftf.HitDVSeed = "HLT_HitDVSeed" # not 'recordable' due to HLT truncation (ATR-23958)
    ftf.HitDVTrk  = "HLT_HitDVTrk"  # not 'recordable' due to HLT truncation (ATR-23958)
    ftf.HitDVSP   = "HLT_HitDVSP"   # not 'recordable' due to HLT truncation (ATR-23958)

  if config.doDisappearingTrk:
    ftf.DisTrkCand = recordable("HLT_DisTrkCand")
    from TrkConfig.TrkGlobalChi2FitterConfig import InDetTrigGlobalChi2FitterCfg
    InDetTrigTrackFitter = acc.popToolsAndMerge(InDetTrigGlobalChi2FitterCfg(flags))
    acc.addPublicTool(InDetTrigTrackFitter)
    ftf.DisTrackFitter = InDetTrigTrackFitter

  if config.doZFinder:
    ftf.doZFinderOnly = config.doZFinderOnly
    ftf.trigZFinder = theTrigZFinder
    ftf.zVertexResolution = 1
    ftf.doFastZVertexSeeding = True

  if inputTracksName:
    ftf.inputTracksName = inputTracksName
    
  acc.addEventAlgo(ftf)
  return acc

