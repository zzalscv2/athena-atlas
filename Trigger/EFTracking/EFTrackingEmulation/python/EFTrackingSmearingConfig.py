
#Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Constants import INFO
from AthenaCommon.Logging import logging
_log = logging.getLogger(__name__)

def EFTrackingSmearingCfg(flags, name = "EFTrackingSmearingAlg", **kwargs):
    # set common parameters
    
    histSvc = CompFactory.THistSvc(Output=["EFTrackingSmearingAlg DATAFILE='EFTrackingSmearingAlg.root' OPT='RECREATE'"])

    result = ComponentAccumulator()    
    result.addService(histSvc) 

    alg = CompFactory.EFTrackingSmearingAlg (
      OutputLevel = INFO,
      OutputTrackParticleContainer = "InDetTrackParticles_smeared_SF"+str(kwargs['smearFactor']),
      InputTrackParticleContainer = kwargs['InputTrackParticle'],
      SmearedTrackEfficiency = kwargs['trackEfficiency'],
      ParameterizedTrackEfficiency = kwargs['parameterizeEfficiency'],
      SmearingScaleFactor = kwargs['smearFactor'],
      InputTracksPtCutGeV = kwargs['trkpTCut'],
      EnableMonitoring = kwargs['EnableMonitoring'],
      RootStreamName = "EFTrackingSmearingAlg",
      RootDirName      = "/EFTSmearing"
      )

    result.addEventAlgo(alg)
    return result





if __name__ == "__main__":
    
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()    
    flags.Input.Files = defaultTestFiles.AOD_RUN3_MC
    flags.Input.isMC=True
    flags.Exec.MaxEvents = 5
    flags.lock()

    # create basic infrastructure
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))
    
    smearer = EFTrackingSmearingCfg("test", trkpTCut=1, smearFactor=1, InputTrackParticle="InDetTrackParticles",
                                    trackEfficiency=1., parameterizeEfficiency=True,
                                    EnableMonitoring=True)
    acc.merge(smearer)    
    acc.wasMerged()
    # debug printout
    acc.printConfig(withDetails=True, summariseProps=True)
   
    # run the job
    status = acc.run()
 
    # report the execution status (0 ok, else error)
    import sys
    sys.exit(not status.isSuccess())
      
