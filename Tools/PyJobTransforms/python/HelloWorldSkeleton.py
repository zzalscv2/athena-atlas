# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import sys

from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from AthExHelloWorld.HelloWorldConfig import HelloWorldCfg
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

# temporarily force no global config flags
from AthenaConfiguration import AllConfigFlags
del AllConfigFlags.ConfigFlags


def fromRunArgs(runArgs):
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    commonRunArgsToFlags(runArgs, flags)

    processPreInclude(runArgs, flags)
    processPreExec(runArgs, flags)

    if '_ATHENA_GENERIC_INPUTFILE_NAME_' in flags.Input.Files:
        flags.Input.Files = []

    # To respect --athenaopts 
    flags.fillFromArgs()

    flags.lock()

    cfg=MainServicesCfg(flags)
    cfg.merge(HelloWorldCfg(flags))

    processPostInclude(runArgs, flags, cfg)
    processPostExec(runArgs, flags, cfg)

    # Run the final accumulator
    sc = cfg.run()
    sys.exit(not sc.isSuccess())
