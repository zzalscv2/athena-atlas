# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.SystemOfUnits import MeV, deg
from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool

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

def hltCaloCellCorrectorCfg(flags,name='HLTCaloCellCorrector', inputEDM='CellsClusters', outputEDM='CorrectedCellsClusters', eventShape='HIEventShape'):
    acc = ComponentAccumulator()
    cellCorrector = CompFactory.HLTCaloCellCorrector(name = name,
                                                     EventShapeCollection = eventShape,
                                                     InputCellKey = inputEDM,
                                                     OutputCellKey = outputEDM)
    acc.addEventAlgo(cellCorrector)
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


def hltCaloLocalCalib(flags, name = "TrigLocalCalib"):
    det_version_is_rome = flags.GeoModel.AtlasVersion.startswith("Rome")
    localCalibTool = CompFactory.CaloLCWeightTool("TrigLCWeight",
           CorrectionKey="H1ClusterCellWeights",
           SignalOverNoiseCut=2.0, UseHadProbability=True)
    trigLCClassify = CompFactory.CaloLCClassificationTool("TrigLCClassify",
           ClassificationKey="EMFracClassify",
           UseSpread=False, MaxProbability=0.85 if det_version_is_rome else 0.5,
           UseNormalizedEnergyDensity=not det_version_is_rome,
           StoreClassificationProbabilityInAOD=True)
    tool = CompFactory.CaloClusterLocalCalib( name,
           ClusterRecoStatus=[1, 2], ClusterClassificationTool=[ trigLCClassify ],
           LocalCalibTools=[ localCalibTool ])
    return tool


def hltCaloOOCalib(flags, name = "TrigOOCCalib"):
    localCalibTool = CompFactory.CaloLCOutOfClusterTool("TrigLCOut",
           CorrectionKey="OOCCorrection",UseEmProbability=False,
           UseHadProbability=True)
    tool = CompFactory.CaloClusterLocalCalib( name,
           ClusterRecoStatus=[1, 2],
           LocalCalibTools=[ localCalibTool ] )
    return tool

def hltCaloOOCPi0Calib(flags, name = "TrigOOCPi0Calib" ):
    localCalibTool = CompFactory.CaloLCOutOfClusterTool("TrigLCOutPi0",
           CorrectionKey="OOCPi0Correction", UseEmProbability=True,
           UseHadProbability=False)
    tool = CompFactory.CaloClusterLocalCalib( name,
           ClusterRecoStatus=[1, 2],
           LocalCalibTools=[ localCalibTool ] )
    return tool

def hltCaloDMCalib(flags, name = "TrigDMCalib" ):
    localCalibTool = CompFactory.CaloLCDeadMaterialTool("TrigLCDeadMaterial",
           HadDMCoeffKey="HadDMCoeff2", ClusterRecoStatus=0,
           WeightModeDM=2,UseHadProbability=True)
    tool = CompFactory.CaloClusterLocalCalib( name,
            ClusterRecoStatus=[1, 2],
            LocalCalibTools=[ localCalibTool ] )
    return tool



def hltTopoClusterMakerCfg(flags, name, clustersKey="HLT_TopoCaloClustersFS",
                           cellsKey="CaloCells", doLC=True):
    acc = ComponentAccumulator()

    from CaloRec.CaloTopoClusterConfig import (
        CaloTopoClusterToolCfg,
        CaloTopoClusterSplitterToolCfg,
    )

    topoMaker = acc.popToolsAndMerge(CaloTopoClusterToolCfg(flags, cellsname=cellsKey))
    topoMaker.RestrictPSNeighbors = False
    listClusterCorrectionTools = []
    if doLC :
       from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
       # We need the electronic noise for the LC weights
       acc.merge(CaloNoiseCondAlgCfg(flags, noisetype="electronicNoise"))
       from CaloRec.CaloTopoClusterConfig import caloTopoCoolFolderCfg
       acc.merge(caloTopoCoolFolderCfg(flags))
       listClusterCorrectionTools = [ hltCaloLocalCalib(flags), hltCaloOOCalib(flags),
             hltCaloOOCPi0Calib(flags), hltCaloDMCalib(flags) ]

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
        ClusterCorrectionTools = listClusterCorrectionTools,
        MonCells = doMonCells,
        MonTool = trigCaloClusterMakerMonTool(flags, doMonCells) )

    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    acc.merge(CaloNoiseCondAlgCfg(flags))
    acc.addEventAlgo(alg, primary=True)
    return acc


def hltCaloTopoClusteringCfg(
    flags, namePrefix=None,nameSuffix=None, CellsName=None, roisKey="UNSPECIFIED",clustersKey="HLT_TopoCaloClustersRoI", doLC=False):
    acc = ComponentAccumulator()
    acc.merge(
        hltCaloCellMakerCfg(flags, namePrefix + "HLTCaloCellMaker"+nameSuffix, roisKey=roisKey, CellsName=CellsName)
    )
    acc.merge(
        hltTopoClusterMakerCfg(
            flags, namePrefix + "TrigCaloClusterMaker_topo"+nameSuffix, clustersKey=clustersKey, doLC=doLC
        )
    )
    return acc

def hltCaloTopoClusteringHICfg(
    flags, namePrefix=None, CellsName=None, roisKey="UNSPECIFIED", doLC=False,algSuffix='HIRoI', ion=True):
    from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import  getTrigEgammaKeys
    TrigEgammaKeys = getTrigEgammaKeys(ion=ion)
    eventShape = TrigEgammaKeys.egEventShape
    clustersKey = TrigEgammaKeys.precisionTopoClusterContainer
    acc = ComponentAccumulator()
    acc.merge(hltCaloCellMakerCfg(flags, namePrefix + "HLTCaloCellMaker"+algSuffix, roisKey=roisKey, CellsName=CellsName))
    acc.merge(hltCaloCellCorrectorCfg(flags,name='HLTRoICaloCellCorrector', inputEDM='CaloCells', outputEDM='CorrectedRoICaloCells', eventShape=eventShape))
    acc.merge(hltTopoClusterMakerCfg(flags, namePrefix + "TrigCaloClusterMaker_topo"+algSuffix, clustersKey=clustersKey,cellsKey="CorrectedRoICaloCells", doLC=doLC))
    return acc

def hltCaloTopoClusterCalibratorCfg(flags, name, clustersin, clustersout, **kwargs):
    """ Create the LC calibrator """
    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg

    # We need the electronic noise for the LC weights
    acc = ComponentAccumulator()
    acc.merge(CaloNoiseCondAlgCfg(flags, noisetype="electronicNoise"))

    from CaloRec.CaloTopoClusterConfig import caloTopoCoolFolderCfg
    acc.merge(caloTopoCoolFolderCfg(flags))

    calibrator = CompFactory.TrigCaloClusterCalibrator(
        name, InputClusters=clustersin, OutputClusters=clustersout, 
        **kwargs
        #OutputCellLinks = clustersout+"_cellLinks", **kwargs
    )

    calibrator.ClusterCorrectionTools = [ hltCaloLocalCalib(flags), hltCaloOOCalib(flags),
             hltCaloOOCPi0Calib(flags), hltCaloDMCalib(flags) ]
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


def hltHICaloTowerMakerCfg(flags, name, towersKey, cellsKey="CaloCellsFS", RoIs=""):
    acc = ComponentAccumulator()
    larcmbtwrbldr = CompFactory.LArTowerBuilderTool("LArCmbTwrBldr",
                                        CellContainerName = cellsKey,
                                        IncludedCalos     = [ "LAREM", "LARHEC" ]
                                        )
    
    fcalcmbtwrbldr = CompFactory.LArFCalTowerBuilderTool("FCalCmbTwrBldr",
                                                CellContainerName = cellsKey,
                                                MinimumEt         = 0.*MeV
                                                )

    #input to  TileTowerBuilder:  cells in TILE
    tilecmbtwrbldr = CompFactory.TileTowerBuilderTool("TileCmbTwrBldr",
                                            CellContainerName = cellsKey,
                                            # debugging aid, keep for convenience
                                            #DumpTowers        = False,
                                            #DumpWeightMap     = False
                                            )



    alg = CompFactory.TrigCaloTowerMaker(name,
                                        Cells=cellsKey,
                                        CaloTowers=towersKey,
                                        NumberOfPhiTowers=64,
                                        NumberOfEtaTowers=100,
                                        EtaMin=-5.0,
                                        EtaMax=5.0,
                                        DeltaEta=1.2,
                                        DeltaPhi=1.2,
                                        RoIs=RoIs,
                                        TowerMakerTools = [ tilecmbtwrbldr, larcmbtwrbldr, fcalcmbtwrbldr ]
                                        )
    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    acc.merge(CaloNoiseCondAlgCfg(flags))
    acc.addEventAlgo(alg, primary=True)
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.Enums import LHCPeriod

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN3
    flags.Input.isMC=False
    flags.GeoModel.Run=LHCPeriod.Run3
    flags.lock()    
    CAs = [hltCaloCellSeedlessMakerCfg(flags),
           hltCaloCellMakerCfg(flags, "SthFS"),
           hltTopoClusterMakerCfg(flags, "TrigCaloClusterMaker_topo"),
           hltCaloTopoClusterCalibratorCfg(flags,"Calibrator",
                                           clustersin="clustersIn",clustersout="clustersOut")]

    for ca in CAs:
        ca.printConfig(withDetails=True, summariseProps=True)
        ca.wasMerged()
