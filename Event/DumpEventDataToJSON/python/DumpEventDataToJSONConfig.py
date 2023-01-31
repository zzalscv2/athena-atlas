# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import VERBOSE


def DumpEventDataToJSONAlgCfg(flags, doExtrap=False, **kwargs):
    result = ComponentAccumulator()
    extrapolationEngine = ""
    if doExtrap:
        from AtlasGeoModel.GeoModelConfig import GeoModelCfg
        gmsAcc = GeoModelCfg(flags)
        result.merge(gmsAcc)

        from TrkConfig.AtlasExtrapolationEngineConfig import AtlasExtrapolationEngineCfg
        extrapAcc = AtlasExtrapolationEngineCfg(flags)
        extrapolationEngine = extrapAcc.getPrimary()
        result.merge(extrapAcc)

        kwargs.setdefault('Extrapolator', extrapolationEngine)

    dumpAlg = CompFactory.DumpEventDataToJsonAlg(
        ExtrapolateTrackParticles=doExtrap, **kwargs)
    result.addEventAlgo(dumpAlg)
    return result


if __name__ == "__main__":
    # Run this with python -m DumpEventDataToJSON.DumpEventDataToJSONConfig myESD.pool.root
    from argparse import ArgumentParser
    parser = ArgumentParser()
    parser.add_argument("input",
                        help="Input pool file", default="../q221/myESD.pool.root")
    parser.add_argument("-o", "--output", dest="output", default='Event.json',
                        help="write JSON to FILE", metavar="FILE")
    parser.add_argument("--prependCalib", help="Prepend a calibartion event with some labelled objects at specific eta/phi.",
                        action="store_true")
    args = parser.parse_args()

    print('Running DumpEventDataToJSON on {} and outputting to {}. Prepend calib event is {}'.format(
        args.input, args.output, args.prependCalib))

    from AthenaCommon.Logging import log
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    # Uncomment for debugging
    # from AthenaCommon.Constants import DEBUG
    # log.setLevel(DEBUG)

    flags = initConfigFlags()
    # To run on MC do e.g.
    flags.Input.Files = [args.input]
    # To run on data do e.g.
    # flags.Input.Files = ["../q431/myESD.pool.root"]

    # This should run serially for the moment.
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1

    flags.dump()
    log.debug('Lock config flags now.')
    flags.lock()

    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    if flags.Detector.GeometryBpipe:
        from BeamPipeGeoModel.BeamPipeGMConfig import BeamPipeGeometryCfg
        cfg.merge(BeamPipeGeometryCfg(flags))

    if flags.Detector.GeometryPixel:
        from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
        cfg.merge(PixelReadoutGeometryCfg(flags))

    if flags.Detector.GeometrySCT:
        from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
        cfg.merge(SCT_ReadoutGeometryCfg(flags))

    if flags.Detector.GeometryTRT:
        from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
        cfg.merge(TRT_ReadoutGeometryCfg(flags))

    if flags.Detector.GeometryITkPixel:
        from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
        cfg.merge(ITkPixelReadoutGeometryCfg(flags))

    if flags.Detector.GeometryITkStrip:
        from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
        cfg.merge(ITkStripReadoutGeometryCfg(flags))

    if flags.Detector.GeometryLAr:
        from LArGeoAlgsNV.LArGMConfig import LArGMCfg
        cfg.merge(LArGMCfg(flags))

    if flags.Detector.GeometryTile:
        from TileGeoModel.TileGMConfig import TileGMCfg
        cfg.merge(TileGMCfg(flags))

    if flags.Detector.GeometryMuon:
        from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
        cfg.merge(MuonGeoModelCfg(flags))

    from TrkConfig.AtlasTrackingGeometrySvcConfig import TrackingGeometrySvcCfg
    cfg.merge(TrackingGeometrySvcCfg(flags))

    from TrkConfig.TrackCollectionReadConfig import TrackCollectionReadCfg
    cfg.merge(TrackCollectionReadCfg(flags, 'Tracks'))

    muon_edm_helper_svc = CompFactory.Muon.MuonEDMHelperSvc("MuonEDMHelperSvc")
    cfg.addService(muon_edm_helper_svc)

    # Disable doExtrap if you would prefer not to use the extrapolator.
    topoAcc = DumpEventDataToJSONAlgCfg(
        flags, doExtrap=False, OutputLevel=VERBOSE, DumpTestEvent=args.prependCalib, OutputLocation=args.output,
        CscPrepRawDataKey = "CSC_Clusters" if flags.Detector.EnableCSC else "",
        MMPrepRawDataKey = "MM_Measurements" if flags.Detector.EnableMM else "",
        sTgcPrepRawDataKey = "STGC_Measurements" if flags.Detector.EnablesTGC else "",
        )
    
    cfg.merge(topoAcc)

    cfg.run(2)
    f = open("DumpEventDataToJSONConfig.pkl", "wb")
    cfg.store(f)
    f.close()
