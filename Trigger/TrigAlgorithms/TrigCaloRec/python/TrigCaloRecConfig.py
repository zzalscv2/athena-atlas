# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.SystemOfUnits import MeV, deg
from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool

from TrigCaloRec.TrigCaloRecConf import TrigCaloClusterMaker, TrigCaloTowerMaker
from TrigEDMConfig.TriggerEDMRun3 import recordable

mlog = logging.getLogger ('TrigCaloRecConfig')


def trigCaloClusterMakerMonTool(flags, doMonCells = False):
    """Monitoring tool for TrigCaloClusterMaker"""

    monTool = GenericMonitoringTool(flags, 'MonTool')

    maxNumberOfClusters = 1200 if doMonCells else 50
    maxProcTime = 150000 if doMonCells else 4500

    monTool.defineHistogram('container_size', path='EXPERT', type='TH1F',  title="Container Size; Number of Clusters; Number of Events", xbins=50, xmin=0.0, xmax=maxNumberOfClusters)
    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title="Total Execution Time; Execution time [ us ] ; Number of runs", xbins=100, xmin=0.0, xmax=maxProcTime)
    monTool.defineHistogram('TIME_ClustMaker', path='EXPERT', type='TH1F', title="Cluster Maker Time; Execution time [ us ] ; Number of runs", xbins=100, xmin=0.0, xmax=maxProcTime)
    monTool.defineHistogram('TIME_ClustCorr', path='EXPERT', type='TH1F', title="Cluster Correction Time; Execution time [ us ] ; Number of runs", xbins=100, xmin=0.0, xmax=100)
    monTool.defineHistogram('Et', path='EXPERT', type='TH1F',  title="Cluster E_T; E_T [ MeV ] ; Number of Clusters", xbins=135, xmin=-200.0, xmax=2500.0)
    monTool.defineHistogram('Eta', path='EXPERT', type='TH1F', title="Cluster #eta; #eta ; Number of Clusters", xbins=100, xmin=-2.5, xmax=2.5)
    monTool.defineHistogram('Phi', path='EXPERT', type='TH1F', title="Cluster #phi; #phi ; Number of Clusters", xbins=64, xmin=-3.2, xmax=3.2)
    monTool.defineHistogram('Eta,Phi', path='EXPERT', type='TH2F', title="Number of Clusters; #eta ; #phi ; Number of Clusters", xbins=100, xmin=-2.5, xmax=2.5, ybins=128, ymin=-3.2, ymax=3.2)
    monTool.defineHistogram('clusterSize', path='EXPERT', type='TH1F', title="Cluster Type; Type ; Number of Clusters", xbins=13, xmin=0.5, xmax=13.5)
    monTool.defineHistogram('signalState', path='EXPERT', type='TH1F', title="Signal State; Signal State ; Number of Clusters", xbins=4, xmin=-1.5, xmax=2.5)
    monTool.defineHistogram('size', path='EXPERT', type='TH1F', title="Cluster Size; Size [Cells] ; Number of Clusters", xbins=125, xmin=0.0, xmax=250.0)
    monTool.defineHistogram('N_BAD_CELLS', path='EXPERT', type='TH1F', title="N_BAD_CELLS; N_BAD_CELLS ; Number of Clusters", xbins=250, xmin=0.5, xmax=250.5)
    monTool.defineHistogram('ENG_FRAC_MAX', path='EXPERT', type='TH1F', title="ENG_FRAC_MAX; ENG_FRAC_MAX ; Number of Clusters", xbins=50, xmin=0.0, xmax=1.1)
    monTool.defineHistogram('mu', path='EXPERT', type='TH1F',  title="mu; mu; Number of Events", xbins=50, xmin=0.0, xmax=100)
    monTool.defineHistogram('mu,container_size', path='EXPERT', type='TH2F',  title="Container Size versus #mu; #mu; cluster container size", xbins=50, xmin=20.0, xmax=70, ybins=50, ymin=0.0, ymax=maxNumberOfClusters)

    if doMonCells:
        monTool.defineHistogram('count_1thrsigma', path='EXPERT', type='TH1F',  title="count_1thrsigma; count_1thresigma; Number of Events", xbins=50, xmin=0.0, xmax=10e3)
        monTool.defineHistogram('count_2thrsigma', path='EXPERT', type='TH1F',  title="count_2thrsigma; count_2thresigma; Number of Events", xbins=50, xmin=0.0, xmax=5e3)
        monTool.defineHistogram('mu,count_1thrsigma', path='EXPERT', type='TH2F',  title="nCells above 1st thr versus #mu; #mu; nCells", xbins=50, xmin=20.0, xmax=70, ybins=50, ymin=0.0, ymax=10e3)
        monTool.defineHistogram('mu,count_2thrsigma', path='EXPERT', type='TH2F',  title="nCells above 2nd thr versus #mu; #mu; nCells", xbins=50, xmin=20.0, xmax=70, ybins=50, ymin=0.0, ymax=5e3)

    return monTool


class TrigCaloTowerMakerBase (TrigCaloTowerMaker):
    __slots__ = []
    def __init__(self, name):
        super( TrigCaloTowerMakerBase, self ).__init__(name)

class TrigCaloClusterMakerBase (TrigCaloClusterMaker):
    __slots__ = []
    def __init__(self, name):
        super( TrigCaloClusterMakerBase, self ).__init__(name)

        from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
        self.MonCells = "FS" in name
        self.MonTool = trigCaloClusterMakerMonTool(flags, self.MonCells)

class TrigCaloClusterMaker_topo (TrigCaloClusterMakerBase):
    __slots__ = []
    def __init__ (self, name='TrigCaloClusterMaker_topo', cells="cells",doMoments=True, doLC=True ):
        super(TrigCaloClusterMaker_topo, self).__init__(name)
        
        self.Cells=cells

        from CaloClusterCorrection.CaloClusterCorrectionConf import CaloLCWeightTool, CaloLCClassificationTool, CaloLCOutOfClusterTool, CaloLCDeadMaterialTool
        from CaloClusterCorrection.CaloClusterCorrectionConf import CaloClusterLocalCalib
        from CaloRec.CaloRecConf import CaloTopoClusterMaker, CaloTopoClusterSplitter, CaloClusterMomentsMaker
        from CaloTools.CaloNoiseCondAlg import CaloNoiseCondAlg
        from CaloRec.CaloTopoClusterFlags import jobproperties
        from AthenaCommon.SystemOfUnits import deg
        from AthenaCommon.GlobalFlags import globalflags
        from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
        
        # tools used by tools

        if doLC:
          #For LCWeightsTool needs electronic noise
          CaloNoiseCondAlg(noisetype="electronicNoise") 
          TrigLCClassify   = CaloLCClassificationTool("TrigLCClassify")
          TrigLCClassify.ClassificationKey   = "EMFracClassify"
          TrigLCClassify.UseSpread = False
          TrigLCClassify.MaxProbability = 0.5
          TrigLCClassify.StoreClassificationProbabilityInAOD = True

          TrigLCWeight = CaloLCWeightTool("TrigLCWeight")
          TrigLCWeight.CorrectionKey       = "H1ClusterCellWeights"
          TrigLCWeight.SignalOverNoiseCut  = 2.0
          TrigLCWeight.UseHadProbability   = True

          TrigLCOut     = CaloLCOutOfClusterTool("TrigLCOut")
          TrigLCOut.CorrectionKey       = "OOCCorrection"
          TrigLCOut.UseEmProbability    = False
          TrigLCOut.UseHadProbability   = True

          TrigLCOutPi0  = CaloLCOutOfClusterTool("TrigLCOutPi0")
          TrigLCOutPi0.CorrectionKey    = "OOCPi0Correction"
          TrigLCOutPi0.UseEmProbability  = True
          TrigLCOutPi0.UseHadProbability = False

          TrigLCDeadMaterial   = CaloLCDeadMaterialTool("TrigLCDeadMaterial")
          TrigLCDeadMaterial.HadDMCoeffKey       = "HadDMCoeff2"
          #TrigLCDeadMaterial.SignalOverNoiseCut  = 1.0
          TrigLCDeadMaterial.ClusterRecoStatus   = 0
          TrigLCDeadMaterial.WeightModeDM        = 2
          TrigLCDeadMaterial.UseHadProbability   = True

          # correction tools using tools
          TrigLocalCalib = CaloClusterLocalCalib ("TrigLocalCalib")
          TrigLocalCalib.ClusterClassificationTool     = [TrigLCClassify]
          #TrigLocalCalib.ClusterRecoStatus             = [2]
          TrigLocalCalib.ClusterRecoStatus             = [1,2]
          TrigLocalCalib.LocalCalibTools               = [TrigLCWeight]

          TrigLocalCalib += TrigLCClassify
          TrigLocalCalib += TrigLCWeight

          TrigOOCCalib   = CaloClusterLocalCalib ("TrigOOCCalib")
          #TrigOOCCalib.ClusterRecoStatus   = [2]
          TrigOOCCalib.ClusterRecoStatus   = [1,2]
          TrigOOCCalib.LocalCalibTools     = [TrigLCOut]

          TrigOOCCalib += TrigLCOut

          TrigOOCPi0Calib   = CaloClusterLocalCalib ("TrigOOCPi0Calib")
          #OOCPi0Calib.ClusterRecoStatus   = [1]
          TrigOOCPi0Calib.ClusterRecoStatus   = [1,2]
          TrigOOCPi0Calib.LocalCalibTools     = [TrigLCOutPi0]

          TrigOOCPi0Calib += TrigLCOutPi0


          TrigDMCalib    = CaloClusterLocalCalib ("TrigDMCalib")
          TrigDMCalib.ClusterRecoStatus   = [1,2]
          TrigDMCalib.LocalCalibTools     = [TrigLCDeadMaterial]

          TrigDMCalib += TrigLCDeadMaterial

       
        if doMoments:
 
          # correction tools not using tools
          TrigTopoMoments = CaloClusterMomentsMaker ("TrigTopoMoments")
          TrigTopoMoments.MaxAxisAngle = 20*deg
          TrigTopoMoments.TwoGaussianNoise = jobproperties.CaloTopoClusterFlags.doTwoGaussianNoise()
          TrigTopoMoments.MinBadLArQuality = 4000
          TrigTopoMoments.MomentsNames = ["FIRST_PHI" 
                                          ,"FIRST_ETA"
                                          ,"SECOND_R" 
                                          ,"SECOND_LAMBDA"
                                          ,"DELTA_PHI"
                                          ,"DELTA_THETA"
                                          ,"DELTA_ALPHA" 
                                          ,"CENTER_X"
                                          ,"CENTER_Y"
                                          ,"CENTER_Z"
                                          ,"CENTER_MAG"
                                          ,"CENTER_LAMBDA"
                                          ,"LATERAL"
                                          ,"LONGITUDINAL"
                                          ,"FIRST_ENG_DENS" 
                                          ,"ENG_FRAC_EM" 
                                          ,"ENG_FRAC_MAX" 
                                          ,"ENG_FRAC_CORE" 
                                          ,"FIRST_ENG_DENS" 
                                          ,"SECOND_ENG_DENS" 
                                          ,"ISOLATION"
                                          ,"ENG_BAD_CELLS"
                                          ,"N_BAD_CELLS"
                                          ,"N_BAD_CELLS_CORR"
                                          ,"BAD_CELLS_CORR_E"
                                          ,"BADLARQ_FRAC"
                                          ,"ENG_POS"
                                          ,"SIGNIFICANCE"
                                          ,"CELL_SIGNIFICANCE"
                                          ,"CELL_SIG_SAMPLING"
                                          ,"AVG_LAR_Q"
                                          ,"AVG_TILE_Q"
                                          ]          
        #TrigLockVariables = CaloClusterLockVars("TrigLockVariables")
        #TrigLockVariables.FixBasicEnergy = True
        #TrigLockVariables.LockedSamplingVariables = []
        #TrigLockVariables.LockedSamplingVariables += [
        #    "Energy", "Max_Energy"]
        #TrigLockVariables.LockedSamplingVariables += [
        #    "Eta", "Phi", "Delta_Eta",
        #    "Delta_Phi", "Max_Eta", "Max_Phi"
        #    ]
        
        # maker tools
        CaloNoiseCondAlg()
        TrigTopoMaker = CaloTopoClusterMaker("TopoMaker")

        TrigTopoMaker.CellsName = cells
        TrigTopoMaker.CalorimeterNames=["LAREM",
                                        "LARHEC",
                                        "LARFCAL",
                                        "TILE"]
        # cells from the following samplings will be able to form
        # seeds. By default no sampling is excluded
        TrigTopoMaker.SeedSamplingNames = ["PreSamplerB", "EMB1", "EMB2", "EMB3",
                                           "PreSamplerE", "EME1", "EME2", "EME3",
                                           "HEC0", "HEC1","HEC2", "HEC3",
                                           "TileBar0", "TileBar1", "TileBar2",
                                           "TileExt0", "TileExt1", "TileExt2",
                                           "TileGap1", "TileGap2", "TileGap3",
                                           "FCAL0", "FCAL1", "FCAL2"]

        TrigTopoMaker.NeighborOption = "super3D"
        TrigTopoMaker.RestrictHECIWandFCalNeighbors  = False
        TrigTopoMaker.CellThresholdOnEorAbsEinSigma     =    0.0
        TrigTopoMaker.NeighborThresholdOnEorAbsEinSigma =    2.0  #instead of 2
        TrigTopoMaker.SeedThresholdOnEorAbsEinSigma     =    4.0  #instead of 4
        # note Et or AbsEt
        #TrigTopoMaker.NeighborCutsInAbsE              = False
        #TrigTopoMaker.CellCutsInAbsE                 = False
        #
        # the following cut must be set to TRUE in order to make double
        # sided cuts on the seed and the cluster level ( neighbor and cell
        # cuts are always double sided)
        #
        TrigTopoMaker.SeedCutsInAbsE                 = True
        TrigTopoMaker.ClusterEtorAbsEtCut            = 0.0*MeV
        # the following Et thresholds are ignored in case UsePileUpNoise
        # is TRUE
        #
        #
        #CellThresholdOnEtorAbsEt = 0.0*MeV
        #NeighborThresholdOnEtorAbsEt = 100.0*MeV
        #SeedThresholdOnEtorAbsEt = 200.0*MeV

        # use 2-gaussian or single gaussian noise for TileCal
        TrigTopoMaker.TwoGaussianNoise = jobproperties.CaloTopoClusterFlags.doTwoGaussianNoise()

        #timing
        TrigTopoMaker.SeedCutsInT = flags.Trigger.Calo.TopoCluster.doTimeCut
        TrigTopoMaker.CutOOTseed = flags.Trigger.Calo.TopoCluster.extendTimeCut and flags.Trigger.Calo.TopoCluster.doTimeCut
        TrigTopoMaker.UseTimeCutUpperLimit = flags.Trigger.Calo.TopoCluster.useUpperLimitForTimeCut
        TrigTopoMaker.TimeCutUpperLimit = flags.Trigger.Calo.TopoCluster.timeCutUpperLimit

        TrigTopoSplitter = CaloTopoClusterSplitter("TopoSplitter")        
        # cells from the following samplings will be able to form local
        # maxima. The excluded samplings are PreSamplerB, EMB1,
        # PreSamplerE, EME1, all Tile samplings, all HEC samplings and the
        # two rear FCal samplings.
        #
        TrigTopoSplitter.SamplingNames = ["EMB2", "EMB3",
                                          "EME2", "EME3",
                                          "FCAL0"]
        # cells from the following samplings will also be able to form
        # local maxima but only if they are not overlapping in eta and phi
        # with local maxima in previous samplings from the primary list.
        #
        TrigTopoSplitter.SecondarySamplingNames = ["EMB1","EME1",
                                                   "TileBar0","TileBar1","TileBar2",
                                                   "TileExt0","TileExt1","TileExt2",
                                                   "HEC0","HEC1","HEC2","HEC3",
                                                   "FCAL1","FCAL2"]
        TrigTopoSplitter.ShareBorderCells = True
        TrigTopoSplitter.RestrictHECIWandFCalNeighbors  = False
        #
        # the following options are not set, since these are the default
        # values
        #
        # NeighborOption                = "super3D",
        # NumberOfCellsCut              = 4,
        # EnergyCut                     = 500*MeV,


        # cluster maker

        if not doMoments:
          self.ClusterMakerTools = [ TrigTopoMaker, TrigTopoSplitter]
        else:
          self.ClusterMakerTools = [ TrigTopoMaker, TrigTopoSplitter,  TrigTopoMoments]

        # do not use BadChannelListCorr since this is not used for jet and tau in offline
        #TrigBadChannelListCorr = CaloClusterBadChannelListCorr()
        #self.ClusterCorrectionTools += [TrigBadChannelListCorr.getFullName()]
        
        self += TrigTopoMaker
        self += TrigTopoSplitter
        #self += TrigBadChannelListCorr
        if doMoments:
          self += TrigTopoMoments

        self.ClusterCorrectionTools = [  ]
        #self.ClusterCorrectionTools = [ TrigLockVariables.getFullName() ]
        #self += TrigLockVariables

        if doLC:
          self.ClusterCorrectionTools += [ TrigLocalCalib,
                                           TrigOOCCalib,
                                           TrigOOCPi0Calib,
                                           TrigDMCalib]
          self += TrigLocalCalib
          self += TrigOOCCalib
          self += TrigOOCPi0Calib
          self += TrigDMCalib

        #
        # pool/cool part
        #
        if doLC:  
          if globalflags.DetDescrVersion().startswith("Rome"):
             self.TrigLocalCalib.TrigLCClassify.MaxProbability = 0.85
             self.TrigLocalCalib.TrigLCClassify.UseNormalizedEnergyDensity = False
          else:
             self.TrigLocalCalib.TrigLCClassify.MaxProbability = 0.50
             self.TrigLocalCalib.TrigLCClassify.UseNormalizedEnergyDensity = True

        from CaloRec import CaloClusterTopoCoolFolder  # noqa: F401


def HLTCaloCellMaker(flags, name, roisKey='UNSPECIFIED', CellsName=None, monitorCells=True):
    """Wrapper for legacy job options"""
    from TriggerMenuMT.HLT.Config.MenuComponents import algorithmCAToGlobalWrapper
    cellmaker = algorithmCAToGlobalWrapper(hltCaloCellMakerCfg, flags, name, roisKey, CellsName, monitorCells)[0]
    return cellmaker


def hltCaloCellMakerCfg(flags, name=None, roisKey='UNSPECIFIED', CellsName=None, monitorCells=True):
    acc = ComponentAccumulator()
    from TrigT2CaloCommon.TrigCaloDataAccessConfig import trigCaloDataAccessSvcCfg, CaloDataAccessSvcDependencies
    acc.merge(trigCaloDataAccessSvcCfg(flags))
    # choose cells name given parameters
    cellsFromName = 'CaloCellsFS' if "FS" in name else "CaloCells"
    cells = cellsFromName if CellsName is None else CellsName

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, 'MonTool')
    monTool.defineHistogram('Cells_N', path='EXPERT', type='TH1F',  title="Cells N; NCells; events",
                            xbins=40, xmin=0, xmax=1600 if monitorCells else 240000)
    monTool.defineHistogram('TIME_exec', path='EXPERT', type='TH1F', title="Cells time; time [ us ] ; Nruns",
                            xbins=80, xmin=0, xmax=800 if monitorCells else 160000)
    if monitorCells:
        monTool.defineHistogram('Cells_eT', path='EXPERT', type='TH1F',  title="Cells E_T; E_T [ GeV ] ; Nclusters",
                                xbins=100, xmin=0.0, xmax=100.0)
        monTool.defineHistogram('Cells_eta', path='EXPERT', type='TH1F', title="Cells #eta; #eta ; Nclusters",
                                xbins=100, xmin=-2.5, xmax=2.5)
        monTool.defineHistogram('Cells_phi', path='EXPERT', type='TH1F', title="Cells #phi; #phi ; Nclusters",
                                xbins=128, xmin=-3.2, xmax=3.2)

    cellMaker = CompFactory.HLTCaloCellMaker(name,
                                             CellsName = cells,
                                             TrigDataAccessMT = acc.getService('TrigCaloDataAccessSvc'),
                                             ExtraInputs = CaloDataAccessSvcDependencies,
                                             RoIs=roisKey,
                                             monitorCells = monitorCells,
                                             MonTool = monTool)
    acc.addEventAlgo(cellMaker, primary=True)
    return acc


def hltCaloCellSeedlessMakerCfg(flags, roisKey='UNSPECIFIED'):
    acc = ComponentAccumulator()
    hltCaloCellMakerAcc = hltCaloCellMakerCfg(flags, "CaloCellSeedLessFS",
                                                    roisKey = roisKey,
                                                    CellsName ="SeedLessFS", 
                                                    monitorCells=False)

    acc.merge(hltCaloCellMakerAcc)

    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    acc.merge(CaloNoiseCondAlgCfg(flags, noisetype="electronicNoise"))
    acc.addCondAlgo(CompFactory.CaloNoiseSigmaDiffCondAlg())

    return acc


def hltTopoClusterMakerCfg(flags, name, clustersKey, cellsKey="CaloCells"):
    acc = ComponentAccumulator()
    from CaloRec.CaloTopoClusterConfig import (
        CaloTopoClusterToolCfg,
        CaloTopoClusterSplitterToolCfg,
    )

    topoMaker = acc.popToolsAndMerge(CaloTopoClusterToolCfg(flags, cellsname=cellsKey))

    #timing
    topoMaker.SeedCutsInT = flags.Trigger.Calo.TopoCluster.doTimeCut
    topoMaker.CutOOTseed = flags.Trigger.Calo.TopoCluster.extendTimeCut and flags.Trigger.Calo.TopoCluster.doTimeCut
    topoMaker.UseTimeCutUpperLimit = flags.Trigger.Calo.TopoCluster.useUpperLimitForTimeCut
    topoMaker.TimeCutUpperLimit = flags.Trigger.Calo.TopoCluster.timeCutUpperLimit
    
    topoSplitter = acc.popToolsAndMerge(CaloTopoClusterSplitterToolCfg(flags))

    topoMoments = CompFactory.CaloClusterMomentsMaker ('TrigTopoMoments')
    topoMoments.MaxAxisAngle = 20*deg
    topoMoments.TwoGaussianNoise = flags.Calo.TopoCluster.doTwoGaussianNoise
    topoMoments.MinBadLArQuality = 4000
    topoMoments.MomentsNames = ['FIRST_PHI',
                                'FIRST_ETA',
                                'SECOND_R' ,
                                'SECOND_LAMBDA',
                                'DELTA_PHI',
                                'DELTA_THETA',
                                'DELTA_ALPHA' ,
                                'CENTER_X',
                                'CENTER_Y',
                                'CENTER_Z',
                                'CENTER_MAG',
                                'CENTER_LAMBDA',
                                'LATERAL',
                                'LONGITUDINAL',
                                'FIRST_ENG_DENS',
                                'ENG_FRAC_EM',
                                'ENG_FRAC_MAX',
                                'ENG_FRAC_CORE' ,
                                'FIRST_ENG_DENS',
                                'SECOND_ENG_DENS',
                                'ISOLATION',
                                'ENG_BAD_CELLS',
                                'N_BAD_CELLS',
                                'N_BAD_CELLS_CORR',
                                'BAD_CELLS_CORR_E',
                                'BADLARQ_FRAC',
                                'ENG_POS',
                                'SIGNIFICANCE',
                                'CELL_SIGNIFICANCE',
                                'CELL_SIG_SAMPLING',
                                'AVG_LAR_Q',
                                'AVG_TILE_Q'
                                ]

    doMonCells = "FS" in name
    alg = CompFactory.TrigCaloClusterMaker(
        name,
        Cells=cellsKey,
        CaloClusters=recordable(clustersKey),
        ClusterMakerTools = [ topoMaker, topoSplitter, topoMoments], # moments are missing yet
        MonCells = doMonCells,
        MonTool = trigCaloClusterMakerMonTool(flags, doMonCells) )

    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    acc.merge(CaloNoiseCondAlgCfg(flags))
    acc.addEventAlgo(alg, primary=True)
    return acc


def hltCaloTopoClusteringCfg(
    flags, namePrefix=None, roisKey="UNSPECIFIED", clustersKey="HLT_TopoCaloClustersRoI"
):
    acc = ComponentAccumulator()
    acc.merge(
        hltCaloCellMakerCfg(flags, namePrefix + "HLTCaloCellMaker", roisKey=roisKey)
    )
    acc.merge(
        hltTopoClusterMakerCfg(
            flags, namePrefix + "TrigCaloClusterMaker_topo", clustersKey=clustersKey
        )
    )
    return acc


def hltCaloTopoClusterCalibratorCfg(flags, name, clustersin, clustersout, **kwargs):
    """ Create the LC calibrator """
    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg

    # We need the electronic noise for the LC weights
    acc = ComponentAccumulator()
    acc.merge(CaloNoiseCondAlgCfg(flags, noisetype="electronicNoise"))

    from CaloRec.CaloTopoClusterConfig import caloTopoCoolFolderCfg
    acc.merge(caloTopoCoolFolderCfg(flags))

    # Figure out the detector version
    det_version_is_rome = flags.GeoModel.AtlasVersion.startswith("Rome")

    calibrator = CompFactory.TrigCaloClusterCalibrator(
        name, InputClusters=clustersin, OutputClusters=clustersout, **kwargs
    )

    calibrator.ClusterCorrectionTools = [
        CompFactory.CaloClusterLocalCalib(
            "TrigLocalCalib",
            ClusterRecoStatus=[1, 2],
            ClusterClassificationTool=[
                CompFactory.CaloLCClassificationTool(
                    "TrigLCClassify",
                    ClassificationKey="EMFracClassify",
                    UseSpread=False,
                    MaxProbability=0.85 if det_version_is_rome else 0.5,
                    UseNormalizedEnergyDensity=not det_version_is_rome,
                    StoreClassificationProbabilityInAOD=True,
                ),
            ],
            LocalCalibTools=[
                CompFactory.CaloLCWeightTool(
                    "TrigLCWeight",
                    CorrectionKey="H1ClusterCellWeights",
                    SignalOverNoiseCut=2.0,
                    UseHadProbability=True,
                )
            ],
        ),
        CompFactory.CaloClusterLocalCalib(
            "TrigOOCCalib",
            ClusterRecoStatus=[1, 2],
            LocalCalibTools=[
                CompFactory.CaloLCOutOfClusterTool(
                    "TrigLCOut",
                    CorrectionKey="OOCCorrection",
                    UseEmProbability=False,
                    UseHadProbability=True,
                ),
            ],
        ),
        CompFactory.CaloClusterLocalCalib(
            "TrigOOCPi0Calib",
            ClusterRecoStatus=[1, 2],
            LocalCalibTools=[
                CompFactory.CaloLCOutOfClusterTool(
                    "TrigLCOutPi0",
                    CorrectionKey="OOCPi0Correction",
                    UseEmProbability=True,
                    UseHadProbability=False,
                ),
            ],
        ),
        CompFactory.CaloClusterLocalCalib(
            "TrigDMCalib",
            ClusterRecoStatus=[1, 2],
            LocalCalibTools=[
                CompFactory.CaloLCDeadMaterialTool(
                    "TrigLCDeadMaterial",
                    HadDMCoeffKey="HadDMCoeff2",
                    ClusterRecoStatus=0,
                    WeightModeDM=2,
                    UseHadProbability=True,
                )
            ],
        ),
    ]  # End ClusterCorrectionTools
    #NB: Could we take these from CaloRec.CaloTopoClusterConfig.getTopoClusterLocalCalibTools?

    # Monitoring
    monTool = GenericMonitoringTool(flags, "MonTool")
    monTool.defineHistogram('Et', path='EXPERT', type='TH1F',
                            title="Cluster E_T; E_T [ MeV ] ; Number of Clusters",
                            xbins=135, xmin=-200.0, xmax=2500.0)
    monTool.defineHistogram('Eta', path='EXPERT', type='TH1F',
                            title="Cluster #eta; #eta ; Number of Clusters",
                            xbins=100, xmin=-2.5, xmax=2.5)
    monTool.defineHistogram('Phi', path='EXPERT', type='TH1F',
                            title="Cluster #phi; #phi ; Number of Clusters",
                            xbins=64, xmin=-3.2, xmax=3.2)
    monTool.defineHistogram('Eta,Phi', path='EXPERT', type='TH2F',
                            title="Number of Clusters; #eta ; #phi ; Number of Clusters",
                            xbins=100, xmin=-2.5, xmax=2.5, ybins=128, ymin=-3.2, ymax=3.2)
    calibrator.MonTool = monTool

    acc.addEventAlgo(calibrator, primary=True)
    return acc

## Heavy Ion 
class TrigCaloTowerMaker_hijet (TrigCaloTowerMakerBase):
    __slots__ = []
    def __init__ (self, name='TrigCaloTowerMaker_hijet'):
        super(TrigCaloTowerMaker_hijet, self).__init__(name)

        # input to LArTowerBuilder:  cells in LArEM and LARHEC 
        from LArRecUtils.LArRecUtilsConf import LArTowerBuilderTool,LArFCalTowerBuilderTool

        larcmbtwrbldr = LArTowerBuilderTool("LArCmbTwrBldr",
                                            CellContainerName = "AllCalo",
                                            IncludedCalos     = [ "LAREM", "LARHEC" ]
                                            )
        
        fcalcmbtwrbldr = LArFCalTowerBuilderTool("FCalCmbTwrBldr",
                                                 CellContainerName = "AllCalo",
                                                 MinimumEt         = 0.*MeV
                                                 )

        #input to  TileTowerBuilder:  cells in TILE
        from TileRecUtils.TileRecUtilsConf import TileTowerBuilderTool
        tilecmbtwrbldr = TileTowerBuilderTool("TileCmbTwrBldr",
                                              CellContainerName = "AllCalo",
                                              #DumpTowers        = False,
                                              #DumpWeightMap     = False
                                              )

        
        self +=larcmbtwrbldr
        self +=fcalcmbtwrbldr
        self +=tilecmbtwrbldr
        self.NumberOfPhiTowers=64
        self.NumberOfEtaTowers=100
        self.EtaMin=-5.0
        self.EtaMax=5.0
        self.DeltaEta=1.2
        self.DeltaPhi=1.2
        self.TowerMakerTools = [ tilecmbtwrbldr.getFullName(), larcmbtwrbldr.getFullName(), fcalcmbtwrbldr.getFullName() ]

def hltHICaloTowerMakerCfg(flags, name, clustersKey, cellsKey="CaloCells"):
    acc = ComponentAccumulator()

    alg = CompFactory.TrigCaloTowerMaker(name,
                                             Cells=cellsKey,
                                             CaloClusters=recordable(clustersKey),
                                            )
    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    acc.merge(CaloNoiseCondAlgCfg(flags))
    acc.addEventAlgo(alg, primary=True)
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW
    flags.Input.isMC=False
    flags.lock()    
    hltCaloCellSeedlessMakerCfg(flags).printConfig(withDetails=True)
    hltCaloCellMakerCfg(flags, "SthFS").printConfig(withDetails=True)
