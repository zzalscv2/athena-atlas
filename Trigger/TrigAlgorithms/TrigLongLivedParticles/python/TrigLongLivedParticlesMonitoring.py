# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
import math


def trigMuonClusterAlgorithmMonitoring(flags):

    montool = GenericMonitoringTool(flags, 'MonTool',
                                    HistPath = 'MuonRoIClusterTrigger/TrigAlgMoni')

    montool.defineHistogram('CluEta', path='EXPERT', type='TH1F', title="MuClu: Eta of the muon RoI cluster; eta; nevents", xbins=60, xmin=-3., xmax=3.)
    montool.defineHistogram('CluPhi', path='EXPERT', type='TH1F', title="MuClu: Phi of the muon RoI cluster; phi; nevents", xbins=32, xmin=-math.pi*1.05, xmax=math.pi*1.05)
    montool.defineHistogram('NumRoi', opt='kAddBinsDynamically', path='EXPERT', type='TH1F', title="MuClu: RoIs in the muon RoI cluster; number of RoI; nevents", xbins=14, xmin=0., xmax=14.)
    montool.defineHistogram('RoiEta', path='EXPERT', type='TH1F', title="MuClu: Eta of all Input Muon RoIs (From Vectors); eta; nevents", xbins=60, xmin=-3., xmax=3.)
    montool.defineHistogram('RoiPhi', path='EXPERT', type='TH1F', title="MuClu: Phi of all Input Muon RoIs (From Vectors); phi; nevents", xbins=32, xmin=-math.pi*1.05, xmax=math.pi*1.05)

    montool.defineHistogram('CluPhi,CluEta', path='EXPERT', type='TH2F', title='MuClu: 2D Phi vs Eta of RoI clusters; phi; eta', xbins=128, xmin=-math.pi*1.05, xmax=math.pi*1.05,  ybins=120, ymin=-3., ymax=3.)
    montool.defineHistogram('nL1RoIs,nRoIinClusters', path='EXPERT', type='TH2F', title='MuClu: Number of L1 RoIs vs Number of RoI in Clusters;nL1RoIs;nCluRoIs', xbins=20, xmin=0, xmax=20, ybins=20, ymin=0, ymax=20)
    montool.defineHistogram('nL1RoIs,nClusters', path='EXPERT', type='TH2F', title='MuClu: Number of L1 RoIs vs Number of RoI-Clusters;nL1RoIs;nClusters', xbins=20, xmin=0, xmax=20, ybins=20, ymin=0, ymax=20)
    montool.defineHistogram('nRoIinClusters,nClusters', path='EXPERT', type='TH2F', title='MuClu: Number of RoI in Clusters vs Number of RoI-Clusters;nCluRoIs;nClusters', xbins=20, xmin=0, xmax=20, ybins=20, ymin=0, ymax=20)

    montool.defineHistogram('dPhiCluSeed', path='EXPERT', type='TH1F', title='MuClu: dPhi between Cluster & Seed RoI; dPhi; nClusters', xbins=64, xmin=-0.8, xmax=0.8)
    montool.defineHistogram('dEtaCluSeed', path='EXPERT', type='TH1F', title='MuClu: dEta between Cluster & Seed RoI; dEta; nClusters', xbins=60, xmin=-1., xmax=1.)
    montool.defineHistogram('dRCluSeed', opt='kAddBinsDynamically', path='EXPERT', type='TH1F', title='MuClu: dR between Cluster & Seed RoI; dR; nClusters', xbins=60, xmin=0, xmax=0.8)

    montool.defineHistogram('TIME_TrigAlg', opt='kAddBinsDynamically', path='EXPERT', type='TH1F', title='MuClu: Timing Variable for entire TrigAlg Execution; time\\ (\\mu s); nTrigAlgs', xbins=150, xmin=-50., xmax=100.)
    montool.defineHistogram('TIME_TrigAlg_Clustering', opt='kAddBinsDynamically', path='EXPERT', type='TH1F', title='MuClu: Timing Variable for Clustering Loop in TrigAlg; time\\ (\\mu s); nTrigAlgs', xbins=150, xmin=-50., xmax=100.)

    return montool
