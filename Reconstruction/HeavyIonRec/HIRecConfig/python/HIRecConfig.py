# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Logging import logging
__log = logging.getLogger('HIRecConfig')


def HIRecCfg(flags):
    '''Configures entire HI reconstruction'''
    acc = ComponentAccumulator()
    
    
    if flags.HeavyIon.doGlobal:
        from HIGlobal.HIGlobalConfig import HIGlobalRecCfg
        acc.merge(HIGlobalRecCfg(flags))

    if flags.HeavyIon.doJet:
        from HIJetRec.HIJetRecConfigCA import HIJetRecCfg
        acc.merge(HIJetRecCfg(flags))
    
    return acc



if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

# testing for Run 2:
    # flags.Input.Files = [defaultTestFiles.d + "/RecJobTransformTests/data18_hi.00367384.physics_HardProbes.daq.RAW._lb0145._SFO-8._0001.data"]
    # flags.IOVDb.GlobalTag = "CONDBR2-BLKPA-RUN2-09"
    # flags.GeoModel.AtlasVersion =  "ATLAS-R2-2016-01-00-01" 

# testing for Run 3:
    flags.Input.Files = [defaultTestFiles.d + "/RecJobTransformTests/data22_hi/RAWFiles/data22_hi.00440101.physics_MinBias.daq.RAW/data22_hi.00440101.physics_MinBias.daq.RAW._lb0214._SFO-11._0001.data"]
    flags.IOVDb.GlobalTag = "CONDBR2-BLKPA-2022-09" 
    flags.GeoModel.AtlasVersion = "ATLAS-R3S-2021-03-01-00"

# set more flags:
    flags.Exec.MaxEvents=5
    flags.Exec.SkipEvents=0
    flags.Concurrency.NumThreads=1
    flags.Output.doWriteAOD = True
    flags.Output.AODFileName = "myAOD.pool.root"
    flags.Output.doWriteESD = True
    flags.Output.ESDFileName = "myESD.pool.root"
    
    flags.Trigger.triggerConfig = "DB"
    flags.Reco.EnableHI = True
    
    flags.HeavyIon.Jet.doTrackJetSeed = True
    flags.HeavyIon.doJet=True
    flags.HeavyIon.doGlobal=True
    
    flags.fillFromArgs() # enable unit tests to switch only parts of reco such as (note the absence of spaces around equal sign): 
                         ### python -m HIRecConfig.HIRecConfig HeavyIon.doGlobal="False" GeoModel.AtlasVersion="ATLAS-R3S-2021-03-01-00"
    
    flags.lock()

    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    acc.merge(ByteStreamReadCfg(flags))
    if flags.Reco.EnableBeamSpotDecoration:
        from xAODEventInfoCnv.EventInfoBeamSpotDecoratorAlgConfig import EventInfoBeamSpotDecoratorAlgCfg
        acc.merge(EventInfoBeamSpotDecoratorAlgCfg(flags))
    if flags.HeavyIon.redoTracking:
        from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
        acc.merge(InDetTrackRecoCfg(flags))
    if flags.HeavyIon.redoEgamma:
        from egammaConfig.egammaSteeringConfig import EGammaSteeringCfg
        acc.merge(EGammaSteeringCfg(flags))
    
    acc.merge(HIRecCfg(flags))
    

    from AthenaPoolCnvSvc.PoolWriteConfig import PoolWriteCfg
    acc.merge(PoolWriteCfg(flags))
    
# debug output level can be switched for particular algorithms:
    # from AthenaCommon.Constants import DEBUG
    # acc.foreach_component("*HISubtractedCellMakerTool*").OutputLevel=DEBUG
    # acc.foreach_component("*HICellCopyTool*").OutputLevel=DEBUG

    acc.printConfig(withDetails=True, summariseProps=True)
    
    flags.dump()
    

    import sys
    sys.exit(acc.run().isFailure())
