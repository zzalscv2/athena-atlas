# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration


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
    flags.Concurrency.NumThreads=1
    flags.Output.doWriteAOD = True
    flags.Output.AODFileName = "myAOD.pool.root"

    flags.fillFromArgs() # enable unit tests to switch only parts of reco: python -m HIRecConfig.HIRecConfig HeavyIon.doGlobal = 0 and so on

    flags.HeavyIon.Jet.doTrackJetSeed = False # set to false in self-test because indetparticles are reconestructed 

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
    readBSAcc = ByteStreamReadCfg(flags)
    acc.merge(HIRecCfg(flags))

    from AthenaPoolCnvSvc.PoolWriteConfig import PoolWriteCfg
    acc.merge(PoolWriteCfg(flags))

    import sys
    sys.exit(acc.run().isFailure())
