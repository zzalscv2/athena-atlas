# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration


if __name__ == "__main__":
    # import the flags and set them
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Exec.MaxEvents = 150

    # use one of the predefined files
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files = defaultTestFiles.AOD_RUN2_MC
    flags.Input.isMC=True
    flags.fillFromArgs()

    # lock the flags
    flags.lock()
    #flags.dump()

    # create basic infrastructure
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags)) #!!!!! THIS IS IMPORTANT
    # add the algorithm to the configuration
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import JpsiAlgCfg
    acc.merge(JpsiAlgCfg(flags, "JpsiAlgTest"))

    # debug printout
    acc.printConfig(withDetails=True, summariseProps=True)

    # run the job
    status = acc.run()

    # report the execution status (0 ok, else error)
    import sys
    sys.exit(not status.isSuccess())
