# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
## @brief this function sets up the L1 simulation sequence with the ZDC
## it covers the case of rerunning the L1 on run2 HI data


def L1ZDCSimCfg(flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()
    from ZdcRec.ZdcRecConfig import ZdcRecRun2Cfg
    acc.merge(ZdcRecRun2Cfg(flags))
    from AthenaConfiguration.ComponentFactory import CompFactory
    acc.addEventAlgo(CompFactory.LVL1.TrigT1ZDC(filepath_LUT = "TrigT1ZDC/zdcRun3T1LUT_v1_30_05_2023.json",
                                                EnergyADCScale = 0.4))
    return acc

if __name__ == '__main__':
    import sys
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test/data15_hi.00287843.physics_EnhancedBias.merge.RAW._lb0226._SFO-2._0001.1']
    flags.Common.isOnline=False
    flags.Exec.MaxEvents=100
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents=1
    flags.Scheduler.ShowDataDeps=True
    flags.Scheduler.CheckDependencies=True
    flags.Scheduler.ShowDataFlow=True
    flags.Trigger.enableL1MuonPhase1=True
    flags.Trigger.triggerMenuSetup='Dev_HI_run3_v1'
    flags.Trigger.EDMVersion=3
    flags.Trigger.doZDC=True
    flags.Trigger.L1.doAlfaCtpin=True
    flags.Trigger.enableL1CaloPhase1 = False # FIXME: ATR-27095
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
    acc.merge(ByteStreamReadCfg(flags))

    from TrigConfigSvc.TrigConfigSvcCfg import generateL1Menu
    generateL1Menu(flags)

    from TriggerJobOpts.Lvl1SimulationConfig import Lvl1SimulationCfg
    acc.merge(Lvl1SimulationCfg(flags))
    from AthenaCommon.Constants import DEBUG, INFO
    acc.getEventAlgo("CTPSimulation").OutputLevel=INFO  # noqa: ATL900
    acc.getEventAlgo("LVL1::TrigT1ZDC").OutputLevel=DEBUG


    acc.printConfig(withDetails=True, summariseProps=True, printDefaults=True)
    with open("L1Sim.pkl", "wb") as p:
        acc.store(p)
        p.close()

    sys.exit(acc.run().isFailure())
