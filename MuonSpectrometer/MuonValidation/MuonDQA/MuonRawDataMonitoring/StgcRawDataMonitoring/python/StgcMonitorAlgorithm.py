#
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration #
#

from AthenaConfiguration.ComponentFactory import CompFactory
from StgcRawDataMonitoring.StgcRawMonLabels import LumiblockYlabel 

def sTgcMonitoringConfig(inputFlags):
    '''Function to configures some algorithms in the monitoring system.'''
    ### STEP 1 ###
    # Define one top-level monitoring algorithm. The new configuration 
    # framework uses a component accumulator.
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    
    # Make sure muon geometry is configured
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    result.merge(MuonGeoModelCfg(inputFlags))

    # The following class will make a sequence, configure algorithms, and link
    # them to GenericMonitoringTools

    from AthenaCommon.AppMgr import ServiceMgr
    ServiceMgr.Dump = False

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags, 'StgcAthMonitorCfg')

    # Adding an algorithm to the helper.
    sTgcMonAlg = helper.addAlgorithm(CompFactory.sTgcRawDataMonAlg,'sTgcMonAlg')
    sTgcMonAlg.dosTgcESD = True  

    # Add a generic monitoring tool (a "group" in old language). The returned      
    # object here is the standard GenericMonitoringTool. 
    sTgcOverviewGroup           = helper.addGroup(sTgcMonAlg, 'sTgcOverview', 'Muon/MuonRawDataMonitoring/sTgc/sTgcOverview')
    sTgcLayerGroup              = helper.addGroup(sTgcMonAlg, 'sTgcLayers', 'Muon/MuonRawDataMonitoring/sTgc/sTgcLayers')
    sTgcOccupancyGroup          = helper.addGroup(sTgcMonAlg, 'sTgcOccupancy', 'Muon/MuonRawDataMonitoring/sTgc/sTgcOccupancy')
    sTgcLumiblockGroup          = helper.addGroup(sTgcMonAlg, 'sTgcLumiblock', 'Muon/MuonRawDataMonitoring/sTgc/sTgcLumiblock')
    sTgcTimingGroup             = helper.addGroup(sTgcMonAlg, 'sTgcTiming', 'Muon/MuonRawDataMonitoring/sTgc/sTgcTiming')
    sTgcClusterFromSegmentGroup = helper.addGroup(sTgcMonAlg, 'sTgcClusterFromSegment', 'Muon/MuonRawDataMonitoring/sTgc/sTgcClusterFromSegment')

    # Configure histograms
    # Overview histograms
    sTgcOverviewGroup.defineHistogram('charge;chargeOverview', type = 'TH1F', title = 'charge; charge [fC]; number of entries', path = 'Overview', xbins = 100, xmin = 0., xmax = 1000.)
    sTgcOverviewGroup.defineHistogram('numberOfStripsPerCluster;numberOfStripsPerCluster', type = 'TH1F', title = 'number of strips per cluster; number of strips per cluster; number of entries', path = 'Overview', xbins = 20, xmin = 0., xmax = 20.)
    sTgcOverviewGroup.defineHistogram('numberOfStripsPerClusterMuonSegments;numberOfStripsPerClusterMuonSegmentsOverview', type = 'TH1F', title = 'number of strips per cluster (muon segments); number of strips per cluster; number of entries', path = 'Overview', xbins = 20, xmin = 0., xmax = 20.)
    sTgcOverviewGroup.defineHistogram('time;timeOverview', type = 'TH1F', title = 'time; time [ns]; number of entries', path = 'Overview', xbins = 900, xmin = -200., xmax = 700.) 
    sTgcOverviewGroup.defineHistogram('stripTimes;stripTimesOverview', type = 'TH1F', title = 'strip time; strip time [ns]; number of entries', path = 'Overview', xbins = 900, xmin = -200., xmax = 700.) 
    sTgcOverviewGroup.defineHistogram('stripCharges;stripChargesOverview', type = 'TH1F', title = 'strip charge; strip charge [fC]; number of entries', path = 'Overview', xbins = 120, xmin = 0., xmax = 1200.) 

    sTgcOverviewGroup.defineHistogram('stripNumbers;stripNumbersOverview', type = 'TH1F', title = 'strip number; strip number; number of entries', path = 'Overview', xbins = 410, xmin = 0., xmax = 410.) 
    sTgcOverviewGroup.defineHistogram('x,y;xyOverview', type = 'TH2F', title='y vs x; sTgc-GlobalX [mm]; sTgc-GlobalY [mm]; number of entries', path = 'Overview', xbins = 500, xmin = -5000., xmax = 5000., ybins = 500, ymin = -5000., ymax = 5000.) 
    sTgcOverviewGroup.defineHistogram('r,z;rzOverview', type = 'TH2F', title = 'z vs R; sTgc-GlobalR [mm]; sTgc-GlobalZ [mm]; number of entries', path = 'Overview', xbins = 500, xmin = 0., xmax = 5000., ybins = 1000, ymin = -10000., ymax = 10000.) 

    # Layered and occupancy histograms
    stationEtaMax = 3
    sectorMax     = 16
    layerMax      = 8
        
    for layerIndex in range(1, layerMax + 1):
        titlePadOccupancy  = f'layer {layerIndex}; sector; pad number; hits'
        varPadOccupancy    = f'sector_layer_{layerIndex},padNumber_layer_{layerIndex};padOccupancy_layer_{layerIndex}'
        weightPadOccupancy = f'padHit_layer_{layerIndex}' 
        sTgcOccupancyGroup.defineHistogram(varPadOccupancy, type = 'TH2F', title = titlePadOccupancy, path = 'padOccupancy', xbins = 2*sectorMax + 2, xmin = -float(sectorMax + 1), xmax = float(sectorMax + 1), ybins = 317, ymin = 0., ymax = 317., opt = 'kAlwaysCreate', weight = weightPadOccupancy)
                        
        titleStripOccupancy  = f'layer {layerIndex}; sector; strip number; hits'
        varStripOccupancy    = f'sector_layer_{layerIndex},stripNumber_layer_{layerIndex};stripOccupancy_layer_{layerIndex}'
        weightStripOccupancy = f'stripHit_layer_{layerIndex}' 
        sTgcOccupancyGroup.defineHistogram(varStripOccupancy, type = 'TH2F', title = titleStripOccupancy, path = 'stripOccupancy', xbins = 2*sectorMax + 2, xmin = -float(sectorMax + 1), xmax = float(sectorMax + 1), ybins = 1130, ymin = 0., ymax = 1130., opt = 'kAlwaysCreate', weight = weightStripOccupancy)
            
        titleWireGroupOccupancyPerQuad  = f'layer {layerIndex}; wire group number; station eta; hits'
        varWireGroupOccupancyPerQuad    = f'wireGroupNumber_layer_{layerIndex},stationEta_layer_{layerIndex};wireGroupOccupancyPerQuad_layer_{layerIndex}'
        weightWireGroupOccupancyPerQuad = f'wireGroupHit_layer_{layerIndex}'
        sTgcOccupancyGroup.defineHistogram(varWireGroupOccupancyPerQuad, type = 'TH2F', title = titleWireGroupOccupancyPerQuad, path = 'wireGroupOccupancy', xbins = 58*sectorMax, xmin = 0., xmax = float(58*sectorMax), ybins = 2*stationEtaMax + 2, ymin = -float(stationEtaMax + 1), ymax = float(stationEtaMax + 1), opt = 'kAlwaysCreate', weight = weightWireGroupOccupancyPerQuad)
            
        titleStationEtaSectorPadHitMap  = f'layer {layerIndex}; sector; station eta; pad hits'
        varStationEtaSectorPadHitMap    = f'sector_layer_{layerIndex},stationEta_layer_{layerIndex};stationEtaSectorPadHitMap_layer_{layerIndex}'
        weightStationEtaSectorPadHitMap = f'padHitLayers_layer_{layerIndex}'
        sTgcLayerGroup.defineHistogram(varStationEtaSectorPadHitMap, type = 'TH2F', title = titleStationEtaSectorPadHitMap, path = 'stationEtaSectorPadHitMap', xbins = sectorMax, xmin = 1., xmax = float(sectorMax + 1), ybins = 2*stationEtaMax + 2, ymin = -float(stationEtaMax + 1), ymax = float(stationEtaMax + 1), opt = 'kAlwaysCreate', weight = weightStationEtaSectorPadHitMap)
            
        titleStationEtaSectorStripHitMap  = f'layer {layerIndex}; sector; station eta; strip hits'
        varStationEtaSectorStripHitMap    = f'sector_layer_{layerIndex},stationEta_layer_{layerIndex};stationEtaSectorStripHitMap_layer_{layerIndex}'
        weightStationEtaSectorStripHitMap = f'stripHitLayers_layer_{layerIndex}'
        sTgcLayerGroup.defineHistogram(varStationEtaSectorStripHitMap, type = 'TH2F', title = titleStationEtaSectorStripHitMap, path = 'stationEtaSectorStripHitMap', xbins = sectorMax, xmin = 1., xmax = float(sectorMax + 1), ybins = 2*stationEtaMax + 2, ymin = -float(stationEtaMax + 1), ymax = float(stationEtaMax + 1), opt = 'kAlwaysCreate', weight = weightStationEtaSectorStripHitMap)

        titleStationEtaSectorWireGroupHitMap  = f'layer {layerIndex}; sector; station eta; wire group hits'
        varStationEtaSectorWireGroupHitMap    = f'sector_layer_{layerIndex},stationEta_layer_{layerIndex};stationEtaSectorStripHitMap_layer_{layerIndex}'
        weightStationEtaSectorWireGroupHitMap = f'wireGroupHitLayers_layer_{layerIndex}'
        sTgcLayerGroup.defineHistogram(varStationEtaSectorWireGroupHitMap, type = 'TH2F', title = titleStationEtaSectorWireGroupHitMap, path = 'stationEtaSectorWireGroupHitMap', xbins = sectorMax, xmin = 1., xmax = float(sectorMax + 1), ybins = 2*stationEtaMax + 2, ymin = -float(stationEtaMax + 1), ymax = float(stationEtaMax + 1), opt = 'kAlwaysCreate', weight = weightStationEtaSectorWireGroupHitMap)
            
        titleTimingPad  = f'layer {layerIndex}; sector; time (pad) [ns]; hits'
        varTimingPad    = f'padSectorSided_layer_{layerIndex},padTiming_layer_{layerIndex};padTiming_layer_{layerIndex}'
        weightTimingPad = f'padHit_layer_{layerIndex}'
        sTgcTimingGroup.defineHistogram(varTimingPad, type = 'TH2F', title = titleTimingPad, path = 'padTiming', xbins = 2*sectorMax + 2, xmin = -float(sectorMax + 1), xmax = float(sectorMax + 1), ybins = 200, ymin = -75., ymax = 125., opt = 'kAlwaysCreate', weight = weightTimingPad)

        titleTimingWireGroup  = f'layer {layerIndex}; sector; time (wire group) [ns]; hits'
        varTimingWireGroup    = f'wireGroupSectorSided_layer_{layerIndex},wireGroupTiming_layer_{layerIndex};wireGroupTiming_layer_{layerIndex}'
        weightTimingWireGroup = f'wireGroupHit_layer_{layerIndex}'
        sTgcTimingGroup.defineHistogram(varTimingWireGroup, type = 'TH2F', title = titleTimingWireGroup, path = 'wireGroupTiming', xbins = 2*sectorMax + 2, xmin = -float(sectorMax + 1), xmax = float(sectorMax + 1), ybins = 200, ymin = -75., ymax = 125., opt = 'kAlwaysCreate', weight = weightTimingWireGroup)

        titleTimingStripCluster  = f'layer {layerIndex}; sector; time (strip segments) [ns]; hits'
        varTimingStripCluster    = f'stripClusterSectorSided_layer_{layerIndex},stripClusterTiming_layer_{layerIndex};stripClusterTiming_layer_{layerIndex}'
        weightTimingStripCluster = f'stripClusterHit_layer_{layerIndex}'
        sTgcClusterFromSegmentGroup.defineHistogram(varTimingStripCluster, type = 'TH2F', title = titleTimingStripCluster, path = 'stripClusterTime', xbins = 2*sectorMax + 2, xmin = -float(sectorMax + 1), xmax = float(sectorMax + 1), ybins = 220, ymin = -85., ymax = 135., opt = 'kAlwaysCreate', weight = weightTimingStripCluster)

        titleStripClusterSize  = f'layer {layerIndex}; sector; cluster size (segments); hits'
        varStripClusterSize    = f'stripClusterSectorSided_layer_{layerIndex},stripClusterSize_layer_{layerIndex};stripClusterSize_layer_{layerIndex}'
        weightStripClusterSize = f'stripClusterHit_layer_{layerIndex}'
        sTgcClusterFromSegmentGroup.defineHistogram(varStripClusterSize, type = 'TH2F', title = titleStripClusterSize, path = 'stripClusterSize', xbins = 2*sectorMax + 2, xmin = -float(sectorMax + 1), xmax = float(sectorMax + 1), ybins = 20, ymin = 0., ymax = 20., opt = 'kAlwaysCreate', weight = weightStripClusterSize)

        titleSectorsVersusLumiblockPad  = f'layer {layerIndex}; LB; all sectors; pad hits'
        varSectorsVersusLumiblockPad    = f'padLumiblock_layer_{layerIndex},padStationEta_layer_{layerIndex};sectorsVersusLumiblockPad_layer_{layerIndex}'
        weightSectorsVersusLumiblockPad = f'padHit_layer_{layerIndex}'
        sTgcLumiblockGroup.defineHistogram(varSectorsVersusLumiblockPad, type = 'TH2F', title = titleSectorsVersusLumiblockPad, path = 'padSTGClumiblock', xbins = 3000, xmin = 0., xmax = 3000., ybins = 2*(3*sectorMax + 1), ymin = -float(3*sectorMax + 1), ymax = float(3*sectorMax + 1), ylabels = LumiblockYlabel, opt = 'kAlwaysCreate', weight = weightSectorsVersusLumiblockPad)
            
        titleSectorsVersusLumiblockStrip  = f'layer {layerIndex}; LB; all sectors; strip hits'
        varSectorsVersusLumiblockStrip    = f'stripLumiblock_layer_{layerIndex},stripStationEta_layer_{layerIndex};sectorsVersusLumiblockStrip_layer_{layerIndex}'
        weightSectorsVersusLumiblockStrip = f'stripHit_layer_{layerIndex}'
        sTgcLumiblockGroup.defineHistogram(varSectorsVersusLumiblockStrip, type = 'TH2F', title = titleSectorsVersusLumiblockStrip, path = 'stripSTGClumiblock', xbins = 3000, xmin = 0., xmax = 3000., ybins = 2*(3*sectorMax + 1), ymin = -float(3*sectorMax + 1), ymax = float(3*sectorMax + 1), ylabels = LumiblockYlabel, opt = 'kAlwaysCreate', weight = weightSectorsVersusLumiblockStrip)

        titleSectorsVersusLumiblockWireGroup  = f'layer {layerIndex}; LB; all sectors; wireGroup hits'
        varSectorsVersusLumiblockWireGroup    = f'wireGroupLumiblock_layer_{layerIndex},wireGroupStationEta_layer_{layerIndex};sectorsVersusLumiblockWireGroup_layer_{layerIndex}'
        weightSectorsVersusLumiblockWireGroup = f'wireGroupHit_layer_{layerIndex}'
        sTgcLumiblockGroup.defineHistogram(varSectorsVersusLumiblockWireGroup, type = 'TH2F', title = titleSectorsVersusLumiblockWireGroup, path = 'wireGroupSTGClumiblock', xbins = 3000, xmin = 0., xmax = 3000., ybins = 2*(3*sectorMax + 1), ymin = -float(3*sectorMax + 1), ymax = float(3*sectorMax + 1), ylabels = LumiblockYlabel, opt = 'kAlwaysCreate', weight = weightSectorsVersusLumiblockWireGroup)

    acc = helper.result()
    result.merge(acc)
    return result
if __name__=='__main__':
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("--events", default = 100, type = int, help = 'Number of events that you want to run.')
    parser.add_argument("--samples", nargs = "+", default = None, help = 'Path to the input samples. If you want to run multiple samples at once you have to introduce them separated by blank spaces.')
    parser.add_argument("--output", default = "monitor_sTgc.root", help = 'Name of the output ROOT file.')
    args = parser.parse_args()

    ConfigFlags.Input.Files = []
    ConfigFlags.Input.Files += args.samples 
    
    ConfigFlags.Output.HISTFileName = args.output

    ConfigFlags.Detector.GeometrysTGC = True
    ConfigFlags.DQ.useTrigger = False

    ConfigFlags.lock()
    ConfigFlags.dump()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    
    cfg = MainServicesCfg(ConfigFlags)
    cfg.merge(PoolReadCfg(ConfigFlags))
    sTgcMonitorAcc  =  sTgcMonitoringConfig(ConfigFlags)
    sTgcMonitorAcc.OutputLevel = 2
    cfg.merge(sTgcMonitorAcc)           
    
    # number of events selected in the ESD
    cfg.run(args.events)
