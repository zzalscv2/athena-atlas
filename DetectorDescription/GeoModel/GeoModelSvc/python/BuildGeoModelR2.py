# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Script for testing the initialization of GeoModel descriptions of ATLAS 
# subsystems in Run2 geometry layout

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AtlasGeoModel.GeoModelConfig import GeoModelCfg
from Campaigns.Utils import Campaign

flags = initConfigFlags()

flags.Exec.MaxEvents = 1

flags.Input.isMC = True
flags.Input.MCCampaign = Campaign.Unknown

flags.GeoModel.AtlasVersion='ATLAS-R2-2016-01-00-01'
flags.IOVDb.GlobalTag = "OFLCOND-MC16-SDR-16"

flags.Detector.GeometryBpipe = True
flags.Detector.GeometryPixel = True
flags.Detector.GeometrySCT = True
flags.Detector.GeometryTRT = True
flags.Detector.GeometryLAr = True
flags.Detector.GeometryTile = True
flags.Detector.GeometryMuon = True

flags.LAr.doAlign = False

flags.lock()

cfg = MainServicesCfg(flags)
cfg.merge(GeoModelCfg(flags))

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

if flags.Detector.GeometryLAr:
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    cfg.merge(LArGMCfg(flags))

if flags.Detector.GeometryTile:
    from TileGeoModel.TileGMConfig import TileGMCfg
    cfg.merge(TileGMCfg(flags))

if flags.Detector.GeometryMuon:
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    cfg.merge(MuonGeoModelCfg(flags))

cfg.run()
