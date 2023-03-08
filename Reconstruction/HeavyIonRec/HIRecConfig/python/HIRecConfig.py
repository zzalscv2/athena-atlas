# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator



def HIRecCfg(flags):
    '''Configures entire HI reconstruction'''
    acc = ComponentAccumulator()

    if flags.HeavyIon.doGlobal:
        from HIGlobal.HIGlobalConfig import HIGlobalRecCfg
        acc.merge(HIGlobalRecCfg(flags))

    if flags.HeavyIon.doJet:
        from HIJetRec.HIJetRecConfigCA import HIJetRecCfg
        acc.merge(HIJetRecCfg(flags))    
     #   pass

    if flags.HeavyIon.doEgamma:
        #        acc.merge(HIEgammaCfg(flags))
        pass
    # ... to add more
    return acc




def HIJetRecCfg(flags):
    acc = ComponentAccumulator()    
    return acc

def HIEgammaCfg():
    acc = ComponentAccumulator()
    # this fragment should pull in egamma reco: i.e. like here: 
    from egammaConfig.egammaSteeringConfig import EGammaSteeringCfg
    acc.merge(EGammaSteeringCfg(flags))
    # add here HI specific parts
    
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    
    flags.Input.Files = [defaultTestFiles.d + "/RecJobTransformTests/data18_hi.00367384.physics_HardProbes.daq.RAW._lb0145._SFO-8._0001.data"]
    
    flags.Exec.MaxEvents=5
    flags.Exec.SkipEvents=0
    flags.Concurrency.NumThreads=1
    flags.Output.doWriteAOD = True
    flags.Output.AODFileName = "myAOD.pool.root"
    flags.IOVDb.GlobalTag = "CONDBR2-BLKPA-RUN2-09" # "CONDBR2-BLKPA-2022-09"
    flags.GeoModel.AtlasVersion = "ATLAS-R2-2016-01-00-01" # "ATLAS-R3S-2021-03-01-00"
    flags.Trigger.triggerConfig = "DB"
    flags.Reco.EnableHI = True

    flags.HeavyIon.Jet.doTrackJetSeed = True
    flags.HeavyIon.doJet=True
    flags.HeavyIon.doGlobal=True

    flags.fillFromArgs() # enable unit tests to switch only parts of reco such as: 
                         ### python -m HIRecConfig.HIRecConfig HeavyIon.doGlobal="False" GeoModel.AtlasVersion="ATLAS-R3S-2021-03-01-00"
    
    flags.lock()

    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    acc.merge(ByteStreamReadCfg(flags))
    if flags.Reco.EnableBeamSpotDecoration:
        from xAODEventInfoCnv.EventInfoBeamSpotDecoratorAlgConfig import EventInfoBeamSpotDecoratorAlgCfg
        acc.merge(EventInfoBeamSpotDecoratorAlgCfg(flags))
    from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
    acc.merge(InDetTrackRecoCfg(flags))
    
    acc.merge(HIRecCfg(flags))
    

    from AthenaPoolCnvSvc.PoolWriteConfig import PoolWriteCfg
    acc.merge(PoolWriteCfg(flags))
    
    acc.printConfig(withDetails=True, summariseProps=True)
    
    flags.dump()


    import sys
    sys.exit(acc.run().isFailure())
