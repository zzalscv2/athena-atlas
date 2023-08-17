
#Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Logging import logging
_log = logging.getLogger(__name__)

def EFTrackingSmearingCfg(flags, name = "EFTrackingSmearingAlg", **kwargs):
    # set common parameters
    histSvc = CompFactory.THistSvc(Output=[name +" DATAFILE='"+name+ ".root' OPT='RECREATE'"])

    result = ComponentAccumulator()    
    result.addService(histSvc) 

    alg = CompFactory.EFTrackingSmearingAlg ( name=name,
      OutputLevel = kwargs['OutputLevel'],      
      SmearedTrackEfficiency = kwargs['trackEfficiency'],
      ParameterizedTrackEfficiency = kwargs['parameterizeEfficiency'],
      SmearingScaleFactor = kwargs['smearFactor'],
      SmearTruthParticle = kwargs['smearTruthParticle'],
      InputTracksPtCutGeV = kwargs['trkpTCut'],
      EnableMonitoring = kwargs['EnableMonitoring'],
      RootStreamName = name, 
      RootDirName = "/EFTSmearing/"
      )

    
    if kwargs['smearTruthParticle']:       
      alg.OutputTruthParticleContainer = "TruthParticle_smeared_SF"+str(kwargs['smearFactor'])
      alg.InputTruthParticleContainer = kwargs['InputTruthParticle']
    else:
      alg.OutputTrackParticleContainer = "InDetTrackParticles_smeared_SF"+str(kwargs['smearFactor'])
      alg.InputTrackParticleContainer = kwargs['InputTrackParticle']     
    
    result.addEventAlgo(alg)
    return result


  

if __name__ == "__main__":
    
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaCommon.Constants import INFO
    flags = initConfigFlags()    
    flags.Input.Files = defaultTestFiles.AOD_RUN3_MC
    flags.Input.isMC=True
    flags.Exec.MaxEvents = 5
    flags.lock()

    # create basic infrastructure
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)        
    acc.getService("MessageSvc").debugLimit = 100000
 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))
    
    # needed to access the decorators for the truth particles
    from InDetPhysValMonitoring.InDetPhysValDecorationConfig import AddDecoratorCfg
    acc.merge(AddDecoratorCfg(flags))
    
    TestsmearFactor = 2
    # example to smear the track particles with smearing factor =2 and efficiency 90%
    smearerTrack = EFTrackingSmearingCfg(flags, name="testTrack", trkpTCut=1, smearFactor=TestsmearFactor, InputTrackParticle="InDetTrackParticles",
                                    trackEfficiency=0.9, parameterizeEfficiency=False, smearTruthParticle=False,
                                    EnableMonitoring=True, OutputLevel=INFO)
    acc.merge(smearerTrack)
    
    
    # example to smear the truth particles with smearing factor =2 and efficiency 90%
    smearerTruth = EFTrackingSmearingCfg(flags, name="testTruth", trkpTCut=1, smearFactor=TestsmearFactor, InputTruthParticle="TruthParticles",
                                    trackEfficiency=0.9, parameterizeEfficiency=False, smearTruthParticle=True,
                                    EnableMonitoring=True, OutputLevel=INFO)
    acc.merge(smearerTruth)
    
    
    # validation of the smeared tracks and truth particles
    validationAlg = CompFactory.EFTrackingSmearMonAlg ( name="EFTrakingSmearMonAlg",
      OutputLevel = INFO, 
      InputTrackParticleContainer = "InDetTrackParticles_smeared_SF"+str(TestsmearFactor),
      InputTruthParticleContainer = "TruthParticle_smeared_SF"+str(TestsmearFactor),
      )
    acc.addEventAlgo(validationAlg)
    
    acc.wasMerged()
    
    # below is validation
    acc.printConfig(withDetails=True, summariseProps=True)
    
    
    # run the job
    status = acc.run()

 
    # report the execution status (0 ok, else error)
    import sys
    sys.exit(not status.isSuccess())
      
