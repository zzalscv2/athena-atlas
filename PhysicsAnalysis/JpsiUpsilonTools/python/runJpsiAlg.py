# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration


if __name__ == "__main__":
    from AthenaCommon.Configurable import Configurable
    Configurable.configurableRun3Behavior = 1

    # import the flags and set them
    from AthenaConfiguration.AllConfigFlags import ConfigFlags

    ConfigFlags.Exec.MaxEvents = 150

    # use one of the predefined files
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    ConfigFlags.Input.Files = defaultTestFiles.AOD_MC
    ConfigFlags.Input.isMC=True
    ConfigFlags.fillFromArgs()

    # lock the flags
    ConfigFlags.lock()
    #ConfigFlags.dump()

    # create basic infrastructure
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(ConfigFlags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(ConfigFlags)) #!!!!! THIS IS IMPORTANT
    # add the algorithm to the configuration
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import JpsiAlgCfg
    acc.merge(JpsiAlgCfg(ConfigFlags, "JpsiAlgTest"))

    # debug printout
    acc.printConfig(withDetails=True, summariseProps=True)

    # run the job
    status = acc.run()

    # report the execution status (0 ok, else error)
    import sys
    sys.exit(not status.isSuccess())