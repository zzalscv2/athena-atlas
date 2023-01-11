#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def InDetGeometryCfg (flags):
    acc = ComponentAccumulator()

    if flags.Detector.GeometryPixel:
        from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
        acc.merge(PixelReadoutGeometryCfg( flags ))

    if flags.Detector.GeometrySCT:
        from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
        acc.merge(SCT_ReadoutGeometryCfg( flags ))
    
    if flags.Detector.GeometryTRT:
        from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
        acc.merge(TRT_ReadoutGeometryCfg( flags ))
    
    if flags.Detector.GeometryPixel or flags.Detector.GeometrySCT or flags.Detector.GeometryTRT:
        from InDetServMatGeoModel.InDetServMatGeoModelConfig import InDetServiceMaterialCfg
        acc.merge(InDetServiceMaterialCfg( flags ))

    if flags.Detector.GeometryITkPixel:
        from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
        acc.merge(ITkPixelReadoutGeometryCfg( flags ))

    if flags.Detector.GeometryITkStrip:
        from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
        acc.merge(ITkStripReadoutGeometryCfg( flags ))

    if flags.Detector.GeometryPLR:
        from PLRGeoModelXml.PLR_GeoModelConfig import PLR_ReadoutGeometryCfg
        acc.merge(PLR_ReadoutGeometryCfg( flags ))

    return acc


if __name__ == "__main__":
  from AthenaCommon.Logging import log
  from AthenaCommon.Constants import DEBUG
  from AthenaConfiguration.AllConfigFlags import initConfigFlags
  flags = initConfigFlags()

  from AthenaConfiguration.MainServicesConfig import MainServicesCfg
  from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
  # Set up logging and new style config
  log.setLevel(DEBUG)
  from AthenaConfiguration.TestDefaults import defaultTestFiles
  # Provide MC input
  flags.Input.Files = defaultTestFiles.HITS_RUN2
  flags.IOVDb.GlobalTag = "OFLCOND-MC16-SDR-16"
  flags.GeoModel.Align.Dynamic = False
  # Provide data input
  # from AthenaConfiguration.TestDefaults import defaultTestFiles
  # flags.Input.Files = defaultTestFiles.AOD
  # flags.GeoModel.Align.Dynamic = True
  flags.lock()
  # Construct ComponentAccumulator
  acc = MainServicesCfg(flags)
  acc.merge(PoolReadCfg(flags))
  acc.merge(InDetGeometryCfg(flags)) # FIXME This sets up the whole ID geometry would be nicer just to set up min required
  #acc.getService("StoreGateSvc").Dump=True
  acc.getService("ConditionStore").Dump=True
  acc.printConfig(withDetails=True)
  f=open('InDetGMCfg2.pkl','wb')
  acc.store(f)
  f.close()
  flags.dump()
  # Execute and finish
  acc.run(maxEvents=3)
