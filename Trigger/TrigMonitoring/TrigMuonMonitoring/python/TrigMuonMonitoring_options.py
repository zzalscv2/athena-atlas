#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration


from TrigMuonMonitoring.TrigMuonMonitoringConfig import TrigMuonMonConfig



if __name__=='__main__':     
    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    nightly = '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CommonInputs/'
    aodfile = 'data16_13TeV.00311321.physics_Main.recon.AOD.r9264/AOD.11038520._000001.pool.root.1'
    flags = initConfigFlags()
    flags.Input.Files = [nightly+aodfile]
    flags.Input.isMC = True
    flags.Output.HISTFileName = 'HIST.root'

    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))
    muonMonitorAcc = TrigMuonMonConfig(flags)
    cfg.merge(muonMonitorAcc)

    cfg.printConfig(withDetails=False)
    cfg.run()
