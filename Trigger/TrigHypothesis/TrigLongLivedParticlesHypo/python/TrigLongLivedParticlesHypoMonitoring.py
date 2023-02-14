# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#MUON CLUSTER HYPO ALG
################ Create Instance for Online Monitoring (basic histograms)
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
import math

def trigMuonClusterHypoAlgMonitoring(flags):

    montool = GenericMonitoringTool(flags, "MonTool",
                                    HistPath = 'MuonRoIClusterTrigger/HypoMoni')

    montool.defineHistogram('AcceptAll', path='EXPERT', type='TH1F', title="MuCluHypo: Whether acceptAll is set; bool; nevents", xbins=2, xmin=0., xmax=2.)
    montool.defineHistogram('nRoIEndCap', path='EXPERT', type='TH1F', title="MuCluHypo: Minimum Number of RoIs Required in Endcap Cluster; number of RoIs; nclusters", xbins=14, xmin=0., xmax=14.)
    montool.defineHistogram('nRoIBarrel', path='EXPERT', type='TH1F', title="MuCluHypo: Minimum Number of RoIs Required in Barrel Cluster; number of RoIs; nclusters", xbins=14, xmin=0., xmax=14.)
    montool.defineHistogram('maxEta', path='EXPERT', type='TH1F', title="MuCluHypo: Value of maxEta parameter; maxEta; nclusters", xbins=60, xmin=-3., xmax=3.)
    montool.defineHistogram('midEta', path='EXPERT', type='TH1F', title="MuCluHypo: Value of midEta parameter; midEta; nclusters", xbins=60, xmin=-3., xmax=3.)
    montool.defineHistogram('isPass', path='EXPERT', type='TH1F', title="MuCluHypo: Number of MuonRoIClusters accepted by the trigger; bool; nclusters", xbins=2, xmin=0., xmax=2.)

    montool.defineHistogram('TIME_HypoAlg', path='EXPERT', type='TH1F', title='MuClusHypo: Timing Variable for entire HypoAlg Execution; time (us); nAlgs', xbins=100, xmin=-20., xmax=4000.)
    montool.defineHistogram('TIME_HypoAlg_DecisionLoop', path='EXPERT', type='TH1F', title='MuClusHypo: Timing Variable for HypoAlg Loop over Decisions; time (us); nAlgs', xbins=100, xmin=-20., xmax=2000.)

    return montool


#MUON CLUSTER HYPO TOOL
################ Create Instance for Online Monitoring (basic histograms)


def trigMuonClusterHypoToolMonitoring(flags):

    montool = GenericMonitoringTool(flags, "MonTool",
                                    HistPath = 'MuonRoIClusterTrigger/HypoToolMoni')

    montool.defineHistogram('etaClust', path='EXPERT', type='TH1F', title="MuCluHypoTool: Eta of the muon RoI cluster; eta; nclusters", xbins=60, xmin=-3., xmax=3.)
    montool.defineHistogram('phiClust', path='EXPERT', type='TH1F', title="MuCluHypoTool: Phi of the muon RoI cluster; phi; nclusters", xbins=32, xmin=-math.pi, xmax=math.pi)
    montool.defineHistogram('numberRoI', path='EXPERT', type='TH1F', title="MuCluHypoTool: RoIs in Passing muon RoI clusters; number of RoI; nclusters", xbins=25, xmin=0., xmax=25.)

    montool.defineHistogram('nRoIBarrel,nRoIBarrelPass', path='EXPERT', type='TH2F', title="MuCluHypoTool: nRoI in All Barrel Clusters vs nRoI in all Passing Clusters; number of Barrel RoI; number of Passing Barrel RoI", xbins=25, xmin=0., xmax=25., ybins=25, ymin=0., ymax=25.)
    montool.defineHistogram('nRoIEndcap,nRoIEndcapPass', path='EXPERT', type='TH2F', title="MuCluHypoTool: nRoI in All Endcap Clusters vs nRoI in all Passing Clusters; number of Endcap RoI; number of Passing Endcap RoI", xbins=25, xmin=0., xmax=25., ybins=25, ymin=0., ymax=25.)
    montool.defineHistogram('result', path='EXPERT', type='TH1F', title="MuCluHypoTool: Number of MuonRoIClusters accepted by the trigger; bool; nclusters", xbins=2, xmin=0., xmax=2.)
    montool.defineHistogram('chainActive', path='EXPERT', type='TH1F', title='MuCluHypoTool: Bool if number of previous Decision IDs matching this cahin > 0; bool; nCallsToHypoTool', xbins=2, xmin=0., xmax=2.)
    montool.defineHistogram('phiClust,etaClust', path='EXPERT', type='TH2F', title="MuCluHypoTool: 2D Phi vs Eta of All muon RoI clusters; phi; eta", xbins=128, xmin=-math.pi, xmax=math.pi,  ybins=120, ymin=-3., ymax=3.)
    montool.defineHistogram('phiClustPass,etaClustPass', path='EXPERT', type='TH2F', title="MuCluHypoTool: 2D Phi vs Eta of Passing muon RoI clusters; phi; eta", xbins=128, xmin=-math.pi, xmax=math.pi,  ybins=120, ymin=-3., ymax=3.)

    montool.defineHistogram('numberRoI,etaClustPass', path='EXPERT', type='TH2F', title="MuCluHypoTool: 2D Number of RoIs in Passing muon RoI clusters vs Eta of Passing muon RoI clusters; nRoIs; eta", xbins=10, xmin=0., xmax=10., ybins=60, ymin=-3., ymax=3.)

    montool.defineHistogram('TIME_HypoTool', path='EXPERT', type='TH1F', title='MuClusHypoTool: Timing Variable for entire HypoTool Execution; time (us); nTools', xbins=100, xmin=-20., xmax=1500.)
    montool.defineHistogram('TIME_HypoTool_GetCluster', path='EXPERT', type='TH1F', title='MuClusHypoTool: Timing Variable for HypoTool to Locate MuonRoICluster Object; time (us); nTools', xbins=100, xmin=-20., xmax=1500.)
    montool.defineHistogram('TIME_HypoTool_Selection', path='EXPERT', type='TH1F', title='MuClusHypoTool: Timing Variable for HypoTool to Decide Pass/Fail; time (us); nTools', xbins=100, xmin=-20., xmax=1500.)

    return montool
