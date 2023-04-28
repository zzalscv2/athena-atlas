# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

import sys

from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from LArCafJobs.LArNoiseConfig import LArNoiseCfg
from AthenaConfiguration.MainServicesConfig import MainServicesCfg


def fromRunArgs(runArgs):
    from AthenaConfiguration.AllConfigFlags import initConfigFlags    

    flags=initConfigFlags()

    from LArCafJobs.LArNoiseFlags import addNoiseFlags
    addNoiseFlags(flags)

    commonRunArgsToFlags(runArgs, flags)

    processPreInclude(runArgs, flags)
    processPreExec(runArgs, flags)

    flags.Input.Files=runArgs.inputESDFile
    if hasattr(runArgs,"outputNTUP_LARNOISEFile"):
       flags.LArNoise.outNtupLAr=runArgs.outputNTUP_LARNOISEFile

    if hasattr(runArgs,"outputNTUP_HECNOISEFile"):
        flags.LArNoise.HECNoiseNtup=runArgs.outputNTUP_HECNOISEFile

    if not hasattr(runArgs,"conditionsTag") or runArgs.conditionsTag=="CURRENT":
        print("Resolving 'CURRENT' express conditions tag ...")
        sys.path.append('/afs/cern.ch/user/a/atlcond/utils22/')
        from CondUtilsLib.AtlCoolBKLib import resolveAlias
        resolver=resolveAlias()
        currentGlobalES=resolver.getCurrentES().replace("*","ST")
        print("Found ",currentGlobalES)
        flags.IOVDb.GlobalTag=currentGlobalES
    else:
        flags.IOVDb.GlobalTag=runArgs.conditionsTag

    if hasattr(runArgs,"skipEvents"):
        flags.Exec.SkipEvents=runArgs.skipEvents

    if hasattr(runArgs,"maxEvents"):
        flags.Exec.MaxEvents=runArgs.maxEvents

    flags.Trigger.doID=False

    # To respect --athenaopts 
    flags.fillFromArgs()

    flags.lock()
    
    cfg=MainServicesCfg(flags)
    cfg.merge(LArNoiseCfg(flags))

    processPostInclude(runArgs, flags, cfg)
    processPostExec(runArgs, flags, cfg)

    # Run the final accumulator
    sc = cfg.run()
    sys.exit(not sc.isSuccess())
