# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from CaloRecGPU.CaloRecGPUConfigurator import CaloRecGPUConfigurator
import CaloRecGPUTesting

#Run the GPU implementation plus any overheads,
#for multithreading studies.

def Configure(Configurator, clustersname = None):
        from AthenaConfiguration.ComponentFactory import CompFactory
        
        result = Configurator.StandardConfigurationPreClusterAlgorithmsConf(clustersname)
        
        HybridClusterProcessor = CompFactory.CaloGPUHybridClusterProcessor("HybridClusterProcessor")
        HybridClusterProcessor.ClustersOutputName = Configurator.ClustersOutputName
        HybridClusterProcessor.MeasureTimes = Configurator.MeasureTimes
        HybridClusterProcessor.TimeFileOutput = "GlobalTimes.txt"
        HybridClusterProcessor.DeferConstantDataPreparationToFirstEvent = True
        HybridClusterProcessor.DoPlots = False
        HybridClusterProcessor.PlotterTool = None
        HybridClusterProcessor.DoMonitoring = False
        HybridClusterProcessor.NumPreAllocatedDataHolders = Configurator.NumPreAllocatedDataHolders
        
        from LArGeoAlgsNV.LArGMConfig import LArGMCfg
        from TileGeoModel.TileGMConfig import TileGMCfg
        
        result.merge(LArGMCfg(Configurator.ConfigFlags))
        result.merge(TileGMCfg(Configurator.ConfigFlags))
        
        ConstantDataExporter = result.popToolsAndMerge( Configurator.BasicConstantDataExporterToolConf() )
        EventDataExporter = result.popToolsAndMerge( Configurator.BasicEventDataExporterToolConf() )
        AthenaClusterImporter = result.popToolsAndMerge( Configurator.AthenaClusterAndMomentsImporterToolConf() )
        
        HybridClusterProcessor.ConstantDataToGPUTool = ConstantDataExporter
        HybridClusterProcessor.EventDataToGPUTool = EventDataExporter
        HybridClusterProcessor.GPUToEventDataTool = AthenaClusterImporter
        
        HybridClusterProcessor.BeforeGPUTools = []
       
        HybridClusterProcessor.GPUTools = []
        
        TopoAutomatonClusteringDef = result.popToolsAndMerge( Configurator.TopoAutomatonClusteringToolConf("TopoAutomatonClustering"))
        HybridClusterProcessor.GPUTools += [TopoAutomatonClusteringDef]
        
        FirstPropCalcDef = result.popToolsAndMerge( Configurator.ClusterInfoCalcToolConf("PostGrowGPUClusterPropertiesCalculator", True))
        HybridClusterProcessor.GPUTools += [FirstPropCalcDef]
        
        GPUClusterSplittingDef = result.popToolsAndMerge( Configurator.TopoAutomatonSplitterToolConf("GPUSplitter") )
        HybridClusterProcessor.GPUTools += [GPUClusterSplittingDef]
        
        GPUMomentsDef = result.popToolsAndMerge( Configurator.GPUClusterMomentsCalculatorToolConf("GPUTopoMoments") )
        HybridClusterProcessor.GPUTools += [GPUMomentsDef]
            
        HybridClusterProcessor.AfterGPUTools = []
        
        result.addEventAlgo(HybridClusterProcessor,primary=True)

        return result


if __name__=="__main__":

    Configurator = CaloRecGPUConfigurator()
    
    cfg, numevents = CaloRecGPUTesting.PrepareTest(Configurator)

    theKey="CaloCalTopoClustersNew"
    
    topoAcc = Configure(Configurator)

    topoAlg = topoAcc.getPrimary()
    topoAlg.ClustersOutputName=theKey
    
    cfg.merge(topoAcc)
    
    cfg.run(numevents)

