# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsPixelClusterAnalysisAlgCfg(flags, name="ActsPixelClusterAnalysisAlg", **kwargs):
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    result = ITkPixelReadoutGeometryCfg(flags)
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, 'ActsClusterAnalysisAlgCfg')

    kwargs.setdefault("ClusterContainerKey", "ITkPixelClusters")
    monitoringAlgorithm = helper.addAlgorithm(CompFactory.ActsTrk.PixelClusterAnalysisAlg, name, **kwargs)
    monitoringGroup = helper.addGroup(monitoringAlgorithm, 'ActsClusterAnalysisAlg', '/ActsAnalysis/')

    monitoringGroup.defineHistogram('globalZ,perp;h_globalZR', title="h_globalZR; z [mm]; r [mm]", type="TH2F", path="PixelClusters",
                                    xbins=1500, xmin=-3000, xmax=3000,
                                    ybins=400, ymin=0, ymax=400)
    monitoringGroup.defineHistogram('eta;h_etaCluster', title="h_etaCluster; cluster #eta", type="TH1F", path="PixelClusters",
                                    xbins=100, xmin=-5, xmax=5)
    
    monitoringGroup.defineTree('barrelEndcap,layerDisk,phiModule,etaModule,isInnermost,isNextToInnermost,eta,globalX,globalY,globalZ,perp,localX,localY,localCovXX,localCovYY,sizeX,sizeY,widthY;PixelClusters',
                               path='ntuples',
                               treedef='barrelEndcap/vector<int>:layerDisk/vector<int>:phiModule/vector<int>:etaModule/vector<int>:isInnermost/vector<int>:isNextToInnermost/vector<int>:eta/vector<double>:globalX/vector<float>:globalY/vector<float>:globalZ/vector<float>:perp/vector<float>:localX/vector<float>:localY/vector<float>:localCovXX/vector<float>:localCovYY/vector<float>:sizeX/vector<int>:sizeY/vector<int>:widthY/vector<float>')

    result.merge(helper.result())
    return result


def ActsStripClusterAnalysisAlgCfg(flags, name="ActsStripClusterAnalysisAlg", **kwargs):
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    result = ITkStripReadoutGeometryCfg(flags)
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, 'ActsClusterAnalysisAlgCfg')

    kwargs.setdefault("ClusterContainerKey", "ITkStripClusters")
    monitoringAlgorithm = helper.addAlgorithm(CompFactory.ActsTrk.StripClusterAnalysisAlg, name, **kwargs)
    monitoringGroup = helper.addGroup(monitoringAlgorithm, 'ActsClusterAnalysisAlg', '/ActsAnalysis/')

    monitoringGroup.defineHistogram('globalZ,perp;h_globalZR', title="h_globalZR; z [mm]; r [mm]", type="TH2F", path="StripClusters",
                                    xbins=1500, xmin=-3000, xmax=3000,
                                    ybins=400, ymin=300, ymax=1100)    
    monitoringGroup.defineHistogram('eta;h_etaCluster', title="h_etaCluster; cluster #eta", type="TH1F", path="StripClusters",
                                    xbins=100, xmin=-5, xmax=5)

    monitoringGroup.defineTree('barrelEndcap,layerDisk,phiModule,etaModule,sideModule,eta,globalX,globalY,globalZ,perp,localX,localCovXX,sizeX;StripClusters', 
                               path='ntuples', 
                               treedef='barrelEndcap/vector<int>:layerDisk/vector<int>:phiModule/vector<int>:etaModule/vector<int>:sideModule/vector<int>:eta/vector<double>:globalX/vector<float>:globalY/vector<float>:globalZ/vector<float>:perp/vector<float>:localX/vector<float>:localCovXX/vector<float>:sizeX/vector<int>')

    result.merge(helper.result())
    return result

def ActsBaseSpacePointAnalysisAlgCfg(flags,
                                     name,
                                     histoPath,
                                     ntupleName,
                                     **kwargs):
    isPixel = 'Pixel' in name
    perp_min = 0 if isPixel else 300
    perp_max = 400 if isPixel else 1100

    acc = ComponentAccumulator()
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, 'ActsSpacePointAnalysisAlgCfg')
    
    monitoringAlgorithm = helper.addAlgorithm(CompFactory.ActsTrk.SpacePointAnalysisAlg, name, **kwargs)
    monitoringGroup = helper.addGroup(monitoringAlgorithm, 'ActsSpacePointAnalysisAlg', '/ActsAnalysis/')
    

    monitoringGroup.defineHistogram('Nsp;h_Nsp', title="Number of Space Points;N;Entries", type="TH1I", path=f"{histoPath}",
                                    xbins=100, xmin=0, xmax=0)

    monitoringGroup.defineHistogram('globalX,globalY;h_globalXY', title="h_globalXY; x [mm]; y [mm]", type="TH2F", path=f"{histoPath}",
                                    xbins=800, xmin=-perp_max, xmax=perp_max,
                                    ybins=800, ymin=-perp_max, ymax=perp_max)
    monitoringGroup.defineHistogram('globalZ,perp;h_globalZR', title="h_globalZR; z [mm]; r [mm]", type="TH2F", path=f"{histoPath}",
                                    xbins=1500, xmin=-3000, xmax=3000,
                                    ybins=400, ymin=perp_min, ymax=perp_max)
    monitoringGroup.defineHistogram('eta;h_etaSpacePoint', title="h_etaSpacePoint; space point #eta", type="TH1F", path=f"{histoPath}",
                                    xbins=100, xmin=-5, xmax=5)

    monitoringGroup.defineTree(f'barrelEndcap,layerDisk,phiModule,etaModule,sideModule,isInnermost,isNextToInnermost,isOverlap,eta,globalX,globalY,globalZ,perp,globalCovR,globalCovZ;{ntupleName}',
                               path='ntuples',
                               treedef='barrelEndcap/vector<int>:layerDisk/vector<int>:phiModule/vector<int>:etaModule/vector<int>:sideModule/vector<int>:isInnermost/vector<int>:isNextToInnermost/vector<int>:isOverlap/vector<int>:eta/vector<double>:globalX/vector<double>:globalY/vector<double>:globalZ/vector<double>:perp/vector<double>:globalCovR/vector<double>:globalCovZ/vector<double>')

    acc.merge(helper.result())
    return acc

def ActsPixelSpacePointAnalysisAlgCfg(flags, name="ActsPixelSpacePointAnalysisAlg", **kwargs):
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    result = ITkPixelReadoutGeometryCfg(flags)

    kwargs.setdefault("SpacePointContainerKey", "ITkPixelSpacePoints")
    kwargs.setdefault("UsePixel", True)
    kwargs.setdefault("UseOverlap", False)

    result.merge(ActsBaseSpacePointAnalysisAlgCfg(flags, 
                                                  name = name,
                                                  histoPath = "PixelSpacePoints",
                                                  ntupleName = "PixelSpacePoints",
                                                  **kwargs))
    return result


def ActsStripSpacePointAnalysisAlgCfg(flags, name="ActsStripSpacePointAnalysisAlg", **kwargs):
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    result = ITkStripReadoutGeometryCfg(flags)

    kwargs.setdefault("SpacePointContainerKey", "ITkStripSpacePoints")
    kwargs.setdefault("UsePixel", False)
    kwargs.setdefault("UseOverlap", False)

    result.merge(ActsBaseSpacePointAnalysisAlgCfg(flags,
                                                  name = name,
                                                  histoPath = "StripSpacePoints",
                                                  ntupleName = "StripSpacePoints",
                                                  **kwargs))
    return result


def ActsStripOverlapSpacePointAnalysisAlgCfg(flags, name="ActsStripOverlapSpacePointAnalysisAlg", **kwargs):
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    result = ITkStripReadoutGeometryCfg(flags)

    kwargs.setdefault("SpacePointContainerKey", "ITkStripOverlapSpacePoints")
    kwargs.setdefault("UsePixel", False)
    kwargs.setdefault("UseOverlap", True)

    result.merge(ActsBaseSpacePointAnalysisAlgCfg(flags,
                                                  name = name,
                                                  histoPath = "StripOverlapSpacePoints",
                                                  ntupleName = "StripOverlapSpacePoints",
                                                  **kwargs))
    return result


def ActsBaseSeedAnalysisAlgCfg(flags, 
                               name,
                               histoPath,
                               ntupleName,
                               **kwargs):
    acc = ComponentAccumulator()

    isPixel = 'Pixel' in name
    perp_min = 0 if isPixel else 300
    perp_max = 400 if isPixel else 1100

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags,'ActsSeedAnalysisAlgCfg')

    kwargs.setdefault('InputSeedCollection', 'ITkPixelSeeds')

    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
    geoTool = acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags))
    acc.addPublicTool(geoTool)
    
    # ATLAS Converter Tool
    from ActsConfig.ActsEventCnvConfig import ActsToTrkConverterToolCfg
    converterTool = acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags))
    
    # Track Param Estimation Tool
    from ActsConfig.ActsTrackParamsEstimationConfig import ActsTrackParamsEstimationToolCfg
    trackEstimationTool = acc.popToolsAndMerge(ActsTrackParamsEstimationToolCfg(flags))
    
    kwargs.setdefault('TrackingGeometryTool', acc.getPublicTool(geoTool.name)) # PublicToolHandle
    kwargs.setdefault('ATLASConverterTool', converterTool)
    kwargs.setdefault('TrackParamsEstimationTool', trackEstimationTool)

    monitoringAlgorithm = helper.addAlgorithm(CompFactory.ActsTrk.SeedAnalysisAlg, name, **kwargs)
    monitoringGroup = helper.addGroup(monitoringAlgorithm, 'ActsSeedAnalysisAlg', 'ActsAnalysis')

    monitoringGroup.defineHistogram('Nseed', title='Number of Seeds;N;Entries', type='TH1I', path=f'{histoPath}',
                                    xbins=100, xmin=0, xmax=0)

    monitoringGroup.defineHistogram('z1,r1;zr1', title='Bottom SP - Z coordinate vs R;z [mm];r [mm]', type='TH2F', path=f'{histoPath}',
                                    xbins=1500, xmin=-3000, xmax=3000,
                                    ybins=400, ymin=perp_min, ymax=perp_max)
    monitoringGroup.defineHistogram('z2,r2;zr2', title='Middle SP - Z coordinate vs R;z [mm];r [mm]', type='TH2F', path=f'{histoPath}',
                                    xbins=1500, xmin=-3000, xmax=3000,
                                    ybins=400, ymin=perp_min, ymax=perp_max)
    monitoringGroup.defineHistogram('z3,r3;zr3', title='Top SP - Z coordinate vs R;z [mm];r [mm]', type='TH2F', path=f'{histoPath}',
                                    xbins=1500, xmin=-3000, xmax=3000,
                                    ybins=400, ymin=perp_min, ymax=perp_max)

    monitoringGroup.defineHistogram('x1;x1', title='Bottom SP - x coordinate;x [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=-perp_max, xmax=perp_max)
    monitoringGroup.defineHistogram('y1;y1', title='Bottom SP - y coordinate;y [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=-perp_max, xmax=perp_max)
    monitoringGroup.defineHistogram('z1;z1', title='Bottom SP - z coordinate;z [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=-3000, xmax=3000)
    monitoringGroup.defineHistogram('r1;r1', title='Bottom SP - radius coordinate;r [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=perp_min, xmax=perp_max)
    
    monitoringGroup.defineHistogram('x2;x2', title='Middle SP - x coordinate;x [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=-perp_max, xmax=perp_max)
    monitoringGroup.defineHistogram('y2;y2', title='Middle SP - y coordinate;y [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=-perp_max, xmax=perp_max)
    monitoringGroup.defineHistogram('z2;z2', title='Middle SP - z coordinate;z [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=-3000, xmax=3000)
    monitoringGroup.defineHistogram('r2;r2', title='Middle SP - radius coordinate;r [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=perp_min, xmax=perp_max)
    
    monitoringGroup.defineHistogram('x3;x3', title='Top SP - x coordinate;x [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=-perp_max, xmax=perp_max)
    monitoringGroup.defineHistogram('y3;y3', title='Top SP - y coordinate;y [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=-perp_max, xmax=perp_max)
    monitoringGroup.defineHistogram('z3;z3', title='Top SP - z coordinate;z [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=-3000, xmax=3000)
    monitoringGroup.defineHistogram('r3;r3', title='Top SP - radius coordinate;r [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=perp_min, xmax=perp_max)
    
    if ntupleName == 'PixelSeeds':
        monitoringGroup.defineHistogram('pt;pT', title='Pt;Pt;Entries;', type='TH1F', path=f'{histoPath}',
                                        xbins=100, xmin=0, xmax=100)
        monitoringGroup.defineHistogram('d0;d0', title='d0;d0 [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=0, xmax=2)
        monitoringGroup.defineHistogram('eta;Eta', title='Pseudo-Rapidity;Pseudo-Rapidity;Entries;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=0, xmax=4.5)
        monitoringGroup.defineHistogram('theta;Theta', title='Theta;Theta;Entries;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=0, xmax=1.6)
        monitoringGroup.defineHistogram('penalty;Penalty', title='Penalty;Penalty;Entries;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=0, xmax=200)
        monitoringGroup.defineHistogram('dzdr_b;dzdr_b', title='dzdr_b;;;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=-30, xmax=30)
        monitoringGroup.defineHistogram('dzdr_t;dzdr_t', title='dzdr_t;;;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=-30, xmax=30)
    elif ntupleName == 'StripSeeds':
        monitoringGroup.defineHistogram('pt;pT', title='Pt;Pt;Entries;', type='TH1F', path=f'{histoPath}',
                                        xbins=100, xmin=0, xmax=2300)
        monitoringGroup.defineHistogram('d0;d0', title='d0;d0 [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=0, xmax=120)
        monitoringGroup.defineHistogram('eta;Eta', title='Pseudo-Rapidity;Pseudo-Rapidity;Entries;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=0, xmax=4.5)
        monitoringGroup.defineHistogram('theta;Theta', title='Theta;Theta;Entries;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=0, xmax=1.6)
        monitoringGroup.defineHistogram('penalty;Penalty', title='Penalty;Penalty;Entries;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=0, xmax=20000)
        monitoringGroup.defineHistogram('dzdr_b;dzdr_b', title='dzdr_b;;;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=-6.5, xmax=6.5)
        monitoringGroup.defineHistogram('dzdr_t;dzdr_t', title='dzdr_t;;;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=-6.5, xmax=6.5)
                
    if flags.Tracking.doTruth:
        monitoringGroup.defineHistogram('passed,estimated_eta;EfficiencyEta', title='Efficiency vs eta;eta;Efficiency', type='TEfficiency', path=f'{histoPath}',
                                        xbins=50, xmin=-5, xmax=5)
        monitoringGroup.defineHistogram('passed,estimated_pt;EfficiencyPt', title='Efficiency vs pT;pT [GeV];Efficiency', type='TEfficiency', path=f'{histoPath}',
                                        xbins=30, xmin=0, xmax=120)

    # Tree
    list_variables = "x1,y1,z1,r1,x2,y2,z2,r2,x3,y3,z3,r3,pt,theta,eta,d0,dzdr_b,dzdr_t,penalty,event_number,actual_mu"
    tree_def = "x1/vector<double>:y1/vector<double>:z1/vector<double>:r1/vector<double>:x2/vector<double>:y2/vector<double>:z2/vector<double>:r2/vector<double>:x3/vector<double>:y3/vector<double>:z3/vector<double>:r3/vector<double>\
:pt/vector<float>:theta/vector<float>:eta/vector<float>:d0/vector<float>:dzdr_b/vector<float>:dzdr_t/vector<float>:penalty/vector<float>:event_number/l:actual_mu/F"
    if flags.Tracking.doTruth:
        list_variables += ",truth_barcode,truth_prob"
        tree_def += ":truth_barcode/vector<int>:truth_prob/vector<double>"

    monitoringGroup.defineTree(f'{list_variables};{ntupleName}',
                               path='ntuples',
                               treedef=tree_def )

    acc.merge(helper.result())
    return acc



def ActsPixelSeedAnalysisAlgCfg(flags, name = "ActsPixelSeedAnalysisAlg", **kwargs):
    kwargs.setdefault('InputSeedCollection', 'ITkPixelSeeds')

    if flags.Tracking.doTruth:
        kwargs.setdefault('DetectorElements', 'ITkPixelDetectorElementCollection')
        kwargs.setdefault('ITkClustersTruth', 'PRD_MultiTruthITkPixel')

    return ActsBaseSeedAnalysisAlgCfg(flags, name, histoPath='PixelSeeds', ntupleName='PixelSeeds', **kwargs)


def ActsStripSeedAnalysisAlgCfg(flags, name = "ActsStripSeedAnalysisAlg", **kwargs):
    kwargs.setdefault('InputSeedCollection', 'ITkStripSeeds')
    kwargs.setdefault('UsePixel', False)

    if flags.Tracking.doTruth:
        kwargs.setdefault('DetectorElements', 'ITkStripDetectorElementCollection')
        kwargs.setdefault('ITkClustersTruth', 'PRD_MultiTruthITkStrip')

    return ActsBaseSeedAnalysisAlgCfg(flags, name, histoPath='StripSeeds', ntupleName='StripSeeds', **kwargs)


def ActsBaseEstimatedTrackParamsAnalysisAlgCfg(flags,
                                               name,
                                               histoPath,
                                               ntupleName,
                                               **kwargs):
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags,'ActsEstimatedTrackParamsAnalysisAlgCfg')

    monitoringAlgorithm = helper.addAlgorithm(CompFactory.ActsTrk.EstimatedTrackParamsAnalysisAlg, name, **kwargs)
    monitoringGroup = helper.addGroup(monitoringAlgorithm, 'ActsEstimatedTrackParamsAnalysisAlg', 'ActsAnalysis')

    monitoringGroup.defineHistogram('Nparams', title='Number of Estimated Parameters from Seeds;N;Entries', type='TH1I', path=f'{histoPath}',
                                    xbins=100, xmin=0, xmax=0)

    monitoringGroup.defineTree(f"track_param_pt,track_param_eta,track_param_phi,track_param_loc0,track_param_loc1,track_param_theta,track_param_qoverp,track_param_time,track_param_charge;{ntupleName}",
                               path="ntuples",
                               treedef="track_param_pt/vector<double>:track_param_eta/vector<double>:track_param_phi/vector<double>:track_param_loc0/vector<double>:track_param_loc1/vector<double>:track_param_theta/vector<double>:track_param_qoverp/vector<double>:track_param_time/vector<double>:track_param_charge/vector<int>")

    return helper.result()

def ActsSeedingAlgorithmAnalysisAlgCfg(flags, name="ActsSeedingAlgorithmAnalysis", **kwargs):
    result = ComponentAccumulator()

    MonitoringGroupNames = []

    if "SeedingTools" not in kwargs:
        from InDetConfig.SiSpacePointsSeedToolConfig import ITkSiSpacePointsSeedMakerCfg
        ITkSiSpacePointsSeedMaker = result.popToolsAndMerge(ITkSiSpacePointsSeedMakerCfg(flags))
        ITkSiSpacePointsSeedMaker.maxSize = 1e8
        MonitoringGroupNames.append("ITkSiSpacePointSeedMaker")

        from ActsConfig.ActsSeedingConfig import ActsSiSpacePointsSeedMakerCfg
        # The default Acts pixel seeding tool performs by default a seed selection after the seed finding
        # We have to disable it or a fair comparison with the other seed computations
        from ActsConfig.ActsSeedingConfig import ActsITkPixelSeedingToolCfg
        seedToolPixel = result.popToolsAndMerge(ActsITkPixelSeedingToolCfg(flags, doSeedQualitySelection=False))
        # We then override the pixel seeding tool inside the ActsSiSpacePointsSeedMakerCfg so that we pick this one
        ActsITkSiSpacePointsSeedMaker = result.popToolsAndMerge(ActsSiSpacePointsSeedMakerCfg(flags, SeedToolPixel=seedToolPixel))
        ActsITkSiSpacePointsSeedMaker.doSeedConversion = False
        MonitoringGroupNames.append("ActsITkSiSpacePointSeedMaker")

        from ActsConfig.ActsSeedingConfig import ActsITkPixelOrthogonalSeedingToolCfg
        orthogonal_seeding_tool = result.popToolsAndMerge(ActsITkPixelOrthogonalSeedingToolCfg(flags))
        ActsITkSiSpacePointsSeedMakerOrthogonal = \
          result.popToolsAndMerge(ActsSiSpacePointsSeedMakerCfg(flags,
                                                                name="ActsSiSpacePointsSeedMakerOrthogonal",
                                                                SeedToolPixel=orthogonal_seeding_tool))
        ActsITkSiSpacePointsSeedMakerOrthogonal.doSeedConversion = False
        MonitoringGroupNames.append("ActsOrthogonalITkSiSpacePointSeedMaker")

        from GaudiKernel.GaudiHandles import PrivateToolHandleArray
        kwargs.setdefault("SeedingTools",
                          PrivateToolHandleArray([ITkSiSpacePointsSeedMaker,
                                                  ActsITkSiSpacePointsSeedMaker,
                                                  ActsITkSiSpacePointsSeedMakerOrthogonal]))

    kwargs.setdefault("MonitorNames", MonitoringGroupNames)

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, 'SeedingAlgorithmAnalysisAlgCfg')
    monitoringAlgorithm = helper.addAlgorithm(CompFactory.ActsTrk.SeedingAlgorithmAnalysisAlg, name, **kwargs)

    for groupName in MonitoringGroupNames:
      monitoringGroup = helper.addGroup(monitoringAlgorithm, groupName, '/'+groupName+'/')
      monitoringGroup.defineTree('eventNumber,stripSeedInitialisationTime,stripSeedProductionTime,pixelSeedInitialisationTime,pixelSeedProductionTime,numberPixelSpacePoints,numberStripSpacePoints,numberPixelSeeds,numberStripSeeds;seedInformation',
                                 path='ntuples',
                                 treedef='eventNumber/I:stripSeedInitialisationTime/F:stripSeedProductionTime/F:pixelSeedInitialisationTime/F:pixelSeedProductionTime/F:numberPixelSpacePoints/I:numberStripSpacePoints/I:numberPixelSeeds/I:numberStripSeeds/I')

    result.merge(helper.result())
    return result


def ActsPixelEstimatedTrackParamsAnalysisAlgCfg(flags, name = 'ActsPixelEstimatedTrackParamsAnalysisAlg', **kwargs):
    kwargs.setdefault('InputTrackParamsCollection', 'ITkPixelEstimatedTrackParams')
    return ActsBaseEstimatedTrackParamsAnalysisAlgCfg(flags, name, histoPath = 'PixelEstimatedTrackParams', ntupleName = 'PixelEstimatedTrackParams', **kwargs)


def ActsStripEstimatedTrackParamsAnalysisAlgCfg(flags, name = 'ActsStripEstimatedTrackParamsAnalysisAlg', **kwargs):
    kwargs.setdefault('InputTrackParamsCollection', 'ITkStripEstimatedTrackParams')
    return ActsBaseEstimatedTrackParamsAnalysisAlgCfg(flags, name, histoPath = 'StripEstimatedTrackParams', ntupleName = 'StripEstimatedTrackParams', **kwargs)

def PhysValActsCfg(flags,
                   name: str = 'PhysValActs') -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.ActsTrk.PhysValTool(name=name))
    return acc
    
def ActsClusterAnalysisCfg(flags):
    acc = ComponentAccumulator()
    if flags.Detector.EnableITkPixel:
        acc.merge(ActsPixelClusterAnalysisAlgCfg(flags))
    if flags.Detector.EnableITkStrip:
        acc.merge(ActsStripClusterAnalysisAlgCfg(flags))
    return acc


def ActsSpacePointAnalysisCfg(flags):
    acc = ComponentAccumulator()
    if flags.Detector.EnableITkPixel:
        acc.merge(ActsPixelSpacePointAnalysisAlgCfg(flags))
    if flags.Detector.EnableITkStrip:
        acc.merge(ActsStripSpacePointAnalysisAlgCfg(flags))
        acc.merge(ActsStripOverlapSpacePointAnalysisAlgCfg(flags))
    return acc


def ActsSeedAnalysisCfg(flags):
    acc = ComponentAccumulator()
    if flags.Detector.EnableITkPixel:
        acc.merge(ActsPixelSeedAnalysisAlgCfg(flags))
    if flags.Detector.EnableITkStrip:
        acc.merge(ActsStripSeedAnalysisAlgCfg(flags))
    return acc


def ActsEstimatedTrackParamsAnalysisCfg(flags):
    acc = ComponentAccumulator()
    if flags.Detector.EnableITkPixel:
        acc.merge(ActsPixelEstimatedTrackParamsAnalysisAlgCfg(flags))
    if flags.Detector.EnableITkStrip:
        acc.merge(ActsStripEstimatedTrackParamsAnalysisAlgCfg(flags))
    return acc
