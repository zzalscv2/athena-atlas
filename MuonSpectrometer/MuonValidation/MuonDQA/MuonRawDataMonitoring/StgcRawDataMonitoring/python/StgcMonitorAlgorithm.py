#
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration #
#

from AthenaConfiguration.ComponentFactory import CompFactory

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
    sTgcOverviewGroup      = helper.addGroup(sTgcMonAlg, 'sTgcOverview', 'Muon/MuonRawDataMonitoring/sTgc/sTgcOverview')
    sTgcLayerGroup         = helper.addGroup(sTgcMonAlg, 'sTgcLayers', 'Muon/MuonRawDataMonitoring/sTgc/sTgcLayers')
    sTgcOccupancyGroup     = helper.addGroup(sTgcMonAlg, 'sTgcOccupancy', 'Muon/MuonRawDataMonitoring/sTgc/sTgcOccupancy')
    
    # Configure histograms
    # Overview histograms
    sTgcOverviewGroup.defineHistogram('charge;chargeOverview', type = 'TH1F', title = 'charge; charge [fC]; number of entries', path = 'Overview', xbins = 200, xmin = 0., xmax = 5000.)
    sTgcOverviewGroup.defineHistogram('numberOfStripsPerCluster;numberOfStripsPerCluster', type = 'TH1F', title = 'number of strips per cluster; number of strips per cluster; number of entries', path = 'Overview', xbins = 12, xmin = 0., xmax = 12.)
    sTgcOverviewGroup.defineHistogram('time;timeOverview', type = 'TH1F', title = 'time; time [ns]; number of entries', path = 'Overview', xbins = 900, xmin = -200., xmax = 700.) 
    
    sTgcOverviewGroup.defineHistogram('stripTimes;stripTimesOverview', type = 'TH1F', title = 'strip time; strip time [ns]; number of entries', path = 'Overview', xbins = 900, xmin = -200., xmax = 700.) 
    sTgcOverviewGroup.defineHistogram('stripCharges;stripChargesOverview', type = 'TH1F', title = 'strip charge; strip charge [fC]; number of entries', path = 'Overview', xbins = 1000, xmin = 0., xmax = 1000.) 
    sTgcOverviewGroup.defineHistogram('stripNumbers;stripNumbersOverview', type = 'TH1F', title = 'strip number; strip number; number of entries', path = 'Overview', xbins = 410, xmin = 0., xmax = 410.) 
    sTgcOverviewGroup.defineHistogram('x,y;xyOverview', type = 'TH2F', title='x vs y; sTgc-GlobalX [mm]; sTgc-GlobalY [mm]; number of entries', path = 'Overview', xbins = 500, xmin = -5000, xmax = 5000., ybins = 500, ymin = -5000., ymax = 5000.) 
    sTgcOverviewGroup.defineHistogram('r,z;rzOverview', type = 'TH2F', title = 'r vs z; sTgc-GlobalR [mm]; sTgc-GlobalZ [mm]; number of entries', path = 'Overview', xbins = 500, xmin = 0., xmax = 5000., ybins = 1000, ymin = -10000., ymax = 10000.) 

    # Layered and occupancy histograms
    stationEtaMax = 3
    sectorMax     = 16
    multipletMax  = 2
    gasGapMax     = 4
    
    for multipletIndex in range(1, multipletMax + 1):
        for gasGapIndex in range(1, gasGapMax + 1):
            titlePadOccupancy  = f'pad occupancy: multiplet {multipletIndex} gasgap {gasGapIndex}; sector; pad number; hits'
            varPadOccupancy    = f'sector_multiplet_{multipletIndex}_gasgap_{gasGapIndex},padNumber_multiplet_{multipletIndex}_gasgap_{gasGapIndex};padOccupancy_multiplet_{multipletIndex}_gasgap_{gasGapIndex}'
            weightPadOccupancy = f'padHit_multiplet_{multipletIndex}_gasgap_{gasGapIndex}' 
            sTgcOccupancyGroup.defineHistogram(varPadOccupancy, type = 'TH2F', title = titlePadOccupancy, path = 'padOccupancy', xbins = 2*sectorMax + 2, xmin = -float(sectorMax + 1), xmax = float(sectorMax + 1), ybins = 317, ymin = 0., ymax = 317., opt = 'kAlwaysCreate', weight = weightPadOccupancy)
                        
            titleStripOccupancy  = f'strip occupancy: multiplet {multipletIndex} gasgap {gasGapIndex}; sector; strip number; hits'
            varStripOccupancy    = f'sector_multiplet_{multipletIndex}_gasgap_{gasGapIndex},stripNumber_multiplet_{multipletIndex}_gasgap_{gasGapIndex};stripOccupancy_multiplet_{multipletIndex}_gasgap_{gasGapIndex}'
            weightStripOccupancy = f'stripHit_multiplet_{multipletIndex}_gasgap_{gasGapIndex}' 
            sTgcOccupancyGroup.defineHistogram(varStripOccupancy, type = 'TH2F', title = titleStripOccupancy, path = 'stripOccupancy', xbins = 2*sectorMax + 2, xmin = -float(sectorMax + 1), xmax = float(sectorMax + 1), ybins = 1130, ymin = 0., ymax = 1130., opt = 'kAlwaysCreate', weight = weightStripOccupancy)
                        
            titleStationEtaSectorPadHitMap  = f'stationEta vs sector hit map (pad): multiplet {multipletIndex} gasgap {gasGapIndex}; sector; station eta; pad hits'
            varStationEtaSectorPadHitMap    = f'sector_multiplet_{multipletIndex}_gasgap_{gasGapIndex},stationEta_multiplet_{multipletIndex}_gasgap_{gasGapIndex};stationEtaSectorPadHitMap_multiplet_{multipletIndex}_gasgap_{gasGapIndex}'
            weightStationEtaSectorPadHitMap = f'padHitLayers_multiplet_{multipletIndex}_gasgap_{gasGapIndex}'
            sTgcLayerGroup.defineHistogram(varStationEtaSectorPadHitMap, type = 'TH2F', title = titleStationEtaSectorPadHitMap, path = 'stationEtaSectorPadHitMap', xbins = sectorMax, xmin = 1., xmax = float(sectorMax + 1), ybins = 2*stationEtaMax + 2, ymin = -float(stationEtaMax + 1), ymax = float(stationEtaMax + 1), opt = 'kAlwaysCreate', weight = weightStationEtaSectorPadHitMap)
            
            titleStationEtaSectorStripHitMap  = f'stationEta vs sector hit map (strip): multiplet {multipletIndex} gasgap {gasGapIndex}; sector; station eta; strip hits'
            varStationEtaSectorStripHitMap    = f'sector_multiplet_{multipletIndex}_gasgap_{gasGapIndex},stationEta_multiplet_{multipletIndex}_gasgap_{gasGapIndex};stationEtaSectorStripHitMap_multiplet_{multipletIndex}_gasgap_{gasGapIndex}'
            weightStationEtaSectorStripHitMap = f'stripHitLayers_multiplet_{multipletIndex}_gasgap_{gasGapIndex}'
            sTgcLayerGroup.defineHistogram(varStationEtaSectorStripHitMap, type = 'TH2F', title = titleStationEtaSectorStripHitMap, path = 'stationEtaSectorStripHitMap', xbins = sectorMax, xmin = 1., xmax = float(sectorMax + 1), ybins = 2*stationEtaMax + 2, ymin = -float(stationEtaMax + 1), ymax = float(stationEtaMax + 1), opt = 'kAlwaysCreate', weight = weightStationEtaSectorStripHitMap)

            titleStationEtaSectorWireGroupHitMap  = f'stationEta vs sector hit map (wire): multiplet {multipletIndex} gasgap {gasGapIndex}; sector; station eta; wire group hits'
            varStationEtaSectorWireGroupHitMap    = f'sector_multiplet_{multipletIndex}_gasgap_{gasGapIndex},stationEta_multiplet_{multipletIndex}_gasgap_{gasGapIndex};stationEtaSectorStripHitMap_multiplet_{multipletIndex}_gasgap_{gasGapIndex}'
            weightStationEtaSectorWireGroupHitMap = f'wireGroupHitLayers_multiplet_{multipletIndex}_gasgap_{gasGapIndex}'
            sTgcLayerGroup.defineHistogram(varStationEtaSectorWireGroupHitMap, type = 'TH2F', title = titleStationEtaSectorWireGroupHitMap, path = 'stationEtaSectorWireGroupHitMap', xbins = sectorMax, xmin = 1., xmax = float(sectorMax + 1), ybins = 2*stationEtaMax + 2, ymin = -float(stationEtaMax + 1), ymax = float(stationEtaMax + 1), opt = 'kAlwaysCreate', weight = weightStationEtaSectorWireGroupHitMap)
            
            titleWireGroupOccupancyPerQuad  = f'quads vs strip occupancy: multiplet {multipletIndex} gasgap {gasGapIndex}; wire group number; station eta; hits'
            varWireGroupOccupancyPerQuad    = f'wireGroupNumber_multiplet_{multipletIndex}_gasgap_{gasGapIndex},stationEta_multiplet_{multipletIndex}_gasgap_{gasGapIndex};wireGroupOccupancyPerQuad_multiplet_{multipletIndex}_gasgap_{gasGapIndex}'
            weightWireGroupOccupancyPerQuad = f'wireGroupHit_multiplet_{multipletIndex}_gasgap_{gasGapIndex}'
            sTgcLayerGroup.defineHistogram(varWireGroupOccupancyPerQuad, type = 'TH2F', title = titleWireGroupOccupancyPerQuad, path = 'wireGroupOccupancyPerQuad', xbins = 58*16, xmin = 0., xmax = 58*16., ybins = 2*stationEtaMax + 2, ymin = -float(stationEtaMax + 1), ymax = float(stationEtaMax + 1),  opt = 'kAlwaysCreate', weight = weightWireGroupOccupancyPerQuad)
            
    acc = helper.result()
    result.merge(acc)
    return result
if __name__=='__main__':
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    import argparse
    
    # If you want to run the package in a standalone way you have to execute the command: 
    # python3 -m StgcRawDataMonitoring.StgcMonitorAlgorithm --events NumberOfEvents --samples PathToInputSamples --output OutputROOTFile
    # in the run folder
    
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
