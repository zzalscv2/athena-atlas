# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
###############################################################
#
# Write the tracking geometry as a obj and json files.
#
###############################################################

##########################################################################
# start from scratch with component accumulator

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from ActsGeometry.ActsGeometryConfig import ActsAlignmentCondAlgCfg
from ActsGeometry.ActsGeometryConfig import ActsTrackingGeometryToolCfg
from ActsGeometry.ActsGeometryConfig import ActsMaterialJsonWriterToolCfg
from ActsGeometry.ActsGeometryConfig import ActsObjWriterToolCfg

def ActsWriteTrackingGeometryCfg(flags, name="ActsWriteTrackingGeometry", **kwargs):

  result = ComponentAccumulator()

  acc = ActsTrackingGeometryToolCfg(flags)
  result.merge(acc)
  ActsMaterialJsonWriterTool = ActsMaterialJsonWriterToolCfg(OutputFile = "geometry-maps.json",
                                                             processSensitives = False,
                                                             processNonMaterial = True)

  kwargs["MaterialJsonWriterTool"] = ActsMaterialJsonWriterTool.getPrimary()                                                           
  result.merge(ActsMaterialJsonWriterTool)

  ActsObjWriterTool = ActsObjWriterToolCfg(OutputDirectory = "obj",
                                           SubDetectors = ["Pixel", "SCT", "TRT"])
 
  kwargs["ObjWriterTool"] = ActsObjWriterTool.getPrimary()     
  result.merge(ActsObjWriterTool)

  ActsWriteTrackingGeometry = CompFactory.ActsWriteTrackingGeometry
  alg = ActsWriteTrackingGeometry(name, **kwargs)
  result.addEventAlgo(alg)

  return result

if "__main__" == __name__:
  from AthenaCommon.Logging import log
  from AthenaCommon.Constants import VERBOSE
  from AthenaConfiguration.AllConfigFlags import initConfigFlags
  from AthenaConfiguration.MainServicesConfig import MainServicesCfg

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

  flags.Concurrency.NumThreads = 1
  flags.Concurrency.NumConcurrentEvents = 1

  flags.lock()
  flags.dump()

  cfg = MainServicesCfg(flags)

  from BeamPipeGeoModel.BeamPipeGMConfig import BeamPipeGeometryCfg
  cfg.merge(BeamPipeGeometryCfg(flags))

  alignCondAlgCfg = ActsAlignmentCondAlgCfg(flags)

  cfg.merge(alignCondAlgCfg)

  alg = ActsWriteTrackingGeometryCfg(flags,
                                     OutputLevel=VERBOSE)

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

  cfg.run(1)
