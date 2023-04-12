# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from CaloRecGPU.CaloRecGPUConfigurator import SingleToolToPlot, ComparedToolsToPlot

from AthenaConfiguration.Enums import Format
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PrevAlgorithmsConfiguration(Configurator, clustersname = None):
    doLCCalib = Configurator.ConfigFlags.Calo.TopoCluster.doTopoClusterLocalCalib
    
    if clustersname is None:
        clustersname = "CaloCalTopoClusters" if doLCCalib else "CaloTopoClusters"
    
    if clustersname=="CaloTopoClusters" and doLCCalib is True: 
        raise RuntimeError("Inconsistent arguments: Name must not be 'CaloTopoClusters' if doLCCalib is True")
    
    Configurator.ClustersOutputName = clustersname
    
    result=ComponentAccumulator()
    
    if Configurator.ConfigFlags.Input.Format is Format.BS:
        #Data-case: Schedule ByteStream reading for LAr & Tile
        from LArByteStream.LArRawDataReadingConfig import LArRawDataReadingCfg
        result.merge(LArRawDataReadingCfg(Configurator.ConfigFlags))

        from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg

        result.merge(ByteStreamReadCfg(Configurator.ConfigFlags,type_names=['TileDigitsContainer/TileDigitsCnt',
                                                               'TileRawChannelContainer/TileRawChannelCnt',
                                                               'TileMuonReceiverContainer/TileMuRcvCnt']))
        result.getService("ByteStreamCnvSvc").ROD2ROBmap=["-1"]
        if Configurator.ConfigFlags.Output.doWriteESD:
            from TileRecAlgs.TileDigitsFilterConfig import TileDigitsFilterOutputCfg
            result.merge(TileDigitsFilterOutputCfg(Configurator.ConfigFlags))
        else: #Mostly for wrapping in RecExCommon
            from TileRecAlgs.TileDigitsFilterConfig import TileDigitsFilterCfg
            result.merge(TileDigitsFilterCfg(Configurator.ConfigFlags))

        from LArROD.LArRawChannelBuilderAlgConfig import LArRawChannelBuilderAlgCfg
        result.merge(LArRawChannelBuilderAlgCfg(Configurator.ConfigFlags))

        from TileRecUtils.TileRawChannelMakerConfig import TileRawChannelMakerCfg
        result.merge(TileRawChannelMakerCfg(Configurator.ConfigFlags))

    if not Configurator.ConfigFlags.Input.isMC and not Configurator.ConfigFlags.Common.isOnline:
        from LArCellRec.LArTimeVetoAlgConfig import LArTimeVetoAlgCfg
        result.merge(LArTimeVetoAlgCfg(Configurator.ConfigFlags))

    if not Configurator.ConfigFlags.Input.isMC and not Configurator.ConfigFlags.Overlay.DataOverlay:
        from LArROD.LArFebErrorSummaryMakerConfig import LArFebErrorSummaryMakerCfg
        result.merge(LArFebErrorSummaryMakerCfg(Configurator.ConfigFlags))
        
    if Configurator.ConfigFlags.Input.Format is Format.BS or 'StreamRDO' in Configurator.ConfigFlags.Input.ProcessingTags:
        result.merge(Configurator.DefaultCaloCellMakerConf())
    elif Configurator.FillMissingCells:
        from AthenaCommon.Logging import log
        log.warning("Asked to fill missing cells but will not run cell maker! Slow path might be taken!")

    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    
    # Schedule total noise cond alg
    result.merge(CaloNoiseCondAlgCfg(Configurator.ConfigFlags,"totalNoise"))
    # Schedule electronic noise cond alg (needed for LC weights)
    result.merge(CaloNoiseCondAlgCfg(Configurator.ConfigFlags,"electronicNoise"))
    
    if not Configurator.ConfigFlags.Common.isOnline:
        from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDbCfg
        result.merge(LArElecCalibDbCfg(Configurator.ConfigFlags,["HVScaleCorr"]))
    
    return result
    


#TestGrow, TestSplit, TestMoments are self-explanatory.
#If `DoCrossTests` is True, outputs and/or plots
#are done considering all possible combinations
#of (GPU, CPU) x (Growing, Splitting)
#PlotterConfigurator takes the CaloGPUClusterAndCellDataMonitor
#and adds the appropriate plots to it.
def FullTestConfiguration(Configurator, TestGrow=False, TestSplit=False, TestMoments=False, DoCrossTests=False,
                          PlotterConfigurator=None, clustersname=None, OutputCellInfo=False, SkipSyncs=True):
    doLCCalib = Configurator.ConfigFlags.Calo.TopoCluster.doTopoClusterLocalCalib
    
    if clustersname is None:
        clustersname = "CaloCalTopoClusters" if doLCCalib else "CaloTopoClusters"
    
    if clustersname=="CaloTopoClusters" and doLCCalib is True: 
        raise RuntimeError("Inconsistent arguments: Name must not be 'CaloTopoClusters' if doLCCalib is True")
    
    Configurator.ClustersOutputName = clustersname
    
    
    result= PrevAlgorithmsConfiguration(Configurator, clustersname)
        
    HybridClusterProcessor = CompFactory.CaloGPUHybridClusterProcessor("HybridClusterProcessor")
    HybridClusterProcessor.ClustersOutputName = Configurator.ClustersOutputName
    HybridClusterProcessor.MeasureTimes = Configurator.MeasureTimes
    HybridClusterProcessor.TimeFileOutput = "GlobalTimes.txt"
    HybridClusterProcessor.DeferConstantDataPreparationToFirstEvent = True
            
    if PlotterConfigurator is None:
        HybridClusterProcessor.DoPlots = False
    else:
        HybridClusterProcessor.DoPlots = True
        Plotter = PlotterConfigurator(result.popToolsAndMerge(Configurator.PlotterMonitoringToolConf()))
        HybridClusterProcessor.PlotterTool = Plotter
        if TestGrow and not DoCrossTests:
            Plotter.ToolsToPlot += [ SingleToolToPlot("DefaultGrowing", "CPU_growing") ]
            Plotter.ToolsToPlot += [ SingleToolToPlot("PropCalcPostGrowing", "GPU_growing") ]
            Plotter.PairsToPlot += [ ComparedToolsToPlot("DefaultGrowing", "PropCalcPostGrowing", "growing") ]
        if TestSplit and not DoCrossTests:
            Plotter.ToolsToPlot += [ SingleToolToPlot("DefaultSplitting", "CPU_splitting") ]
            Plotter.ToolsToPlot += [ SingleToolToPlot("PropCalcPostSplitting", "GPU_splitting") ]
            Plotter.PairsToPlot += [ ComparedToolsToPlot("DefaultSplitting", "PropCalcPostSplitting", "splitting") ]
        if TestMoments:
            Plotter.ToolsToPlot += [ SingleToolToPlot("CPUMoments", "CPU_moments") ]
            Plotter.ToolsToPlot += [ SingleToolToPlot("AthenaClusterImporter", "GPU_moments") ]
            Plotter.PairsToPlot += [ ComparedToolsToPlot("CPUMoments", "AthenaClusterImporter", "moments") ]
        if DoCrossTests:
            Plotter.ToolsToPlot += [ SingleToolToPlot("DefaultGrowing", "CPU_growing") ]
            Plotter.ToolsToPlot += [ SingleToolToPlot("PropCalcPostGrowing", "GPU_growing") ]
            Plotter.PairsToPlot += [ ComparedToolsToPlot("DefaultGrowing", "PropCalcPostGrowing", "growing") ]
            
            Plotter.ToolsToPlot += [ SingleToolToPlot("DefaultSplitting", "CPUCPU_splitting") ]
            Plotter.ToolsToPlot += [ SingleToolToPlot("PropCalcPostSplitting", "GPUGPU_splitting") ]
            Plotter.PairsToPlot += [ ComparedToolsToPlot("DefaultSplitting", "PropCalcPostSplitting", "CPU_to_GPUGPU_splitting") ]
            
            Plotter.ToolsToPlot += [ SingleToolToPlot("PropCalcDefaultGrowGPUSplit", "CPUGPU_splitting") ]
            Plotter.ToolsToPlot += [ SingleToolToPlot("DefaultPostGPUSplitting", "GPUCPU_splitting") ]
            Plotter.PairsToPlot += [ ComparedToolsToPlot("DefaultSplitting", "PropCalcDefaultGrowGPUSplit", "CPU_to_CPUGPU_splitting") ]
            Plotter.PairsToPlot += [ ComparedToolsToPlot("DefaultSplitting", "DefaultPostGPUSplitting", "CPU_to_GPUCPU_splitting") ]
            
        
    HybridClusterProcessor.DoMonitoring = Configurator.DoMonitoring
    
    if Configurator.DoMonitoring or PlotterConfigurator is not None:
        histSvc = CompFactory.THistSvc(Output = ["EXPERT DATAFILE='expert-monitoring.root', OPT='RECREATE'"])
        result.addService(histSvc)
    
    if Configurator.DoMonitoring:
        HybridClusterProcessor.MonitoringTool = Configurator.MonitorizationToolConf()
    
    HybridClusterProcessor.NumPreAllocatedDataHolders = Configurator.NumPreAllocatedDataHolders
    
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    from TileGeoModel.TileGMConfig import TileGMCfg
    
    result.merge(LArGMCfg(Configurator.ConfigFlags))
    result.merge(TileGMCfg(Configurator.ConfigFlags))
    
    ConstantDataExporter = result.popToolsAndMerge( Configurator.BasicConstantDataExporterToolConf() )
    EventDataExporter = result.popToolsAndMerge( Configurator.BasicEventDataExporterToolConf() )
    
    AthenaClusterImporter = None
    
    if TestMoments:
         AthenaClusterImporter = result.popToolsAndMerge( Configurator.AthenaClusterAndMomentsImporterToolConf("AthenaClusterImporter") )
    else:
         AthenaClusterImporter = result.popToolsAndMerge( Configurator.BasicAthenaClusterImporterToolConf("AthenaClusterImporter") )
        
    
    HybridClusterProcessor.ConstantDataToGPUTool = ConstantDataExporter
    HybridClusterProcessor.EventDataToGPUTool = EventDataExporter
    HybridClusterProcessor.GPUToEventDataTool = AthenaClusterImporter
    
    HybridClusterProcessor.BeforeGPUTools = []
           
    if TestGrow or TestSplit or TestMoments:
        DefaultClustering = result.popToolsAndMerge( Configurator.DefaultTopologicalClusteringToolConf("DefaultGrowing") )
        
        HybridClusterProcessor.BeforeGPUTools += [DefaultClustering]
                    
        if TestGrow and (not TestSplit or DoCrossTests):
            if Configurator.OutputCountsToFile:
                CPUCount1 = result.popToolsAndMerge( Configurator.CellsCounterCPUToolConf("./counts", "DefaultGrowCounter", "default_grow") )
                HybridClusterProcessor.BeforeGPUTools += [CPUCount1]
            if Configurator.OutputClustersToFile:
                CPUOut1 = result.popToolsAndMerge( Configurator.CPUOutputToolConf("./out_default_grow", "DefaultGrowOutput") )
                HybridClusterProcessor.BeforeGPUTools += [CPUOut1]
        
            
        if TestSplit or TestMoments:
            FirstSplitter = result.popToolsAndMerge( Configurator.DefaultClusterSplittingToolConf("DefaultSplitting") )
            HybridClusterProcessor.BeforeGPUTools += [FirstSplitter]
            
            if Configurator.OutputCountsToFile:
                CPUCount2 = result.popToolsAndMerge( Configurator.CellsCounterCPUToolConf("./counts", "DefaultGrowAndSplitCounter", "default_grow_split") )
                HybridClusterProcessor.BeforeGPUTools += [CPUCount2]
            if Configurator.OutputClustersToFile:
                CPUOut2 = result.popToolsAndMerge( Configurator.CPUOutputToolConf("./out_default_grow_split", "DefaultSplitOutput") )
                HybridClusterProcessor.BeforeGPUTools += [CPUOut2]
        
        if TestMoments:
            CPUMoments = result.popToolsAndMerge( Configurator.DefaultClusterMomentsCalculatorToolConf("CPUMoments") )
            HybridClusterProcessor.BeforeGPUTools += [CPUMoments]
            if Configurator.OutputCountsToFile:
                CPUDumper = result.popToolsAndMerge( Configurator.MomentsDumperToolConf("./moments", "CPUMomentsDumper", "CPU") )
                HybridClusterProcessor.BeforeGPUTools += [CPUDumper]
        
        Deleter = result.popToolsAndMerge( Configurator.CaloClusterDeleterToolConf())
        HybridClusterProcessor.BeforeGPUTools += [Deleter]                
        
        if TestSplit and (not TestGrow or DoCrossTests):
            SecondDefaultClustering = result.popToolsAndMerge( Configurator.DefaultTopologicalClusteringToolConf("SecondDefaultGrowing") )
            HybridClusterProcessor.BeforeGPUTools += [SecondDefaultClustering]
        
    HybridClusterProcessor.GPUTools = []
    
    if OutputCellInfo:
        CellOut = result.popToolsAndMerge( Configurator.GPUOutputToolConf("./out_cell", "CellInfoOutput", OnlyOutputCells = True) )
        HybridClusterProcessor.GPUTools += [CellOut]
        
    if TestSplit:
        if not TestGrow:
            GPUClusterSplitting1 = result.popToolsAndMerge( Configurator.TopoAutomatonSplitterToolConf("GPUSplitter") )
            if SkipSyncs:
                GPUClusterSplitting1.MeasureTimes = False
            HybridClusterProcessor.GPUTools += [GPUClusterSplitting1]
            PropCalc1 = result.popToolsAndMerge( Configurator.ClusterInfoCalcToolConf("PropCalcPostSplitting", False) )
            if SkipSyncs:
                PropCalc1.MeasureTimes = False
            HybridClusterProcessor.GPUTools += [PropCalc1]
        elif DoCrossTests:
            GPUClusterSplitting1 = result.popToolsAndMerge( Configurator.TopoAutomatonSplitterToolConf("FirstGPUSplitter") )
            GPUClusterSplitting1.TimeFileOutput = "" #This means there's no output.
            GPUClusterSplitting1.MeasureTimes = False
            HybridClusterProcessor.GPUTools += [GPUClusterSplitting1]
            PropCalc1 = result.popToolsAndMerge( Configurator.ClusterInfoCalcToolConf("PropCalcDefaultGrowGPUSplit", False) )
            PropCalc1.TimeFileOutput = "" #This means there's no output.
            PropCalc1.MeasureTimes = False
            HybridClusterProcessor.GPUTools += [PropCalc1]
        
        if ((not TestGrow) or DoCrossTests):
            if Configurator.OutputCountsToFile:
                GPUCount1 = result.popToolsAndMerge( Configurator.CellsCounterGPUToolConf("./counts", "DefaultGrowModifiedSplitCounter", "default_grow_modified_split") )
                HybridClusterProcessor.GPUTools += [GPUCount1]
            if Configurator.OutputClustersToFile:
                GPUOut1 = result.popToolsAndMerge( Configurator.GPUOutputToolConf("./out_default_grow_modified_split", "DefaultGrowModifiedSplitOutput") )
                HybridClusterProcessor.GPUTools += [GPUOut1]
        
    if TestGrow:
        TopoAutomatonClustering1 = result.popToolsAndMerge( Configurator.TopoAutomatonClusteringToolConf("GPUGrowing") )
        if SkipSyncs:
            TopoAutomatonClustering1.MeasureTimes = False
        HybridClusterProcessor.GPUTools += [TopoAutomatonClustering1]
        
        PropCalc2 = result.popToolsAndMerge( Configurator.ClusterInfoCalcToolConf("PropCalcPostGrowing", True))
        if SkipSyncs:
            PropCalc2.MeasureTimes = False
        HybridClusterProcessor.GPUTools += [PropCalc2]
        if ((not TestSplit) or DoCrossTests):
            if Configurator.OutputCountsToFile:
                GPUCount2 = result.popToolsAndMerge( Configurator.CellsCounterGPUToolConf("./counts", "ModifiedGrowCounter", "modified_grow") )
                HybridClusterProcessor.GPUTools += [GPUCount2]
            if Configurator.OutputClustersToFile:
                GPUOut2 = result.popToolsAndMerge( Configurator.GPUOutputToolConf("./out_modified_grow", "ModifiedGrowOutput") )
                HybridClusterProcessor.GPUTools += [GPUOut2]
    
    if TestGrow and TestSplit:
        GPUClusterSplitting2 = result.popToolsAndMerge( Configurator.TopoAutomatonSplitterToolConf("GPUSplitter") )
        if SkipSyncs:
            GPUClusterSplitting2.MeasureTimes = False
        HybridClusterProcessor.GPUTools += [GPUClusterSplitting2]
        
        if HybridClusterProcessor.DoPlots or not TestMoments:
          PropCalc3 = result.popToolsAndMerge( Configurator.ClusterInfoCalcToolConf("PropCalcPostSplitting", False) )
          if SkipSyncs:
              PropCalc3.MeasureTimes = False
          HybridClusterProcessor.GPUTools += [PropCalc3]
        
        if Configurator.OutputCountsToFile:
            GPUCount3 = result.popToolsAndMerge( Configurator.CellsCounterGPUToolConf("./counts", "ModifiedGrowSplitCounter", "modified_grow_split") )
            HybridClusterProcessor.GPUTools += [GPUCount3]
        if Configurator.OutputClustersToFile:
            GPUOut3 = result.popToolsAndMerge( Configurator.GPUOutputToolConf("./out_modified_grow_split", "ModifiedGrowSplitOutput") )
            HybridClusterProcessor.GPUTools += [GPUOut3]
        
        if DoCrossTests:
            TopoAutomatonClustering2 = result.popToolsAndMerge( Configurator.TopoAutomatonClusteringToolConf("SecondGPUGrowing") )
            TopoAutomatonClustering2.MeasureTimes = False
            TopoAutomatonClustering2.TimeFileOutput = "" #This means there's no output.
            HybridClusterProcessor.GPUTools += [TopoAutomatonClustering2]
            
            PropCalc4 = result.popToolsAndMerge( Configurator.ClusterInfoCalcToolConf("PropCalc4", True) )
            PropCalc4.TimeFileOutput = "" #This means there's no output.
            PropCalc4.MeasureTimes = False
            HybridClusterProcessor.GPUTools += [PropCalc4]
        
    if not (TestGrow or TestSplit):
        TopoAutomatonClusteringDef = result.popToolsAndMerge( Configurator.TopoAutomatonClusteringToolConf("TopoAutomatonClustering") )
        if SkipSyncs:
            TopoAutomatonClusteringDef.MeasureTimes = False
        HybridClusterProcessor.GPUTools += [TopoAutomatonClusteringDef]
        
        FirstPropCalcDef = result.popToolsAndMerge( Configurator.ClusterInfoCalcToolConf("PropCalcPostGrowing", True) )
        if SkipSyncs:
            FirstPropCalcDef.MeasureTimes = False
        HybridClusterProcessor.GPUTools += [FirstPropCalcDef]
        
        GPUClusterSplittingDef = result.popToolsAndMerge( Configurator.TopoAutomatonSplitterToolConf("GPUTopoSplitter") )
        if SkipSyncs:
            GPUClusterSplittingDef.MeasureTimes = False
        HybridClusterProcessor.GPUTools += [GPUClusterSplittingDef]
        
        if not TestMoments:
            SecondPropCalcDef = result.popToolsAndMerge( Configurator.ClusterInfoCalcToolConf("PropCalcPostSplitting", False) )
            if SkipSyncs:
                SecondPropCalcDef.MeasureTimes = False
            HybridClusterProcessor.GPUTools += [SecondPropCalcDef]
    
    if TestMoments and not DoCrossTests:
        if TestGrow and not TestSplit:
            GPUClusterSplittingDef = result.popToolsAndMerge( Configurator.TopoAutomatonSplitterToolConf("GPUTopoSplitter") )
            if SkipSyncs:
                GPUClusterSplittingDef.MeasureTimes = False
            HybridClusterProcessor.GPUTools += [GPUClusterSplittingDef]
            
        GPUMomentsDef = result.popToolsAndMerge( Configurator.GPUClusterMomentsCalculatorToolConf("GPUTopoMoments") )
        if SkipSyncs:
            GPUMomentsDef.MeasureTimes = False
        HybridClusterProcessor.GPUTools += [GPUMomentsDef]
        
    HybridClusterProcessor.AfterGPUTools = []
    
    if TestMoments and Configurator.OutputCountsToFile:
        GPUDumper = result.popToolsAndMerge( Configurator.MomentsDumperToolConf("./moments", "GPUMomentsDumper", "GPU") )
        HybridClusterProcessor.AfterGPUTools += [GPUDumper]
        
    if TestGrow and (not TestSplit or DoCrossTests):
    
        TopoSplitter = result.popToolsAndMerge( Configurator.DefaultClusterSplittingToolConf("DefaultPostGPUSplitting") )
        HybridClusterProcessor.AfterGPUTools += [TopoSplitter]
        
        if DoCrossTests:
            if Configurator.OutputCountsToFile:
                CPUCount3 = result.popToolsAndMerge( Configurator.CellsCounterCPUToolConf("./counts", "ModifiedGrowDefaultSplitCounter", "modified_grow_default_split") )
                HybridClusterProcessor.AfterGPUTools += [CPUCount3]
            if Configurator.OutputClustersToFile:
                CPUOut3 = result.popToolsAndMerge( Configurator.CPUOutputToolConf("./out_modified_grow_default_split", "ModifiedGrowDefaultSplitOutput") )
                HybridClusterProcessor.AfterGPUTools += [CPUOut3]
            
    
    from CaloBadChannelTool.CaloBadChanToolConfig import CaloBadChanToolCfg
    caloBadChanTool = result.popToolsAndMerge( CaloBadChanToolCfg(Configurator.ConfigFlags) )
    CaloClusterBadChannelList=CompFactory.CaloClusterBadChannelList
    BadChannelListCorr = CaloClusterBadChannelList(badChannelTool = caloBadChanTool)
    HybridClusterProcessor.AfterGPUTools += [BadChannelListCorr]

    result.addEventAlgo(HybridClusterProcessor,primary=True)

    return result
        
    
def PrepareTest(Configurator,
                default_files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/mc20e_13TeV/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.ESD.e4993_s3227_r12689/myESD.pool.root"],
                parse_command_arguments = True,
                allocate_as_many_as_threads = True):

    import argparse
    
    args = None
    rest = None
    
    if parse_command_arguments:
        parser = argparse.ArgumentParser()
        
        parser.add_argument('-events','--numevents', type=int, default = 10)
        parser.add_argument('-threads','--numthreads', type=int, default = 1)
        parser.add_argument('-f','--files', action = 'extend', nargs = '*')
        parser.add_argument('-t','--measuretimes', action = 'store_true')
        parser.add_argument('-o','--outputclusters', action = 'store_true')
        parser.add_argument('-c','--outputcounts', action = 'store_true')
        parser.add_argument('-nfc','--notfillcells', action = 'store_true')
        
        parser.add_argument('-m','--perfmon', action = 'store_true')
        parser.add_argument('-fm','--fullmon', action = 'store_true')
                   
        
        (args, pre_rest) = parser.parse_known_args()
                    
        if pre_rest is None or len(pre_rest) == 0:
            rest = ['--threads', '1']
            #Crude workaround for a condition within ConfigFlags.fillFromArgs
            #that would make it inspect the whole command-line when provided
            #with an empty list of arguments.
            #(I'd personally suggest changing the "listOfArgs or sys.argv[1:]"
            # used there since empty arrays are falsy while being non-null,
            # to a "sys.argv[1:] if listofArgs is None else listofArgs",
            # but I'm not about to suggest potentially code-breaking behaviour changes
            # when I can work around them easily, even if rather inelegantly...)
        else:
            rest = pre_rest
    
    from AthenaCommon.Configurable import Configurable
    Configurable.configurableRun3Behavior=1

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    #from AthenaConfiguration.TestDefaults import defaultTestFiles
                    
    Configurator.ConfigFlags = initConfigFlags() #ConfigFlags.clone()
    
    if parse_command_arguments:
        Configurator.ConfigFlags.fillFromArgs(listOfArgs=rest)
        #We could instead use our parser to overload here and so on, but...
    
    if args is None or args.files is None:
        Configurator.ConfigFlags.Input.Files = default_files
    elif len(args.files) == 0:
        Configurator.ConfigFlags.Input.Files = default_files
    elif len(args.files) == 1:
        if args.files[0] == 'default':
            Configurator.ConfigFlags.Input.Files = default_files
        elif args.files[0] == 'ttbar':
            Configurator.ConfigFlags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigInDetValidation/samples/mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.recon.RDO.e3698_s2608_s2183_r7195/RDO.06752780._000001.pool.root.1",
                                       "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigInDetValidation/samples/mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.recon.RDO.e3698_s2608_s2183_r7195/RDO.06752780._000002.pool.root.1",
                                       "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigInDetValidation/samples/mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.recon.RDO.e3698_s2608_s2183_r7195/RDO.06752780._000003.pool.root.1" ]
            
        elif args.files[0] == 'jets':
            Configurator.ConfigFlags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigEgammaValidation/valid3.147917.Pythia8_AU2CT10_jetjet_JZ7W.recon.RDO.e3099_s2578_r6596_tid05293007_00/RDO.05293007._000001.pool.root.1"]
        else:
            Configurator.ConfigFlags.Input.Files = args.files
    else:
        Configurator.ConfigFlags.Input.Files = args.files
           
    #Configurator.ConfigFlags.Input.Files = defaultTestFiles.RDO_RUN2
            
    if parse_command_arguments:
        Configurator.ConfigFlags.Concurrency.NumThreads = int(args.numthreads)
        Configurator.ConfigFlags.Concurrency.NumConcurrentEvents = int(args.numthreads)
        #This is to ensure the measurments are multi-threaded in the way we expect, I guess?
        Configurator.ConfigFlags.PerfMon.doFastMonMT = args.perfmon
        Configurator.ConfigFlags.PerfMon.doFullMonMT = args.fullmon
            
    
    Configurator.ConfigFlags.lock()
    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    cfg=MainServicesCfg(Configurator.ConfigFlags)
    cfg.merge(PoolReadCfg(Configurator.ConfigFlags))
    
    if parse_command_arguments:
        if args.perfmon or args.fullmon:
             from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
             cfg.merge(PerfMonMTSvcCfg(Configurator.ConfigFlags))
            
    if parse_command_arguments:
        Configurator.MeasureTimes = args.measuretimes
        Configurator.OutputClustersToFile = args.outputclusters
        Configurator.OutputCountsToFile = args.outputcounts
        Configurator.FillMissingCells = not args.notfillcells
        if allocate_as_many_as_threads:
            Configurator.NumPreAllocatedDataHolders = int(args.numthreads)
    
    if 'StreamRDO' in Configurator.ConfigFlags.Input.ProcessingTags:
        cfg.addEventAlgo(CompFactory.xAODMaker.EventInfoCnvAlg(),sequenceName="AthAlgSeq")
    
    if parse_command_arguments:
        return (cfg, int(args.numevents))
    else:
        return (cfg, 10)

#For pretty printing things in axes when it comes to moments:
#<MOMENT_NAME>: <PLOT TITLE> <AXIS TITLE> <UNITS>
name_to_moment_map =  {
    "time"                        :  ("time",                "time",                ""),
    "FIRST_PHI"                   :  ("firstPhi",            "firstPhi",            ""),
    "FIRST_ETA"                   :  ("firstEta",            "firstEta",            ""),
    "SECOND_R"                    :  ("secondR",             "secondR",             ""),
    "SECOND_LAMBDA"               :  ("secondLambda",        "secondLambda",        ""),
    "DELTA_PHI"                   :  ("deltaPhi",            "deltaPhi",            ""),
    "DELTA_THETA"                 :  ("deltaTheta",          "deltaTheta",          ""),
    "DELTA_ALPHA"                 :  ("deltaAlpha",          "deltaAlpha",          ""),
    "CENTER_X"                    :  ("centerX",             "centerX",             ""),
    "CENTER_Y"                    :  ("centerY",             "centerY",             ""),
    "CENTER_Z"                    :  ("centerZ",             "centerZ",             ""),
    "CENTER_MAG"                  :  ("centerMag",           "centerMag",           ""),
    "CENTER_LAMBDA"               :  ("centerLambda",        "centerLambda",        ""),
    "LATERAL"                     :  ("lateral",             "lateral",             ""),
    "LONGITUDINAL"                :  ("longitudinal",        "longitudinal",        ""),
    "ENG_FRAC_EM"                 :  ("engFracEM",           "engFracEM",           ""),
    "ENG_FRAC_MAX"                :  ("engFracMax",          "engFracMax",          ""),
    "ENG_FRAC_CORE"               :  ("engFracCore",         "engFracCore",         ""),
    "FIRST_ENG_DENS"              :  ("firstEngDens",        "firstEngDens",        ""),
    "SECOND_ENG_DENS"             :  ("secondEngDens",       "secondEngDens",       ""),
    "ISOLATION"                   :  ("isolation",           "isolation",           ""),
    "ENG_BAD_CELLS"               :  ("engBadCells",         "engBadCells",         ""),
    "N_BAD_CELLS"                 :  ("nBadCells",           "nBadCells",           ""),
    "N_BAD_CELLS_CORR"            :  ("nBadCellsCorr",       "nBadCellsCorr",       ""),
    "BAD_CELLS_CORR_E"            :  ("badCellsCorrE",       "badCellsCorrE",       ""),
    "BADLARQ_FRAC"                :  ("badLArQFrac",         "badLArQFrac",         ""),
    "ENG_POS"                     :  ("engPos",              "engPos",              ""),
    "SIGNIFICANCE"                :  ("significance",        "significance",        ""),
    "CELL_SIGNIFICANCE"           :  ("cellSignificance",    "cellSignificance",    ""),
    "CELL_SIG_SAMPLING"           :  ("cellSigSampling",     "cellSigSampling",     ""),
    "AVG_LAR_Q"                   :  ("avgLArQ",             "avgLArQ",             ""),
    "AVG_TILE_Q"                  :  ("avgTileQ",            "avgTileQ",            ""),
    "ENG_BAD_HV_CELLS"            :  ("engBadHVCells",       "engBadHVCells",       ""),
    "N_BAD_HV_CELLS"              :  ("nBadHVCells",         "nBadHVCells",         ""),
    "PTD"                         :  ("PTD",                 "PTD",                 ""),
    "MASS"                        :  ("mass",                "mass",                ""),
    "EM_PROBABILITY"              :  ("EMProbability",       "EMProbability",       ""),
    "HAD_WEIGHT"                  :  ("hadWeight",           "hadWeight",           ""),
    "OOC_WEIGHT"                  :  ("OOCweight",           "OOCweight",           ""),
    "DM_WEIGHT"                   :  ("DMweight",            "DMweight",            ""),
    "TILE_CONFIDENCE_LEVEL"       :  ("tileConfidenceLevel", "tileConfidenceLevel", ""),
    "SECOND_TIME"                 :  ("secondTime",          "secondTime",          ""),
    "number_of_cells"             :  ("numCells",            "numCells",            ""),
    "VERTEX_FRACTION"             :  ("vertexFraction",      "vertexFraction",      ""),
    "NVERTEX_FRACTION"            :  ("nVertexFraction",     "nVertexFraction",     ""),
    "ETACALOFRAME"                :  ("etaCaloFrame",        "etaCaloFrame",        ""),
    "PHICALOFRAME"                :  ("phiCaloFrame",        "phiCaloFrame",        ""),
    "ETA1CALOFRAME"               :  ("eta1CaloFrame",       "eta1CaloFrame",       ""),
    "PHI1CALOFRAME"               :  ("phi1CaloFrame",       "phi1CaloFrame",       ""),
    "ETA2CALOFRAME"               :  ("eta2CaloFrame",       "eta2CaloFrame",       ""),
    "PHI2CALOFRAME"               :  ("phi2CaloFrame",       "phi2CaloFrame",       ""),
    "ENG_CALIB_TOT"               :  ("engCalibTot",         "engCalibTot",         ""),
    "ENG_CALIB_OUT_L"             :  ("engCalibOutL",        "engCalibOutL",        ""),
    "ENG_CALIB_OUT_M"             :  ("engCalibOutM",        "engCalibOutM",        ""),
    "ENG_CALIB_OUT_T"             :  ("engCalibOutT",        "engCalibOutT",        ""),
    "ENG_CALIB_DEAD_L"            :  ("engCalibDeadL",       "engCalibDeadL",       ""),
    "ENG_CALIB_DEAD_M"            :  ("engCalibDeadM",       "engCalibDeadM",       ""),
    "ENG_CALIB_DEAD_T"            :  ("engCalibDeadT",       "engCalibDeadT",       ""),
    "ENG_CALIB_EMB0"              :  ("engCalibEMB0",        "engCalibEMB0",        ""),
    "ENG_CALIB_EME0"              :  ("engCalibEME0",        "engCalibEME0",        ""),
    "ENG_CALIB_TILEG3"            :  ("engCalibTileG3",      "engCalibTileG3",      ""),
    "ENG_CALIB_DEAD_TOT"          :  ("engCalibDeadTot",     "engCalibDeadTot",     ""),
    "ENG_CALIB_DEAD_EMB0"         :  ("engCalibDeadEMB0",    "engCalibDeadEMB0",    ""),
    "ENG_CALIB_DEAD_TILE0"        :  ("engCalibDeadTile0",   "engCalibDeadTile0",   ""),
    "ENG_CALIB_DEAD_TILEG3"       :  ("engCalibDeadTileG3",  "engCalibDeadTileG3",  ""),
    "ENG_CALIB_DEAD_EME0"         :  ("engCalibDeadEME0",    "engCalibDeadEME0",    ""),
    "ENG_CALIB_DEAD_HEC0"         :  ("engCalibDeadHEC0",    "engCalibDeadHEC0",    ""),
    "ENG_CALIB_DEAD_FCAL"         :  ("engCalibDeadFCAL",    "engCalibDeadFCAL",    ""),
    "ENG_CALIB_DEAD_LEAKAGE"      :  ("engCalibDeadLeakage", "engCalibDeadLeakage", ""),
    "ENG_CALIB_DEAD_UNCLASS"      :  ("engCalibDeadUnclass", "engCalibDeadUnclass", ""),
    "ENG_CALIB_FRAC_EM"           :  ("engCalibFracEM",      "engCalibFracEM",      ""),
    "ENG_CALIB_FRAC_HAD"          :  ("engCalibFracHad",     "engCalibFracHad",     ""),
    "ENG_CALIB_FRAC_REST"         :  ("engCalibFracRest"     "engCalibFracRest"     "")
}

class PlotterConfigurator:
    #DoCells currently changes nothing
    #(originally was intended to show
    #that we had cells with the same energy),
    #keeping it here to remain part of the interface
    #if/when we port the cell maker as well.
    def __init__ (self, StepsToPlot = [], PairsToPlot = [], DoStandard = True, DoMoments = False, DoCells = False):
        self.PlotsToDo = []
        if DoStandard:
            for step in StepsToPlot:
                self.PlotsToDo += [
                                    ( (step + "_cluster_E",),
                                      {'type': 'TH1F', 
                                       'title': "Cluster Energy; E [MeV]; Number of Events",
                                       'xbins':  63,
                                       'xmin':  -5020,
                                       'xmax':   5020,
                                       'path': "EXPERT"}
                                    ),
                                    ( (step + "_cluster_Et",),
                                      {'type': 'TH1F', 
                                       'title': "Cluster Transverse Energy; E_T [MeV]; Number of Events",
                                       'xbins':  63,
                                       'xmin':  -5020,
                                       'xmax':   5020,
                                       'path': "EXPERT"}
                                    ),
                                    ( (step + "_cluster_eta",),
                                      {'type': 'TH1F', 
                                       'title': "Cluster #eta; #eta; Number of Events",
                                       'xbins':  84,
                                       'xmin':  -10.5,
                                       'xmax':   10.5,
                                       'path': "EXPERT"}
                                    ),
                                    ( (step + "_cluster_phi",),
                                      {'type': 'TH1F', 
                                       'title': "Cluster #phi; #phi; Number of Events",
                                       'xbins':  61,
                                       'xmin':  -3.25,
                                       'xmax':   3.25,
                                       'path': "EXPERT"}
                                    ),
                                    ( (step + "_cluster_eta," + step + "_cluster_phi",),
                                      {'type': 'TH2F', 
                                       'title': "Cluster #eta versus #phi; #eta; #phi",
                                       'xbins':  84,
                                       'xmin':  -10.5,
                                       'xmax':   10.5,
                                       'ybins':  61,
                                       'ymin':  -3.25,
                                       'ymax':   3.25,
                                       'path': "EXPERT"}
                                    )
                                  ]
            for pair in PairsToPlot:
                self.PlotsToDo += [
                                    ( (pair + "_num_unmatched_clusters",),
                                      {'type': 'TH1F', 
                                       'title': "Number of Unmatched Clusters; # of Unmatched Clusters; Number of Events",
                                       'xbins':  21,
                                       'xmin':  -0.5,
                                       'xmax':   20.5,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_E_ref," + pair + "_cluster_E_test",),
                                      {'type': 'TH2F', 
                                       'title': "Cluster Energy Comparison; E^{(CPU)} [MeV]; E^{(GPU)} [MeV]",
                                       'xbins':  63,
                                       'xmin':  -50.5,
                                       'xmax':   50.5,
                                       'ybins':  63,
                                       'ymin':  -50.5,
                                       'ymax':   50.5,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_E_ref," + pair + "_cluster_delta_E_rel_ref",),
                                      {'type': 'TH2F', 
                                       'title': "Cluster Energy Resolution; E^{(CPU)} [MeV]; #Delta E / #(){E^{(CPU)}}",
                                       'xbins':  63,
                                       'xmin':  -50.5,
                                       'xmax':   50.5,
                                       'ybins':  63,
                                       'ymin':  -0.025,
                                       'ymax':   0.025,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_Et_ref," + pair + "_cluster_Et_test",),
                                      {'type': 'TH2F', 
                                       'title': "Cluster Transverse Energy Comparison; E_T^{(CPU)} [MeV]; E_T^{(GPU)} [MeV]",
                                       'xbins':  63,
                                       'xmin':  -50.5,
                                       'xmax':   50.5,
                                       'ybins':  63,
                                       'ymin':  -50.5,
                                       'ymax':   50.5,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_Et_ref," + pair + "_cluster_delta_Et_rel_ref",),
                                      {'type': 'TH2F', 
                                       'title': "Cluster Transverse Energy Resolution; E_T^{(CPU)} [MeV]; #Delta E_T / #(){E_T^{(CPU)}}",
                                       'xbins':  63,
                                       'xmin':  -50.5,
                                       'xmax':   50.5,
                                       'ybins':  63,
                                       'ymin':  -0.025,
                                       'ymax':   0.025,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_eta_ref," + pair + "_cluster_eta_test",),
                                      {'type': 'TH2F', 
                                       'title': "Cluster #eta Comparison; #eta^{(CPU)}; #eta^{(GPU)}",
                                       'xbins':  63,
                                       'xmin':  -10.5,
                                       'xmax':   10.5,
                                       'ybins':  63,
                                       'ymin':  -10.5,
                                       'ymax':   10.5,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_eta_ref," + pair + "_cluster_delta_eta_rel_ref",),
                                      {'type': 'TH2F', 
                                       'title': "Cluster #eta Resolution; #eta^{(CPU)}; #Delta #eta / #(){#eta^{(CPU)}}",
                                       'xbins':  63,
                                       'xmin':  -10.5,
                                       'xmax':   10.5,
                                       'ybins':  63,
                                       'ymin':  -0.025,
                                       'ymax':   0.025,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_phi_ref," + pair + "_cluster_phi_test",),
                                      {'type': 'TH2F', 
                                       'title': "Cluster #phi Comparison; #phi^{(CPU)}; #phi^{(GPU)}",
                                       'xbins':  63,
                                       'xmin':  -3.3,
                                       'xmax':   3.3,
                                       'ybins':  63,
                                       'ymin':  -3.3,
                                       'ymax':   3.3,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_phi_ref," + pair + "_cluster_delta_phi_in_range",),
                                      {'type': 'TH2F', 
                                       'title': "Cluster #phi Resolution; #phi^{(CPU)}; #Delta #phi",
                                       'xbins':  63,
                                       'xmin':  -10.5,
                                       'xmax':   10.5,
                                       'ybins':  63,
                                       'ymin':  -0.025,
                                       'ymax':   0.025,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_delta_phi_in_range",),
                                      {'type': 'TH1F', 
                                       'title': "Cluster #phi; #phi; Number of Clusters",
                                       'xbins':  61,
                                       'xmin':  -3.25,
                                       'xmax':   3.25,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cell_secondary_weight_ref," + pair + "_cell_secondary_weight_test",),
                                      {'type': 'TH2F', 
                                       'title': "Shared Cell Secondary Weight Comparison; w_2^{(CPU)}; w_2^{(GPU)}",
                                       'xbins': 51,
                                       'xmin':  -0.05,
                                       'xmax':  0.505,
                                       'ybins': 51,
                                       'ymin':  -0.05,
                                       'ymax':  0.505,
                                       'path': "EXPERT"}
                                    )
                                  ]
        if DoMoments:
            for pair in PairsToPlot:
                for mom, prettynames in name_to_moment_map:
                    self.PlotsToDo += [
                                        ( (pair + "_cluster_moments_delta_" + mom + "_rel_ref;" + pair + "_cluster_moments_delta_" + mom + "_rel_ref_zoom_0",),
                                          {'type': 'TH1F', 
                                           'title': prettynames[0] + ";" + prettynames[1] + prettynames[2] + "; #Delta " + prettynames[1],
                                           'xbins': 51,
                                           'xmin':  -1,
                                           'xmax':  1,
                                           'path': "EXPERT"}
                                        ),
                                        ( (pair + "_cluster_moments_delta_" + mom + "_rel_ref;" + pair + "_cluster_moments_delta_" + mom + "_rel_ref_zoom_1",),
                                          {'type': 'TH1F', 
                                           'title': prettynames[0] + ";" + prettynames[1] + prettynames[2] + "; #Delta " + prettynames[1],
                                           'xbins': 51,
                                           'xmin':  -0.5,
                                           'xmax':  0.5,
                                           'path': "EXPERT"}
                                        ),
                                        ( (pair + "_cluster_moments_delta_" + mom + "_rel_ref;" + pair + "_cluster_moments_delta_" + mom + "_rel_ref_zoom_2",),
                                          {'type': 'TH1F', 
                                           'title': prettynames[0] + ";" + prettynames[1] + prettynames[2] + "; #Delta " + prettynames[1],
                                           'xbins': 51,
                                           'xmin':  -0.1,
                                           'xmax':  0.1,
                                           'path': "EXPERT"}
                                        ),
                                        ( (pair + "_cluster_moments_delta_" + mom + "_rel_ref;" + pair + "_cluster_moments_delta_" + mom + "_rel_ref_zoom_3",),
                                          {'type': 'TH1F', 
                                           'title': prettynames[0] + ";" + prettynames[1] + prettynames[2] + "; #Delta " + prettynames[1],
                                           'xbins': 51,
                                           'xmin':  -0.01,
                                           'xmax':  0.01,
                                           'path': "EXPERT"}
                                        ),
                                        ( (pair + "_cluster_moments_delta_" + mom + "_rel_ref;" + pair + "_cluster_moments_delta_" + mom + "_rel_ref_zoom_4",),
                                          {'type': 'TH1F', 
                                           'title': prettynames[0] + ";" + prettynames[1] + prettynames[2] + "; #Delta " + prettynames[1],
                                           'xbins': 51,
                                           'xmin':  -0.001,
                                           'xmax':  0.001,
                                           'path': "EXPERT"}
                                        ),
                                        ( (pair + "_cluster_moments_delta_" + mom + "_rel_ref;" + pair + "_cluster_moments_delta_" + mom + "_rel_ref_zoom_5",),
                                          {'type': 'TH1F', 
                                           'title': prettynames[0] + ";" + prettynames[1] + prettynames[2] + "; #Delta " + prettynames[1],
                                           'xbins': 51,
                                           'xmin':  -0.0001,
                                           'xmax':  0.0001,
                                           'path': "EXPERT"}
                                        )
                                      ]
                          
    def __call__(self, Plotter):
        for plotdef in self.PlotsToDo:
            Plotter.MonitoringTool.defineHistogram(*plotdef[0], **plotdef[1])
        return Plotter
        
