# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format
from AthenaCommon.SystemOfUnits import MeV, ns, cm

class CaloRecGPUConfigurator:
    def __init__ (self, configFlags = None, cellsname ="AllCalo"):
        self.ConfigFlags = configFlags
        self.CellsName = cellsname
        
        self.TopoClusterSNRSeedThreshold = 4.0
        self.TopoClusterSNRGrowThreshold = 2.0
        self.TopoClusterSNRCellThreshold = 0.0
        self.TopoClusterSeedCutsInAbsE = True
        self.TopoClusterNeighborCutsInAbsE = True
        self.TopoClusterCellCutsInAbsE = True
        
        self.CalorimeterNames = ["LAREM",
                                 "LARHEC",
                                 "LARFCAL",
                                 "TILE"]
        self.TopoClusterSeedSamplingNames = ["PreSamplerB", "EMB1", "EMB2", "EMB3",
                                  "PreSamplerE", "EME1", "EME2", "EME3",
                                  "HEC0", "HEC1","HEC2", "HEC3",
                                  "TileBar0", "TileBar1", "TileBar2",
                                  "TileExt0", "TileExt1", "TileExt2",
                                  "TileGap1", "TileGap2", "TileGap3",
                                  "FCAL0", "FCAL1", "FCAL2"]
                                  
        self.NeighborOption = "super3D"
        self.RestrictHECIWandFCalNeighbors  = False
        self.RestrictPSNeighbors  = True
        
        if configFlags is not None:
            self.SeedCutsInT = configFlags.Calo.TopoCluster.doTimeCut
            self.CutOOTseed = configFlags.Calo.TopoCluster.extendTimeCut and configFlags.Calo.TopoCluster.doTimeCut
            self.UseTimeCutUpperLimit = configFlags.Calo.TopoCluster.useUpperLimitForTimeCut
        else:
            self.SeedCutsInT = False
            self.CutOOTseed = False
            self.UseTimeCutUpperLimit = False
            
        self.TimeCutUpperLimit = 20.0
        self.TreatL1PredictedCellsAsGood = True
        
        self.SeedThresholdOnTAbs = 12.5 * ns
        
        self.CutClustersInAbsEt = True
        self.ClusterEtorAbsEtCut = 0.0*MeV
        
        if configFlags is not None:
            self.TwoGaussianNoise = configFlags.Calo.TopoCluster.doTwoGaussianNoise
        else:
            self.TwoGaussianNoise = False
        
        
        self.SplitterNumberOfCellsCut = 4
        self.SplitterEnergyCut = 500 * MeV
        self.SplitterSamplingNames = ["EMB2", "EMB3",
                                      "EME2", "EME3",
                                      "FCAL0"]
        self.SplitterSecondarySamplingNames = ["EMB1","EME1",
                                               "TileBar0","TileBar1","TileBar2",
                                               "TileExt0","TileExt1","TileExt2",
                                               "HEC0","HEC1","HEC2","HEC3",
                                               "FCAL1","FCAL2"]
        self.SplitterShareBorderCells = False # True
                                              # NSF: As far as I am aware, this option does not yet work
                                              #      on the GPU implementation we currently have available...
        self.EMShowerScale = 5.0 * cm
        
        
        if configFlags is not None:
            self.SplitterUseNegativeClusters = configFlags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute
        else:
            self.SplitterUseNegativeClusters = False
        
        
        self.MeasureTimes = True
        self.TestGPUGrowing = False
        self.TestGPUSplitting = False
        self.OutputCountsToFile = False
        self.OutputClustersToFile = False
        self.ClustersOutputName = "Clusters"
        
        self.NumPreAllocatedDataHolders = 0
            
    def BasicConstantDataExporterToolConf(self, name = "ConstantDataExporter"):
        result=ComponentAccumulator()
        ConstantDataExporter = CompFactory.BasicConstantGPUDataExporter(name)
        ConstantDataExporter.MeasureTimes = False
        ConstantDataExporter.TimeFileOutput = "ConstantDataExporterTimes.txt"
        ConstantDataExporter.NeighborOption = self.NeighborOption
        ConstantDataExporter.RestrictHECIWandFCalNeighbors  = self.RestrictHECIWandFCalNeighbors
        ConstantDataExporter.RestrictPSNeighbors  = self.RestrictPSNeighbors
        result.setPrivateTools(ConstantDataExporter)
        return result

    def BasicEventDataExporterToolConf(self, name = "EventDataExporter"):
        result=ComponentAccumulator()
        EventDataExporter = CompFactory.BasicEventDataGPUExporter(name)
        EventDataExporter.MeasureTimes = self.MeasureTimes
        EventDataExporter.TimeFileOutput = "EventDataExporterTimes.txt"
        EventDataExporter.CellsName = self.CellsName
        EventDataExporter.SeedCutsInT = self.SeedCutsInT
        EventDataExporter.CutOOTseed = self.CutOOTseed
        EventDataExporter.UseTimeCutUpperLimit = self.UseTimeCutUpperLimit
        EventDataExporter.TimeCutUpperLimit = self.TimeCutUpperLimit
        EventDataExporter.SeedThresholdOnEorAbsEinSigma = self.TopoClusterSNRSeedThreshold
        EventDataExporter.SeedThresholdOnTAbs = self.SeedThresholdOnTAbs
        EventDataExporter.TreatL1PredictedCellsAsGood = self.TreatL1PredictedCellsAsGood
        result.setPrivateTools(EventDataExporter)
        return result

    def BasicAthenaClusterImporterToolConf(self, name = "AthenaClusterImporter"):
        result=ComponentAccumulator()
        AthenaClusterImporter = CompFactory.BasicGPUToAthenaImporter(name)
        AthenaClusterImporter.MeasureTimes = self.MeasureTimes
        AthenaClusterImporter.TimeFileOutput = "ClusterImporterTimes.txt"
        AthenaClusterImporter.CellsName = self.CellsName
        AthenaClusterImporter.ClusterCutsInAbsE = self.CutClustersInAbsEt
        AthenaClusterImporter.ClusterEtorAbsEtCut = self.ClusterEtorAbsEtCut
        result.setPrivateTools(AthenaClusterImporter)
        return result

    def CaloClusterDeleterToolConf(self, name = "ClusterDeleter"):
        result=ComponentAccumulator()
        ClusterDeleter = CompFactory.CaloClusterDeleter(name)
        result.setPrivateTools(ClusterDeleter)
        return result
        
    def CPUOutputToolConf(self, folder, name = "CPUOutput", prefix = "", suffix = ""):
        result=ComponentAccumulator()
        CPUOutput = CompFactory.CaloCPUOutput(name)
        CPUOutput.SavePath = folder
        CPUOutput.FilePrefix = prefix
        CPUOutput.FileSuffix = suffix
        CPUOutput.CellsName = self.CellsName
        CPUOutput.SeedCutsInT = self.SeedCutsInT
        CPUOutput.CutOOTseed = self.CutOOTseed
        CPUOutput.UseTimeCutUpperLimit = self.UseTimeCutUpperLimit
        CPUOutput.TimeCutUpperLimit = self.TimeCutUpperLimit
        CPUOutput.SeedThresholdOnEorAbsEinSigma = self.TopoClusterSNRSeedThreshold
        CPUOutput.SeedThresholdOnTAbs = self.SeedThresholdOnTAbs
        CPUOutput.TreatL1PredictedCellsAsGood = self.TreatL1PredictedCellsAsGood
        result.setPrivateTools(CPUOutput)
        return result

    def GPUOutputToolConf(self, folder, name = "GPUOutput", prefix = "", suffix = ""):
        result=ComponentAccumulator()
        GPUOutput = CompFactory.CaloGPUOutput(name)
        GPUOutput.SavePath = folder
        GPUOutput.FilePrefix = prefix
        GPUOutput.FileSuffix = suffix
        GPUOutput.ClusterCutsInAbsE = self.CutClustersInAbsEt
        GPUOutput.ClusterEtorAbsEtCut = self.ClusterEtorAbsEtCut
        GPUOutput.UseSortedAndCutClusters = True
        result.setPrivateTools(GPUOutput)
        return result
        
    def ClusterInfoCalcToolConf(self, name = "GPUClusterInfoCalculator"):
        result=ComponentAccumulator()
        CalcTool = CompFactory.BasicGPUClusterInfoCalculator(name)
        CalcTool.MeasureTimes = self.MeasureTimes
        CalcTool.TimeFileOutput = name + "Times.txt"
        CalcTool.NumPreAllocatedDataHolders = self.NumPreAllocatedDataHolders
        result.setPrivateTools(CalcTool)
        return result
                
    def TopoAutomatonClusteringToolConf(self, name = "TAClusterMaker"):
        result=ComponentAccumulator()
        # maker tools
        TAClusterMaker = CompFactory.TopoAutomatonClustering(name)

        TAClusterMaker.MeasureTimes = self.MeasureTimes
        TAClusterMaker.TimeFileOutput = "TopoAutomatonClusteringTimes.txt"

        TAClusterMaker.SeedSamplingNames = self.TopoClusterSeedSamplingNames
        
        TAClusterMaker.CellThresholdOnEorAbsEinSigma = self.TopoClusterSNRCellThreshold
        TAClusterMaker.NeighborThresholdOnEorAbsEinSigma = self.TopoClusterSNRGrowThreshold
        TAClusterMaker.SeedThresholdOnEorAbsEinSigma = self.TopoClusterSNRSeedThreshold
        
        TAClusterMaker.SeedCutsInAbsE = self.TopoClusterSeedCutsInAbsE
        TAClusterMaker.NeighborCutsInAbsE = self.TopoClusterNeighborCutsInAbsE
        TAClusterMaker.CellCutsInAbsE = self.TopoClusterCellCutsInAbsE
        
        TAClusterMaker.TwoGaussianNoise = self.TwoGaussianNoise
        
        TAClusterMaker.NumPreAllocatedDataHolders = self.NumPreAllocatedDataHolders
        
        result.setPrivateTools(TAClusterMaker)
        return result
        
    def DefaultTopologicalClusteringToolConf(self, name = "TopoMaker"):
        result=ComponentAccumulator()
        # maker tools
        TopoMaker = CompFactory.CaloTopoClusterMaker(name)

        TopoMaker.CellsName = self.CellsName
        TopoMaker.CalorimeterNames= self.CalorimeterNames
        TopoMaker.SeedSamplingNames = self.TopoClusterSeedSamplingNames
        TopoMaker.NeighborOption = self.NeighborOption
        TopoMaker.RestrictHECIWandFCalNeighbors  = self.RestrictHECIWandFCalNeighbors
        TopoMaker.RestrictPSNeighbors  = self.RestrictPSNeighbors
        TopoMaker.CellThresholdOnEorAbsEinSigma = self.TopoClusterSNRCellThreshold
        TopoMaker.NeighborThresholdOnEorAbsEinSigma = self.TopoClusterSNRGrowThreshold
        TopoMaker.SeedThresholdOnEorAbsEinSigma = self.TopoClusterSNRSeedThreshold

        TopoMaker.SeedCutsInT = self.SeedCutsInT
        TopoMaker.CutOOTseed = self.CutOOTseed
        TopoMaker.UseTimeCutUpperLimit = self.UseTimeCutUpperLimit
        TopoMaker.TimeCutUpperLimit = self.TimeCutUpperLimit

        TopoMaker.ClusterEtorAbsEtCut  = self.ClusterEtorAbsEtCut
        TopoMaker.TwoGaussianNoise = self.TwoGaussianNoise
        TopoMaker.SeedCutsInAbsE = self.TopoClusterSeedCutsInAbsE
        TopoMaker.NeighborCutsInAbsE = self.TopoClusterNeighborCutsInAbsE
        TopoMaker.CellCutsInAbsE = self.TopoClusterCellCutsInAbsE
        TopoMaker.SeedThresholdOnTAbs = self.SeedThresholdOnTAbs
        
        TopoMaker.TreatL1PredictedCellsAsGood = self.TreatL1PredictedCellsAsGood
        result.setPrivateTools(TopoMaker)
        return result
        
        
    def GPUClusterSplitterToolConf(self, name = "GPUClusterSplitter"):
        result=ComponentAccumulator()
        # maker tools
        Splitter = CompFactory.CaloTopoClusterSplitterGPU(name)

        Splitter.MeasureTimes = self.MeasureTimes
        Splitter.TimeFileOutput = "ClusterSplitterTimes.txt"
        
        Splitter.NumberOfCellsCut = self.SplitterNumberOfCellsCut
        Splitter.EnergyCut = self.SplitterEnergyCut
        Splitter.SamplingNames = self.SplitterSamplingNames
        Splitter.SecondarySamplingNames = self.SplitterSecondarySamplingNames
        Splitter.ShareBorderCells = self.SplitterShareBorderCells
        Splitter.EMShowerScale = self.EMShowerScale
        Splitter.WeightingOfNegClusters = self.SplitterUseNegativeClusters
        
        Splitter.NumPreAllocatedDataHolders = self.NumPreAllocatedDataHolders
        
        result.setPrivateTools(Splitter)
        return result
        
    def DefaultClusterSplittingToolConf(self, name = "TopoSplitter"):        
        result=ComponentAccumulator()
        # maker tools
        TopoSplitter = CompFactory.CaloTopoClusterSplitter(name)

        
        TopoSplitter.NeighborOption = self.NeighborOption
        TopoSplitter.RestrictHECIWandFCalNeighbors  = self.RestrictHECIWandFCalNeighbors
        
        TopoSplitter.NumberOfCellsCut = self.SplitterNumberOfCellsCut
        TopoSplitter.EnergyCut = self.SplitterEnergyCut
        
        TopoSplitter.SamplingNames = self.SplitterSamplingNames
        TopoSplitter.SecondarySamplingNames = self.SplitterSecondarySamplingNames
        
        TopoSplitter.ShareBorderCells = self.SplitterShareBorderCells
        TopoSplitter.EMShowerScale = self.EMShowerScale
        
        TopoSplitter.TreatL1PredictedCellsAsGood = self.TreatL1PredictedCellsAsGood
        
        TopoSplitter.WeightingOfNegClusters = self.SplitterUseNegativeClusters
        
        result.setPrivateTools(TopoSplitter)
        return result
        
        
    def CellsCounterCPUToolConf(self, folder, name = "CPUCounts", prefix = "CPU", suffix = ""):
        result=ComponentAccumulator()
        CPUCount = CompFactory.CaloCellsCounterCPU(name)
        CPUCount.SavePath = folder
        CPUCount.FilePrefix = prefix
        CPUCount.FileSuffix = suffix
        CPUCount.CellsName = self.CellsName
        CPUCount.SeedCutsInT = self.SeedCutsInT
        CPUCount.CutOOTseed = self.CutOOTseed
        CPUCount.UseTimeCutUpperLimit = self.UseTimeCutUpperLimit
        CPUCount.TimeCutUpperLimit = self.TimeCutUpperLimit
        
        CPUCount.CellThresholdOnEorAbsEinSigma = self.TopoClusterSNRCellThreshold
        CPUCount.NeighborThresholdOnEorAbsEinSigma = self.TopoClusterSNRGrowThreshold
        CPUCount.SeedThresholdOnEorAbsEinSigma = self.TopoClusterSNRSeedThreshold
        
        CPUCount.SeedThresholdOnTAbs = self.SeedThresholdOnTAbs
        CPUCount.TreatL1PredictedCellsAsGood = self.TreatL1PredictedCellsAsGood
        
        result.setPrivateTools(CPUCount)
        return result
        
    def CellsCounterGPUToolConf(self, folder, name = "GPUCounts", prefix = "GPU", suffix = ""):
        result=ComponentAccumulator()
        GPUCount = CompFactory.CaloCellsCounterGPU(name)
        GPUCount.SavePath = folder
        GPUCount.FilePrefix = prefix
        GPUCount.FileSuffix = suffix
        
        GPUCount.CellThresholdOnEorAbsEinSigma = self.TopoClusterSNRCellThreshold
        GPUCount.NeighborThresholdOnEorAbsEinSigma = self.TopoClusterSNRGrowThreshold
        GPUCount.SeedThresholdOnEorAbsEinSigma = self.TopoClusterSNRSeedThreshold
        
        result.setPrivateTools(GPUCount)
        return result
        
    
        
    
    def DefaultCaloCellMakerConf(self):
        from LArCellRec.LArCellBuilderConfig import LArCellBuilderCfg,LArCellCorrectorCfg
        from TileRecUtils.TileCellBuilderConfig import TileCellBuilderCfg
        from CaloCellCorrection.CaloCellCorrectionConfig import CaloCellPedestalCorrCfg, CaloCellNeighborsAverageCorrCfg, CaloCellTimeCorrCfg, CaloEnergyRescalerCfg
        result=ComponentAccumulator()
       
        from LArGeoAlgsNV.LArGMConfig import LArGMCfg
        from TileGeoModel.TileGMConfig import TileGMCfg
        
        result.merge(LArGMCfg(self.ConfigFlags))
        result.merge(TileGMCfg(self.ConfigFlags))

        larCellBuilder     = result.popToolsAndMerge(LArCellBuilderCfg(self.ConfigFlags))
        larCellCorrectors  = result.popToolsAndMerge(LArCellCorrectorCfg(self.ConfigFlags))
        tileCellBuilder = result.popToolsAndMerge(TileCellBuilderCfg(self.ConfigFlags))
        cellFinalizer  = CompFactory.CaloCellContainerFinalizerTool()

        cellMakerTools=[larCellBuilder,tileCellBuilder,cellFinalizer]+larCellCorrectors

        #Add corrections tools that are not LAr or Tile specific:
        if self.ConfigFlags.Calo.Cell.doPileupOffsetBCIDCorr or self.ConfigFlags.Cell.doPedestalCorr:
            theCaloCellPedestalCorr=CaloCellPedestalCorrCfg(self.ConfigFlags)
            cellMakerTools.append(result.popToolsAndMerge(theCaloCellPedestalCorr))

        #LAr HV scale corr must come after pedestal corr
        if self.ConfigFlags.LAr.doHVCorr:
            from LArCellRec.LArCellBuilderConfig import LArHVCellContCorrCfg
            theLArHVCellContCorr=LArHVCellContCorrCfg(self.ConfigFlags)
            cellMakerTools.append(result.popToolsAndMerge(theLArHVCellContCorr))


        if self.ConfigFlags.Calo.Cell.doDeadCellCorr:
            theCaloCellNeighborAvg=CaloCellNeighborsAverageCorrCfg(self.ConfigFlags)
            cellMakerTools.append(result.popToolsAndMerge(theCaloCellNeighborAvg))

        if self.ConfigFlags.Calo.Cell.doEnergyCorr:
            theCaloCellEnergyRescaler=CaloEnergyRescalerCfg(self.ConfigFlags)
            cellMakerTools.append(result.popToolsAndMerge(theCaloCellEnergyRescaler))
        if self.ConfigFlags.Calo.Cell.doTimeCorr:
            theCaloTimeCorr=CaloCellTimeCorrCfg(self.ConfigFlags)
            cellMakerTools.append(result.popToolsAndMerge(theCaloTimeCorr))


        #Old Config:
        #CaloCellMakerToolNames': PrivateToolHandleArray(['LArCellBuilderFromLArRawChannelTool/LArCellBuilderFromLArRawChannelTool','TileCellBuilder/TileCellBuilder','CaloCellContainerFinalizerTool/CaloCellContainerFinalizerTool','LArCellNoiseMaskingTool/LArCellNoiseMaskingTool','CaloCellPedestalCorr/CaloCellPedestalCorr','CaloCellNeighborsAverageCorr/CaloCellNeighborsAverageCorr','CaloCellContainerCheckerTool/CaloCellContainerCheckerTool']),

        print(cellMakerTools)

        cellAlgo=CompFactory.CaloCellMaker(CaloCellMakerToolNames = cellMakerTools,
                                           CaloCellsOutputName = self.CellsName)
        result.addEventAlgo(cellAlgo,primary=False)
        return result
        
    
    def StandardConfigurationPreClusterAlgorithmsConf(self, clustersname = None):
        doLCCalib = self.ConfigFlags.Calo.TopoCluster.doTopoClusterLocalCalib
        
        if clustersname is None:
            clustersname = "CaloCalTopoClusters" if doLCCalib else "CaloTopoClusters"
        
        if clustersname=="CaloTopoClusters" and doLCCalib is True: 
            raise RuntimeError("Inconistent arguments: Name must not be 'CaloTopoClusters' if doLCCalib is True")
        
        self.ClustersOutputName = clustersname
        
        result=ComponentAccumulator()
        
        if self.ConfigFlags.Input.Format is Format.BS:
            #Data-case: Schedule ByteStream reading for LAr & Tile
            from LArByteStream.LArRawDataReadingConfig import LArRawDataReadingCfg
            result.merge(LArRawDataReadingCfg(self.ConfigFlags))

            from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg

            result.merge(ByteStreamReadCfg(self.ConfigFlags,type_names=['TileDigitsContainer/TileDigitsCnt',
                                                                   'TileRawChannelContainer/TileRawChannelCnt',
                                                                   'TileMuonReceiverContainer/TileMuRcvCnt']))
            result.getService("ByteStreamCnvSvc").ROD2ROBmap=["-1"]
            if self.ConfigFlags.Output.doWriteESD:
                from TileRecAlgs.TileDigitsFilterConfig import TileDigitsFilterOutputCfg
                result.merge(TileDigitsFilterOutputCfg(self.ConfigFlags))
            else: #Mostly for wrapping in RecExCommon
                from TileRecAlgs.TileDigitsFilterConfig import TileDigitsFilterCfg
                result.merge(TileDigitsFilterCfg(self.ConfigFlags))

            from LArROD.LArRawChannelBuilderAlgConfig import LArRawChannelBuilderAlgCfg
            result.merge(LArRawChannelBuilderAlgCfg(self.ConfigFlags))

            from TileRecUtils.TileRawChannelMakerConfig import TileRawChannelMakerCfg
            result.merge(TileRawChannelMakerCfg(self.ConfigFlags))

        if not self.ConfigFlags.Input.isMC and not self.ConfigFlags.Common.isOnline:
            from LArCellRec.LArTimeVetoAlgConfig import LArTimeVetoAlgCfg
            result.merge(LArTimeVetoAlgCfg(self.ConfigFlags))

        if not self.ConfigFlags.Input.isMC and not self.ConfigFlags.Overlay.DataOverlay:
            from LArROD.LArFebErrorSummaryMakerConfig import LArFebErrorSummaryMakerCfg
            result.merge(LArFebErrorSummaryMakerCfg(self.ConfigFlags))
            
        if self.ConfigFlags.Input.Format is Format.BS or 'StreamRDO' in self.ConfigFlags.Input.ProcessingTags:
            result.merge(self.DefaultCaloCellMakerConf())

        from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
        
        # Schedule total noise cond alg
        result.merge(CaloNoiseCondAlgCfg(self.ConfigFlags,"totalNoise"))
        # Schedule electronic noise cond alg (needed for LC weights)
        result.merge(CaloNoiseCondAlgCfg(self.ConfigFlags,"electronicNoise"))
        return result
        
    def HybridClusterProcessorConf(self, clustersname=None):
        result = self.StandardConfigurationPreClusterAlgorithmsConf(clustersname)
        
        HybridClusterProcessor = CompFactory.CaloGPUHybridClusterProcessor("HybridClusterProcessor")
        HybridClusterProcessor.ClustersOutputName = self.ClustersOutputName
        HybridClusterProcessor.MeasureTimes = self.MeasureTimes
        HybridClusterProcessor.TimeFileOutput = "GlobalTimes.txt"
        HybridClusterProcessor.DeferConstantDataPreparationToFirstEvent = True
        
        HybridClusterProcessor.NumPreAllocatedDataHolders = self.NumPreAllocatedDataHolders
        
        from LArGeoAlgsNV.LArGMConfig import LArGMCfg
        from TileGeoModel.TileGMConfig import TileGMCfg
        
        result.merge(LArGMCfg(self.ConfigFlags))
        result.merge(TileGMCfg(self.ConfigFlags))
        
        ConstantDataExporter = result.popToolsAndMerge( self.BasicConstantDataExporterToolConf() )
        EventDataExporter = result.popToolsAndMerge( self.BasicEventDataExporterToolConf() )
        AthenaClusterImporter = result.popToolsAndMerge( self.BasicAthenaClusterImporterToolConf() )
        
        HybridClusterProcessor.ConstantDataToGPUTool = ConstantDataExporter
        HybridClusterProcessor.EventDataToGPUTool = EventDataExporter
        HybridClusterProcessor.GPUToEventDataTool = AthenaClusterImporter
        
        HybridClusterProcessor.BeforeGPUTools = []
       
        if self.TestGPUGrowing or self.TestGPUSplitting:
            DefaultClustering = result.popToolsAndMerge( self.DefaultTopologicalClusteringToolConf("DefaultGrowing") )
            
            HybridClusterProcessor.BeforeGPUTools += [DefaultClustering]
            
            if self.TestGPUGrowing:
                if self.OutputCountsToFile:
                    CPUCount1 = result.popToolsAndMerge( self.CellsCounterCPUToolConf("./counts", "DefaultGrowCounter", "default_grow") )
                    HybridClusterProcessor.BeforeGPUTools += [CPUCount1]
                if self.OutputClustersToFile:
                    CPUOut1 = result.popToolsAndMerge( self.CPUOutputToolConf("./val_default_grow", "DefaultGrowOutput") )
                    HybridClusterProcessor.BeforeGPUTools += [CPUOut1]
            
                
            if self.TestGPUSplitting:
                FirstSplitter = result.popToolsAndMerge( self.DefaultClusterSplittingToolConf("DefaultSplitting") )
                HybridClusterProcessor.BeforeGPUTools += [FirstSplitter]
                
                if self.OutputCountsToFile:
                    CPUCount2 = result.popToolsAndMerge( self.CellsCounterCPUToolConf("./counts", "DefaultGrowAndSplitCounter", "default_grow_split") )
                    HybridClusterProcessor.BeforeGPUTools += [CPUCount2]
                if self.OutputClustersToFile:
                    CPUOut2 = result.popToolsAndMerge( self.CPUOutputToolConf("./val_default_grow_split", "DefaultSplitOutput") )
                    HybridClusterProcessor.BeforeGPUTools += [CPUOut2]
            
            if self.TestGPUGrowing:
                Deleter = result.popToolsAndMerge( self.CaloClusterDeleterToolConf())
                HybridClusterProcessor.BeforeGPUTools += [Deleter]                
                if self.TestGPUSplitting:
                    SecondDefaultClustering = result.popToolsAndMerge( self.DefaultTopologicalClusteringToolConf("SecondDefaultGrowing") )
                    HybridClusterProcessor.BeforeGPUTools += [SecondDefaultClustering]
            
        HybridClusterProcessor.GPUTools = []
        
        if self.TestGPUSplitting:
            if not self.TestGPUGrowing:
                GPUClusterSplitting1 = result.popToolsAndMerge( self.GPUClusterSplitterToolConf("GPUSplitter"))
                HybridClusterProcessor.GPUTools += [GPUClusterSplitting1]
                PropCalc1 = result.popToolsAndMerge( self.ClusterInfoCalcToolConf("PropCalcPostSplitting"))
                HybridClusterProcessor.GPUTools += [PropCalc1]
            else:
                GPUClusterSplitting1 = result.popToolsAndMerge( self.GPUClusterSplitterToolConf("FirstGPUSplitter"))
                GPUClusterSplitting1.TimeFileOutput = "ignore.txt" #Not the most elegant solution, but...
                HybridClusterProcessor.GPUTools += [GPUClusterSplitting1]
                PropCalc1 = result.popToolsAndMerge( self.ClusterInfoCalcToolConf("PropCalc1"))
                PropCalc1.TimeFileOutput = "ignore.txt" #Not the most elegant solution, but...
                HybridClusterProcessor.GPUTools += [PropCalc1]
            
            if self.OutputCountsToFile:
                GPUCount1 = result.popToolsAndMerge( self.CellsCounterGPUToolConf("./counts", "DefaultGrowModifiedSplitCounter", "default_grow_modified_split") )
                HybridClusterProcessor.GPUTools += [GPUCount1]
            if self.OutputClustersToFile:
                GPUOut1 = result.popToolsAndMerge( self.GPUOutputToolConf("./val_default_grow_modified_split", "DefaultGrowModifiedSplitOutput") )
                HybridClusterProcessor.GPUTools += [GPUOut1]
            
        
        if self.TestGPUGrowing:
            TopoAutomatonClustering1 = result.popToolsAndMerge( self.TopoAutomatonClusteringToolConf("GPUGrowing"))
            HybridClusterProcessor.GPUTools += [TopoAutomatonClustering1]
            
            PropCalc2 = result.popToolsAndMerge( self.ClusterInfoCalcToolConf("PropCalcPostGrowing"))
            HybridClusterProcessor.GPUTools += [PropCalc2]
            if self.OutputCountsToFile:
                GPUCount2 = result.popToolsAndMerge( self.CellsCounterGPUToolConf("./counts", "ModifiedGrowCounter", "modified_grow") )
                HybridClusterProcessor.GPUTools += [GPUCount2]
            if self.OutputClustersToFile:
                GPUOut2 = result.popToolsAndMerge( self.GPUOutputToolConf("./val_modified_grow", "ModifiedGrowOutput") )
                HybridClusterProcessor.GPUTools += [GPUOut2]
        
        if self.TestGPUGrowing and self.TestGPUSplitting:
            GPUClusterSplitting2 = result.popToolsAndMerge( self.GPUClusterSplitterToolConf("GPUSplitter"))
            HybridClusterProcessor.GPUTools += [GPUClusterSplitting2]
            
            PropCalc3 = result.popToolsAndMerge( self.ClusterInfoCalcToolConf("PropCalcPostSplitting"))
            HybridClusterProcessor.GPUTools += [PropCalc3]
            
            if self.OutputCountsToFile:
                GPUCount3 = result.popToolsAndMerge( self.CellsCounterGPUToolConf("./counts", "ModifiedGrowSplitCounter", "modified_grow_split") )
                HybridClusterProcessor.GPUTools += [GPUCount3]
            if self.OutputClustersToFile:
                GPUOut3 = result.popToolsAndMerge( self.GPUOutputToolConf("./val_modified_grow_split", "ModifiedGrowSplitOutput") )
                HybridClusterProcessor.GPUTools += [GPUOut3]
            
            TopoAutomatonClustering2 = result.popToolsAndMerge( self.TopoAutomatonClusteringToolConf("SecondGPUGrowing"))
            TopoAutomatonClustering2.TimeFileOutput = "ignore.txt" #Not the most elegant solution, but...
            HybridClusterProcessor.GPUTools += [TopoAutomatonClustering2]
            
            PropCalc4 = result.popToolsAndMerge( self.ClusterInfoCalcToolConf("PropCalc4"))
            PropCalc4.TimeFileOutput = "ignore.txt" #Not the most elegant solution, but...
            HybridClusterProcessor.GPUTools += [PropCalc4]
            
        if not (self.TestGPUGrowing or self.TestGPUSplitting):
            TopoAutomatonClusteringDef = result.popToolsAndMerge( self.TopoAutomatonClusteringToolConf("TopoAutomatonClustering"))
            HybridClusterProcessor.GPUTools += [TopoAutomatonClusteringDef]
            
            GPUClusterSplittingDef = result.popToolsAndMerge( self.GPUClusterSplitterToolConf("GPUTopoSplitter"))
            HybridClusterProcessor.GPUTools += [GPUClusterSplittingDef]
            
            PropCalcDef = result.popToolsAndMerge( self.ClusterInfoCalcToolConf("GPUClusterPropertiesCalculator"))
            HybridClusterProcessor.GPUTools += [PropCalcDef]
            
            
        
        HybridClusterProcessor.AfterGPUTools = []
        
        if self.TestGPUGrowing:
        
            TopoSplitter = result.popToolsAndMerge( self.DefaultClusterSplittingToolConf() )
            HybridClusterProcessor.AfterGPUTools += [TopoSplitter]
            
            if self.TestGPUSplitting:
                if self.OutputCountsToFile:
                    CPUCount3 = result.popToolsAndMerge( self.CellsCounterCPUToolConf("./counts", "ModifiedGrowDefaultSplitCounter", "modified_grow_default_split") )
                    HybridClusterProcessor.AfterGPUTools += [CPUCount3]
                if self.OutputClustersToFile:
                    CPUOut3 = result.popToolsAndMerge( self.CPUOutputToolConf("./val_modified_grow_default_split", "ModifiedGrowDefaultSplitOutput") )
                    HybridClusterProcessor.AfterGPUTools += [CPUOut3]
                
        
        from CaloBadChannelTool.CaloBadChanToolConfig import CaloBadChanToolCfg
        caloBadChanTool = result.popToolsAndMerge( CaloBadChanToolCfg(self.ConfigFlags) )
        CaloClusterBadChannelList=CompFactory.CaloClusterBadChannelList
        BadChannelListCorr = CaloClusterBadChannelList(badChannelTool = caloBadChanTool)
        HybridClusterProcessor.AfterGPUTools += [BadChannelListCorr]

        from CaloRec.CaloTopoClusterConfig import getTopoMoments
        
        momentsMaker=result.popToolsAndMerge(getTopoMoments(self.ConfigFlags))
        HybridClusterProcessor.AfterGPUTools += [momentsMaker]
            
        if self.ConfigFlags.Calo.TopoCluster.doTopoClusterLocalCalib:    
            #Took out CaloClusterSnapshot that wanted to be a part of a CaloClusterMaker.
            #Possibly change in the future?
            from CaloRec.CaloTopoClusterConfig import getTopoClusterLocalCalibTools
            HybridClusterProcessor.AfterGPUTools += getTopoClusterLocalCalibTools(self.ConfigFlags)

            from CaloRec.CaloTopoClusterConfig import caloTopoCoolFolderCfg
            result.merge(caloTopoCoolFolderCfg(self.ConfigFlags))

        result.addEventAlgo(HybridClusterProcessor,primary=True)

        return result
        
    
    def PrepareTest(self, default_files):
    
        import argparse
        
        parser = argparse.ArgumentParser()
        
        parser.add_argument('-events','--numevents', default = 10)
        parser.add_argument('-threads','--numthreads', default = 1)
        parser.add_argument('-f','--files', action = 'extend', nargs = '*')
        parser.add_argument('-t','--measuretimes', action = 'store_true')
        parser.add_argument('-o','--outputclusters', action = 'store_true')
        parser.add_argument('-c','--outputcounts', action = 'store_true')
        
        args = parser.parse_args()
        
        from AthenaCommon.Configurable import Configurable
        Configurable.configurableRun3Behavior=1

        from AthenaConfiguration.AllConfigFlags import ConfigFlags
        #from AthenaConfiguration.TestDefaults import defaultTestFiles

        if args.files is None:
            ConfigFlags.Input.Files = default_files
        elif len(args.files) == 0:
            ConfigFlags.Input.Files = default_files
        elif len(args.files) == 1:
            if args.files[0] == 'default':
                ConfigFlags.Input.Files = default_files
            elif args.files[0] == 'ttbar':
                ConfigFlags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigInDetValidation/samples/mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.recon.RDO.e3698_s2608_s2183_r7195/RDO.06752780._000001.pool.root.1",
                                           "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigInDetValidation/samples/mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.recon.RDO.e3698_s2608_s2183_r7195/RDO.06752780._000002.pool.root.1",
                                           "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigInDetValidation/samples/mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.recon.RDO.e3698_s2608_s2183_r7195/RDO.06752780._000003.pool.root.1" ]
                
            elif args.files[0] == 'jets':
                ConfigFlags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigEgammaValidation/valid3.147917.Pythia8_AU2CT10_jetjet_JZ7W.recon.RDO.e3099_s2578_r6596_tid05293007_00/RDO.05293007._000001.pool.root.1"]
            else:
                ConfigFlags.Input.Files = args.files
        else:
            ConfigFlags.Input.Files = args.files
               
        #ConfigFlags.Input.Files = defaultTestFiles.RDO_RUN2
        ConfigFlags.Concurrency.NumThreads = int(args.numthreads)
        ConfigFlags.Concurrency.NumConcurrentEvents = int(args.numthreads)
        #This is to ensure the measurments are multi-threaded in the way we expect, I guess?
        ConfigFlags.lock()
        
        self.ConfigFlags = ConfigFlags

        from AthenaConfiguration.MainServicesConfig import MainServicesCfg
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

        cfg=MainServicesCfg(ConfigFlags)
        cfg.merge(PoolReadCfg(ConfigFlags))
        
        self.MeasureTimes = args.measuretimes
        self.OutputClustersToFile = args.outputclusters
        self.OutputCountsToFile = args.outputcounts
        
        #self.NumPreAllocatedDataHolders = int(args.numthreads)
        
        if 'StreamRDO' in self.ConfigFlags.Input.ProcessingTags:
            cfg.addEventAlgo(CompFactory.xAODMaker.EventInfoCnvAlg(),sequenceName="AthAlgSeq")
        
        return (cfg, int(args.numevents))

        


        