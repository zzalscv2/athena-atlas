# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
###############################################################
#
# Map material from a Geantino scan onto the surfaces and 
# volumes of the detector to creat a material map.
#
###############################################################


##########################################################################
# start from scratch with component accumulator

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from ActsGeometry.ActsGeometryConfig import ActsMaterialStepConverterToolCfg
from ActsGeometry.ActsGeometryConfig import ActsSurfaceMappingToolCfg, ActsVolumeMappingToolCfg
from ActsGeometry.ActsGeometryConfig import ActsMaterialJsonWriterToolCfg

from ActsGeometry.ActsGeometryConfig import ActsAlignmentCondAlgCfg

def ActsMaterialMappingCfg(flags, name = "ActsMaterialMapping", **kwargs):
  result = ComponentAccumulator()

  MaterialStepConverterTool = ActsMaterialStepConverterToolCfg()
  kwargs["MaterialStepConverterTool"] = MaterialStepConverterTool.getPrimary()   
  result.merge(MaterialStepConverterTool)

  ActsSurfaceMappingTool = ActsSurfaceMappingToolCfg(flags)
  kwargs["SurfaceMappingTool"] = ActsSurfaceMappingTool.getPrimary()   
  result.merge(ActsSurfaceMappingTool)

  ActsVolumeMappingTool = ActsVolumeMappingToolCfg(flags)
  kwargs["VolumeMappingTool"] = ActsVolumeMappingTool.getPrimary()
  result.merge(ActsVolumeMappingTool)

  ActsMaterialJsonWriterTool = ActsMaterialJsonWriterToolCfg(OutputFile = "material-maps.json",
                                                            processSensitives = False,
                                                            processNonMaterial = False)
                                                            
  kwargs["MaterialJsonWriterTool"] = ActsMaterialJsonWriterTool.getPrimary()   
  result.merge(ActsMaterialJsonWriterTool)

  ActsMaterialMapping = CompFactory.ActsMaterialMapping
  alg = ActsMaterialMapping(name, **kwargs)
  result.addEventAlgo(alg)

  return result

if "__main__" == __name__:
  from AthenaCommon.Logging import log
  from AthenaCommon.Constants import INFO
  from AthenaConfiguration.AllConfigFlags import initConfigFlags
  from AthenaConfiguration.MainServicesConfig import MainServicesCfg
  from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
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
  flags.Acts.TrackingGeometry.MaterialSource = "geometry-maps.json"
  # flags.Acts.TrackingGeometry.MaterialSource = "/eos/project-a/acts/public/MaterialMaps/ATLAS/geometry-maps.json"
  flags.Concurrency.NumThreads = 1
  flags.Concurrency.NumConcurrentEvents = 1

  flags.lock()
  flags.dump()

  cfg = MainServicesCfg(flags)

  cfg.merge(ActsMaterialTrackWriterSvcCfg(flags,
                                          "ActsMaterialTrackWriterSvc",
                                          "MaterialTracks_mapping.root"))

  cfg.merge(PoolReadCfg(flags))
  eventSelector = cfg.getService("EventSelector")
  eventSelector.InputCollections = ["MaterialStepFile.root"]

  from BeamPipeGeoModel.BeamPipeGMConfig import BeamPipeGeometryCfg
  cfg.merge(BeamPipeGeometryCfg(flags))

  alignCondAlgCfg = ActsAlignmentCondAlgCfg(flags)

  cfg.merge(alignCondAlgCfg)

  alg = ActsMaterialMappingCfg(flags,
                               OutputLevel=INFO,
                               mapSurfaces = True,
                               mapVolumes = True)

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

  cfg.run(80000)
