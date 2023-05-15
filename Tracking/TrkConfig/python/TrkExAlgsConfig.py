# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator 
from AthenaConfiguration.ComponentFactory import CompFactory

def PropResultRootWriterSvcCfg(flags, name="PropResultRootWriterSvc", **kwargs) :
  result = ComponentAccumulator()
  result.addService(CompFactory.Trk.PropResultRootWriterSvc(name, **kwargs))
  return result

def ExtrapolatorComparisonTestCfg(flags, name = "ExtrapolatorComparisonTest", **kwargs ) :
  result=ComponentAccumulator()  
  
  if "Extrapolator" not in kwargs:
    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    kwargs.setdefault("Extrapolator", result.popToolsAndMerge(
      AtlasExtrapolatorCfg(flags)))
  
  if "ExtrapolationTool" not in kwargs:
    from ActsConfig.ActsTrkGeometryConfig import ActsExtrapolationToolCfg
    kwargs.setdefault("ExtrapolationTool", result.popToolsAndMerge(
      ActsExtrapolationToolCfg(flags))) # PrivateToolHandle
  
  result.addEventAlgo(CompFactory.Trk.ExtrapolatorComparisonTest(name, **kwargs))
  return result

if __name__=="__main__":
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import VERBOSE
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg    
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    #log.setLevel(VERBOSE)
    
    flags = initConfigFlags()
    # Need some actual input to get flags properly configured
    flags.Input.Files            = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc20_13TeV.361107.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Zmumu.recon.AOD.e3601_s3681_r13167/AOD.27312826._000061.pool.root.1']

    ## Just enable ID for the moment.
    flags.Detector.GeometryBpipe = True
    flags.Detector.GeometryID    = True
    flags.Detector.GeometryPixel = True
    flags.Detector.GeometrySCT   = True
    flags.Detector.GeometryCalo  = False
    flags.Detector.GeometryMuon  = False
    #flags.Detector.GeometryTRT   = True
    flags.TrackingGeometry.MaterialSource = "Input"
    
    # This should run serially for the moment.
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    
    log.debug('Lock config flags now.')
    flags.lock()
    
    flags.dump()

    cfg=MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))
    
    histSvc = CompFactory.THistSvc(Output = ["ExtrapolationStudies DATAFILE='ExtrapolationStudies.root' OPT='RECREATE'"])
    histSvc.OutputLevel=VERBOSE
    cfg.addService( histSvc )
    
    cfg.merge(PropResultRootWriterSvcCfg(flags, 
                                         name="ATLASPropResultRootWriterSvc",
                                         TreeName="ATLAS"))
  
    cfg.merge(PropResultRootWriterSvcCfg(flags, 
                                         name="ACTSPropResultRootWriterSvc",
                                         TreeName="ACTS"))

    cfg.merge(ExtrapolatorComparisonTestCfg(
      flags,
      EventsPerExecute = 1000,
      StartPerigeeMinPt   = 10000,
      StartPerigeeMaxPt   = 10000,
      ReferenceSurfaceRadius = [80],
      ReferenceSurfaceHalfZ  = [500],
      ATLASPropResultRootWriter = cfg.getService("ATLASPropResultRootWriterSvc"), 
      ACTSPropResultRootWriter = cfg.getService("ACTSPropResultRootWriterSvc")))
    
    cfg.printConfig()

    cfg.run(10)
    f=open("ExtrapolatorComparisonTestConfig.pkl","w")
    cfg.store(f)
    f.close()
