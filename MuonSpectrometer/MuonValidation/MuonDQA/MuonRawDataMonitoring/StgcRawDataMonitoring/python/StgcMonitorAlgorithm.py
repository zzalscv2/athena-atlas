#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration #
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

    #Shifter
    sTgcOverviewGroup  = helper.addGroup(sTgcMonAlg, 'sTgcOverview', 'Muon/MuonRawDataMonitoring/sTgc/Shifter')
    sTgcQuadOccupancyGroup     = helper.addGroup(sTgcMonAlg, 'sTgcQuadOccupancy', 'Muon/MuonRawDataMonitoring/sTgc/Shifter/Occupancy')
    sTgcLumiblockGroup = helper.addGroup(sTgcMonAlg, 'sTgcLumiblock', 'Muon/MuonRawDataMonitoring/sTgc/Shifter/Lumiblock')
    sTgcTimingGroup = helper.addGroup(sTgcMonAlg, 'sTgcTiming', 'Muon/MuonRawDataMonitoring/sTgc/Shifter/Timing')
    
    #Expert
    sTgcOccupancyGroup = helper.addGroup(sTgcMonAlg, 'sTgcOccupancy', 'Muon/MuonRawDataMonitoring/sTgc/Expert/Occupancy')

    
    # Layered and occupancy histograms
    side          = ['A', 'C']
    stationEtaMax = 3
    sectorMax     = 16
    layerMax      = 8
    
    for stationEtaIndex in range(1, stationEtaMax + 1):
        sTgcPadTimingExpertGroup = helper.addGroup(sTgcMonAlg, f'padTiming_quad_{stationEtaIndex}', 'Muon/MuonRawDataMonitoring/sTgc/Expert/Timing/Pad')
        sTgcStripTimingExpertGroup = helper.addGroup(sTgcMonAlg, f'stripTiming_quad_{stationEtaIndex}', 'Muon/MuonRawDataMonitoring/sTgc/Expert/Timing/Strip')
        sTgcWireTimingExpertGroup = helper.addGroup(sTgcMonAlg, f'wireTiming_quad_{stationEtaIndex}', 'Muon/MuonRawDataMonitoring/sTgc/Expert/Timing/Wire')
        for layerIndex in range(1, layerMax + 1):
            titleTimingPadTrack  = f'Q{stationEtaIndex}L{layerIndex}; Sector; Pad Timing (on-track) [ns]; Hits'
            varTimingPadTrack    = f'padTrackSectorSided_quad_{stationEtaIndex}_layer_{layerIndex},padTrackTiming_quad_{stationEtaIndex}_layer_{layerIndex};All_pad_timing_in_Q{stationEtaIndex}_Layer{layerIndex}'
            sTgcPadTimingExpertGroup.defineHistogram(varTimingPadTrack, type = 'TH2F', title = titleTimingPadTrack, path = f'Q{stationEtaIndex}', xbins = 2*sectorMax + 2, xmin = -float(sectorMax + 1), xmax = float(sectorMax + 1), ybins = 200, ymin = -75., ymax = 125., opt = 'kAlwaysCreate')

            titleTimingStripTrack  = f'Q{stationEtaIndex}L{layerIndex}; Sector; Strip Cluster Timing (on-track) [ns]; Hits'
            varTimingStripTrack    = f'stripTrackSectorSided_quad_{stationEtaIndex}_layer_{layerIndex},stripTrackTiming_quad_{stationEtaIndex}_layer_{layerIndex};All_strip_timing_in_Q{stationEtaIndex}_Layer{layerIndex}'
            sTgcStripTimingExpertGroup.defineHistogram(varTimingStripTrack, type = 'TH2F', title = titleTimingStripTrack, path = f'Q{stationEtaIndex}', xbins = 2*sectorMax + 2, xmin = -float(sectorMax + 1), xmax = float(sectorMax + 1), ybins = 200, ymin = -75., ymax = 125., opt = 'kAlwaysCreate')
            
            titleTimingWireTrack  = f'Q{stationEtaIndex}L{layerIndex}; Sector; Wire Group Timing (on-track) [ns]; Hits'
            varTimingWireTrack    = f'wireTrackSectorSided_quad_{stationEtaIndex}_layer_{layerIndex},wireTrackTiming_quad_{stationEtaIndex}_layer_{layerIndex};All_wire_timing_in_Q{stationEtaIndex}_Layer{layerIndex}'
            sTgcWireTimingExpertGroup.defineHistogram(varTimingWireTrack, type = 'TH2F', title = titleTimingWireTrack, path = f'Q{stationEtaIndex}', xbins = 2*sectorMax + 2, xmin = -float(sectorMax + 1), xmax = float(sectorMax + 1), ybins = 200, ymin = -75., ymax = 125., opt = 'kAlwaysCreate')

    for sideIndex in side:
        for stationEtaIndex in range(1, stationEtaMax + 1):
            for sectorIndex in range(1, sectorMax + 1):            
                padChargeGroup = helper.addGroup(sTgcMonAlg, f'padCharge_{sideIndex}{sectorIndex}_quad_{stationEtaIndex}', f'Muon/MuonRawDataMonitoring/sTgc/Expert/Charge/{sideIndex}' + f'{sectorIndex}'.zfill(2) + '/Pad')
                stripChargeGroup = helper.addGroup(sTgcMonAlg, f'stripCharge_{sideIndex}{sectorIndex}_quad_{stationEtaIndex}', f'Muon/MuonRawDataMonitoring/sTgc/Expert/Charge/{sideIndex}' + f'{sectorIndex}'.zfill(2) + '/Strip')
                wireGroupChargeGroup = helper.addGroup(sTgcMonAlg, f'wireGroupCharge_{sideIndex}{sectorIndex}_quad_{stationEtaIndex}', f'Muon/MuonRawDataMonitoring/sTgc/Expert/Charge/{sideIndex}' + f'{sectorIndex}'.zfill(2) + '/Wire')
                residualGroup = helper.addGroup(sTgcMonAlg, f'sTgcResiduals_{sideIndex}{sectorIndex}_quad_{stationEtaIndex}', f'Muon/MuonRawDataMonitoring/sTgc/Expert/Residuals/{sideIndex}' + f'{sectorIndex}'.zfill(2))
                
                for layerIndex in range(1, layerMax + 1):
                    titlePadChargeTrack = f'{sideIndex}' + f'{sectorIndex}'.zfill(2) + f'L{layerIndex}Q{stationEtaIndex}; Pad Charge (on-track) [fC]; Number of Entries'
                    varPadChargeTrack   = f'padTrackCharge_{sideIndex}_quad_{stationEtaIndex}_sector_{sectorIndex}_layer_{layerIndex};All_pad_charge_in_Q{stationEtaIndex}_Layer{layerIndex}'
                    padChargeGroup.defineHistogram(varPadChargeTrack, type = 'TH1F', title = titlePadChargeTrack, path = f'Q{stationEtaIndex}', xbins = 100, xmin = 0., xmax = 1000., opt = 'kAlwaysCreate')

                    titleStripChargeTrack = f'{sideIndex}' + f'{sectorIndex}'.zfill(2) + f'L{layerIndex}Q{stationEtaIndex}; Strip Cluster Charge (on-track) [fC]; Number of Entries'
                    varStripChargeTrack   = f'stripTrackCharge_{sideIndex}_quad_{stationEtaIndex}_sector_{sectorIndex}_layer_{layerIndex};All_strip_charge_in_Q{stationEtaIndex}_Layer{layerIndex}'
                    stripChargeGroup.defineHistogram(varStripChargeTrack, type = 'TH1F', title = titleStripChargeTrack, path = f'Q{stationEtaIndex}', xbins = 120, xmin = 0., xmax = 1200., opt = 'kAlwaysCreate')
    
                    titleWireGroupChargeTrack = f'{sideIndex}' + f'{sectorIndex}'.zfill(2) + f'L{layerIndex}Q{stationEtaIndex}; Wire Group Charge (on-track) [fC]; Number of Entries'
                    varWireGroupChargeTrack   = f'wireGroupTrackCharge_{sideIndex}_quad_{stationEtaIndex}_sector_{sectorIndex}_layer_{layerIndex};All_wire_charge_in_Q{stationEtaIndex}_Layer{layerIndex}'
                    wireGroupChargeGroup.defineHistogram(varWireGroupChargeTrack, type = 'TH1F', title = titleWireGroupChargeTrack, path = f'Q{stationEtaIndex}', xbins = 100, xmin = 0., xmax = 1000., opt = 'kAlwaysCreate')
                    
                    titleResidual = f'{sideIndex}' + f'{sectorIndex}'.zfill(2) + f'L{layerIndex}Q{stationEtaIndex}; Residual [mm]; Number of Entries'
                    varResidual = f'residual_{sideIndex}_quad_{stationEtaIndex}_sector_{sectorIndex}_layer_{layerIndex};Residuals_in_Q{stationEtaIndex}_Layer{layerIndex}'
                    residualGroup.defineHistogram(varResidual, type = 'TH1F', title = titleResidual, path = f'Q{stationEtaIndex}', xbins = 1000, xmin = -2., xmax = 2., opt = 'kAlwaysCreate')

    
    for layerIndex in range(1, layerMax + 1):
        titleStripClusterSizeTrack = f'L{layerIndex}; Sector; Strip Cluster Size (on-track); Hits'
        varStripClusterSizeTrack   = f'stripTrackSectorSided_layer_{layerIndex},stripTrackClusterSize_layer_{layerIndex};Strip_cluster_size_ontrk_per_sector_Layer{layerIndex}'
        sTgcOverviewGroup.defineHistogram(varStripClusterSizeTrack, type = 'TH2F', title = titleStripClusterSizeTrack, path = 'Overview', xbins = 2*sectorMax + 2, xmin = -float(sectorMax + 1), xmax = float(sectorMax + 1), ybins = 10, ymin = 0., ymax = 10., opt = 'kAlwaysCreate')

        titleTimingStripTrack  = f'L{layerIndex}; Sector; Strip Cluster Timing (on-track) [ns]; Hits'
        varTimingStripTrack    = f'stripTrackSectorSided_layer_{layerIndex},stripTrackTiming_layer_{layerIndex};Strip_cluster_timing_ontrk_per_sector_Layer{layerIndex}'
        sTgcOverviewGroup.defineHistogram(varTimingStripTrack, type = 'TH2F', title = titleTimingStripTrack, path = 'Overview', xbins = 2*sectorMax + 2, xmin = -float(sectorMax + 1), xmax = float(sectorMax + 1), ybins = 200, ymin = -75., ymax = 125., opt = 'kAlwaysCreate')
        
        titleStationEtaSectorPadHitMap  = f'L{layerIndex}; Sector; Quad; Hits'
        varStationEtaSectorPadHitMap    = f'sector_layer_{layerIndex},stationEta_layer_{layerIndex};Pad_quad_occupancy_per_sector_Layer{layerIndex}'
        sTgcQuadOccupancyGroup.defineHistogram(varStationEtaSectorPadHitMap, type = 'TH2F', title = titleStationEtaSectorPadHitMap, path = 'Pad', xbins = sectorMax, xmin = 1., xmax = float(sectorMax + 1), ybins = 2*stationEtaMax + 2, ymin = -float(stationEtaMax + 1), ymax = float(stationEtaMax + 1), opt = 'kAlwaysCreate')
            
        titleStationEtaSectorStripHitMap  = f'L{layerIndex}; Sector; Quad; Hits'
        varStationEtaSectorStripHitMap    = f'sector_layer_{layerIndex},stationEta_layer_{layerIndex};Strip_quad_occupancy_per_sector_Layer{layerIndex}'
        sTgcQuadOccupancyGroup.defineHistogram(varStationEtaSectorStripHitMap, type = 'TH2F', title = titleStationEtaSectorStripHitMap, path = 'Strip', xbins = sectorMax, xmin = 1., xmax = float(sectorMax + 1), ybins = 2*stationEtaMax + 2, ymin = -float(stationEtaMax + 1), ymax = float(stationEtaMax + 1), opt = 'kAlwaysCreate')

        titleStationEtaSectorWireGroupHitMap  = f'L{layerIndex}; Sector; Quad; Hits'
        varStationEtaSectorWireGroupHitMap    = f'sector_layer_{layerIndex},stationEta_layer_{layerIndex};Wire_quad_occupancy_per_sector_Layer{layerIndex}'
        sTgcQuadOccupancyGroup.defineHistogram(varStationEtaSectorWireGroupHitMap, type = 'TH2F', title = titleStationEtaSectorWireGroupHitMap, path = 'Wire', xbins = sectorMax, xmin = 1., xmax = float(sectorMax + 1), ybins = 2*stationEtaMax + 2, ymin = -float(stationEtaMax + 1), ymax = float(stationEtaMax + 1), opt = 'kAlwaysCreate')

        titleTimingPadTrack  = f'L{layerIndex}; Sector; Pad Timing (on-track) [ns]; Hits'
        varTimingPadTrack    = f'padTrackSectorSided_layer_{layerIndex},padTrackTiming_layer_{layerIndex};All_pad_timing_per_sector_Layer{layerIndex}'
        sTgcTimingGroup.defineHistogram(varTimingPadTrack, type = 'TH2F', title = titleTimingPadTrack, path = 'Pad', xbins = 2*sectorMax + 2, xmin = -float(sectorMax + 1), xmax = float(sectorMax + 1), ybins = 200, ymin = -75., ymax = 125., opt = 'kAlwaysCreate')

        titleTimingStripTrack  = f'L{layerIndex}; Sector; Strip Cluster Timing (on-track) [ns]; Hits'
        varTimingStripTrack    = f'stripTrackSectorSided_layer_{layerIndex},stripTrackTiming_layer_{layerIndex};All_strip_timing_per_sector_Layer{layerIndex}'
        sTgcTimingGroup.defineHistogram(varTimingStripTrack, type = 'TH2F', title = titleTimingStripTrack, path = 'Strip', xbins = 2*sectorMax + 2, xmin = -float(sectorMax + 1), xmax = float(sectorMax + 1), ybins = 200, ymin = -75., ymax = 125., opt = 'kAlwaysCreate')
        
        titleTimingWireGroupTrack  = f'L{layerIndex}; Sector; Wire Group timing (on-track) [ns]; Hits'
        varTimingWireGroupTrack    = f'wireGroupTrackSectorSided_layer_{layerIndex},wireGroupTrackTiming_layer_{layerIndex};All_wire_timing_per_sector_Layer{layerIndex}'
        sTgcTimingGroup.defineHistogram(varTimingWireGroupTrack, type = 'TH2F', title = titleTimingWireGroupTrack, path = 'Wire', xbins = 2*sectorMax + 2, xmin = -float(sectorMax + 1), xmax = float(sectorMax + 1), ybins = 200, ymin = -75., ymax = 125., opt = 'kAlwaysCreate')

        titleSectorsVersusLumiblockPad  = f'L{layerIndex}; LB (Pad); All Sectors; Hits'
        varSectorsVersusLumiblockPad    = f'padLumiblock_layer_{layerIndex},padStationEta_layer_{layerIndex};Nhits_all_pad_in_sector_per_LB_Layer{layerIndex}'
        sTgcLumiblockGroup.defineHistogram(varSectorsVersusLumiblockPad, type = 'TH2F', title = titleSectorsVersusLumiblockPad, path = 'Pad', xbins = 3000, xmin = 0., xmax = 3000., ybins = 2*(3*sectorMax + 1), ymin = -float(3*sectorMax + 1), ymax = float(3*sectorMax + 1), ylabels = LumiblockYlabel, opt = 'kAlwaysCreate')
            
        titleSectorsVersusLumiblockStrip  = f'L{layerIndex}; LB (Strip); All Sectors; Hits'
        varSectorsVersusLumiblockStrip    = f'stripLumiblock_layer_{layerIndex},stripStationEta_layer_{layerIndex};Nhits_all_strip_in_sector_per_LB_Layer{layerIndex}'
        sTgcLumiblockGroup.defineHistogram(varSectorsVersusLumiblockStrip, type = 'TH2F', title = titleSectorsVersusLumiblockStrip, path = 'Strip', xbins = 3000, xmin = 0., xmax = 3000., ybins = 2*(3*sectorMax + 1), ymin = -float(3*sectorMax + 1), ymax = float(3*sectorMax + 1), ylabels = LumiblockYlabel, opt = 'kAlwaysCreate')

        titleSectorsVersusLumiblockWireGroup  = f'L{layerIndex}; LB (Wire); All Sectors; Hits'
        varSectorsVersusLumiblockWireGroup    = f'wireGroupLumiblock_layer_{layerIndex},wireGroupStationEta_layer_{layerIndex};Nhits_all_wire_in_sector_per_LB_Layer{layerIndex}'
        sTgcLumiblockGroup.defineHistogram(varSectorsVersusLumiblockWireGroup, type = 'TH2F', title = titleSectorsVersusLumiblockWireGroup, path = 'Wire', xbins = 3000, xmin = 0., xmax = 3000., ybins = 2*(3*sectorMax + 1), ymin = -float(3*sectorMax + 1), ymax = float(3*sectorMax + 1), ylabels = LumiblockYlabel, opt = 'kAlwaysCreate')
        
        titlePadOccupancy  = f'L{layerIndex}; Sector; Pad Number; Hits'
        varPadOccupancy    = f'sector_layer_{layerIndex},padNumber_layer_{layerIndex};Pad_ch_occupancy_per_sector_Layer{layerIndex}'
        sTgcOccupancyGroup.defineHistogram(varPadOccupancy, type = 'TH2F', title = titlePadOccupancy, path = 'Pad', xbins = 2*sectorMax + 2, xmin = -float(sectorMax + 1), xmax = float(sectorMax + 1), ybins = 317, ymin = 0., ymax = 317., opt = 'kAlwaysCreate')
                        
        titleStripOccupancy  = f'L{layerIndex}; Sector; Strip Number; Hits'
        varStripOccupancy    = f'sector_layer_{layerIndex},stripNumber_layer_{layerIndex};Strip_ch_occupancy_per_sector_Layer{layerIndex}'
        sTgcOccupancyGroup.defineHistogram(varStripOccupancy, type = 'TH2F', title = titleStripOccupancy, path = 'Strip', xbins = 2*sectorMax + 2, xmin = -float(sectorMax + 1), xmax = float(sectorMax + 1), ybins = 1130, ymin = 0., ymax = 1130., opt = 'kAlwaysCreate')
            
        titleWireGroupOccupancyPerQuad  = f'L{layerIndex}; Wire Group Number; Quad; Hits'
        varWireGroupOccupancyPerQuad    = f'wireGroupNumber_layer_{layerIndex},stationEta_layer_{layerIndex};Wire_ch_occupancy_per_sector_Layer{layerIndex}'
        sTgcOccupancyGroup.defineHistogram(varWireGroupOccupancyPerQuad, type = 'TH2F', title = titleWireGroupOccupancyPerQuad, path = 'Wire', xbins = 58*sectorMax, xmin = 0., xmax = float(58*sectorMax), ybins = 2*stationEtaMax + 2, ymin = -float(stationEtaMax + 1), ymax = float(stationEtaMax + 1), opt = 'kAlwaysCreate')
    

    acc = helper.result()
    result.merge(acc)
    return result
if __name__=='__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("--events", default = 100, type = int, help = 'Number of events that you want to run.')
    parser.add_argument("--samples", nargs = "+", default = None, help = 'Path to the input samples. If you want to run multiple samples at once you have to introduce them separated by blank spaces.')
    parser.add_argument("--output", default = "sTgcExampleOutput.root", help = 'Name of the output ROOT file.')
    args = parser.parse_args()

    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Input.Files += args.samples 
    
    flags.Output.HISTFileName = args.output

    flags.Detector.GeometrysTGC = True
    flags.DQ.useTrigger = False

    flags.lock()
    flags.dump()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))
    sTgcMonitorAcc  =  sTgcMonitoringConfig(flags)
    sTgcMonitorAcc.OutputLevel = 2
    cfg.merge(sTgcMonitorAcc)           
    
    # number of events selected in the ESD
    cfg.run(args.events)
