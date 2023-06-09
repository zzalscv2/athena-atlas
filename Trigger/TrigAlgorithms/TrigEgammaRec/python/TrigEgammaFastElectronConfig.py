# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.SystemOfUnits import GeV, mm
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
from TrigEDMConfig.TriggerEDMRun3 import recordable


def fastElectronFexAlgCfg(flags, name="EgammaFastElectronFex_1", rois="EMRoIs"):

    acc = ComponentAccumulator()

    from TrackToCalo.TrackToCaloConfig import EMParticleCaloExtensionToolCfg
    extAcc = EMParticleCaloExtensionToolCfg(flags)
    extTool = acc.popToolsAndMerge(extAcc)

    efex = CompFactory.TrigEgammaFastElectronReAlgo( name,
                                         AcceptAll=False,
                                         TrackPt=1.0 * GeV,
                                         TrackPtHighEt=2.0 * GeV,
                                         ClusEt=20.0 * GeV,
                                         CaloTrackdEtaNoExtrap=0.5,
                                         CaloTrackdEtaNoExtrapHighEt=0.1,
                                         CaloTrackdETA=0.2,
                                         CaloTrackdPHI=0.3,
                                         CaloTrackdEoverPLow=0.0,
                                         CaloTrackdEoverPHigh=999.0,
                                         RCalBarrelFace=1470.0*mm,
                                         ZCalEndcapFace=3800.0*mm,
                                         useCaloInfoInExtrap=flags.Trigger.egamma.useCaloInfoInExtrap,
                                         ParticleCaloExtensionTool=extTool,
                                         ElectronsName=recordable("HLT_FastElectrons"),
                                         RoIs=rois,
                                         TrackParticlesName="HLT_IDTrack_Electron_FTF",
                                         TrigEMClusterName="HLT_FastCaloEMClusters",
                                         DummyElectronsName="HLT_FastDummyElectrons"
                                         )

    monTool = GenericMonitoringTool(flags, 'MonTool')
    monTool.defineHistogram('CaloTrackdEta', path='EXPERT', type='TH1F', title="FastElectron Hypo #Delta #eta between cluster and track;#Delta #eta;Nevents", xbins=80, xmin=-0.4, xmax=0.4)
    monTool.defineHistogram('CaloTrackdPhi', path='EXPERT', type='TH1F', title="FastElectron Hypo #Delta #phi between cluster and track;#Delta #phi;Nevents", xbins=80, xmin=-0.4, xmax=0.4)
    monTool.defineHistogram('CaloTrackEoverP', path='EXPERT', type='TH1F', title="FastElectron Hypo E/p;E/p;Nevents", xbins=120, xmin=0, xmax=12)
    monTool.defineHistogram('PtTrack', path='EXPERT', type='TH1F', title="FastElectron Hypo p_{T}^{track} [MeV];p_{T}^{track} [MeV];Nevents", xbins=50, xmin=0, xmax=100000)
    monTool.defineHistogram('PtCalo', path='EXPERT', type='TH1F', title="FastElectron Hypo p_{T}^{calo} [MeV];p_{T}^{calo} [MeV];Nevents", xbins=50, xmin=0, xmax=100000)
    monTool.defineHistogram('CaloEta',path='EXPERT',  type='TH1F', title="FastElectron Hypo #eta^{calo} ; #eta^{calo};Nevents", xbins=200, xmin=-2.5, xmax=2.5)
    monTool.defineHistogram('CaloPhi', path='EXPERT', type='TH1F', title="FastElectron Hypo #phi^{calo} ; #phi^{calo};Nevents", xbins=320, xmin=-3.2, xmax=3.2)
    monTool.defineHistogram('CaloTrackdEtaNoExtrapMon', path='EXPERT',type='TH1F', title="FastElectron Fex #Delta #eta between cluster and track;#Delta #eta;Nevents", xbins=80, xmin=-0.4, xmax=0.4)
    efex.MonTool = monTool

    acc.addEventAlgo(efex)
    return acc


