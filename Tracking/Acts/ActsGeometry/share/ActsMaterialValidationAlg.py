"""
This job options file will run an example extrapolation using the
Acts tracking geometry and the Acts extrapolation toolchain.

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""

# start from scratch with component accumulator

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from ActsGeometry.ActsGeometryConfig import ActsExtrapolationToolCfg
from ActsGeometry.ActsGeometryConfig import ActsAlignmentCondAlgCfg

def ActsExtrapolationAlgCfg(flags, name = "ActsExtrapolationAlg", **kwargs):
  result = ComponentAccumulator()

  if "ExtrapolationTool" not in kwargs:
    extrapTool = ActsExtrapolationToolCfg(flags)
    kwargs["ExtrapolationTool"] = extrapTool.getPrimary()
    result.merge(extrapTool)

  ActsExtrapolationAlg = CompFactory.ActsExtrapolationAlg
  alg = ActsExtrapolationAlg(name, **kwargs)
  result.addEventAlgo(alg)

  return result

if "__main__" == __name__:
  from AthenaCommon.Logging import log
  from AthenaCommon.Constants import INFO
  from AthenaConfiguration.AllConfigFlags import initConfigFlags
  from AthenaConfiguration.MainServicesConfig import MainServicesCfg
  from ActsGeometry.ActsGeometryConfig import ActsMaterialTrackWriterSvcCfg

  flags = initConfigFlags()

  ## Just enable ID for the moment.
  flags.Input.isMC             = True
  flags.GeoModel.AtlasVersion  = "ATLAS-R2-2016-01-00-01"
  flags.IOVDb.GlobalTag        = "OFLCOND-SIM-00-00-00"
  flags.Detector.GeometryBpipe = True
  flags.Detector.GeometryID    = True
  flags.Detector.GeometryPixel = True
  flags.Detector.GeometrySCT   = True
  flags.Detector.GeometryCalo  = True
  flags.Detector.GeometryMuon  = False
  flags.Detector.GeometryTRT   = True
  flags.Acts.TrackingGeometry.MaterialSource = "material-maps.json"
  # flags.Acts.TrackingGeometry.MaterialSource = "/eos/project-a/acts/public/MaterialMaps/ATLAS/material-maps.json"

  flags.Concurrency.NumThreads = 10
  flags.Concurrency.NumConcurrentEvents = 10

  flags.lock()
  flags.dump()

  cfg = MainServicesCfg(flags)

  from BeamPipeGeoModel.BeamPipeGMConfig import BeamPipeGeometryCfg
  cfg.merge(BeamPipeGeometryCfg(flags))

  alignCondAlgCfg = ActsAlignmentCondAlgCfg(flags)

  cfg.merge(alignCondAlgCfg)

  cfg.merge(ActsMaterialTrackWriterSvcCfg(flags,
                                          "ActsMaterialTrackWriterSvc",
                                          "MaterialTracks_mapped.root"))

  print('DEF WRITER : ')
  extrapol = ActsExtrapolationToolCfg(flags,
                                      InteractionMultiScatering = True,
                                      InteractionEloss = True,
                                      InteractionRecord = True)
  cfg.merge(extrapol)
  
  alg = ActsExtrapolationAlgCfg(flags,
                                OutputLevel=INFO,
                                NParticlesPerEvent=int(1e4),
                                EtaRange=[-2.5, 2.5],
                                PtRange=[20, 100],
                                WriteMaterialTracks = True,
                                ExtrapolationTool=extrapol.getPrimary())

  cfg.merge(alg)

  tgSvc = cfg.getService("ActsTrackingGeometrySvc")

  # Service will have removed TRT and Calo
  # We want them enabled for testing
  tgSvc.BuildSubDetectors += [
    "TRT",
    "Calo"
  ]
  # needed to construct the calo geometry in ACTS
  tgSvc.CaloVolumeBuilder = CompFactory.ActsCaloTrackingVolumeBuilder()


  cfg.printConfig()

  log.info("CONFIG DONE")

  cfg.run(100)

