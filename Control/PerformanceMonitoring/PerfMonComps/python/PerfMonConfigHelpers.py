# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def setPerfmonFlagsFromRunArgs(flags, runArgs):
    """ A helper function to set perfmon flags from runArgs."""

    if hasattr(runArgs, 'perfmon') and runArgs.perfmon != 'none':
        if runArgs.perfmon == 'fastmonmt':
            flags.PerfMon.doFastMonMT = True
        elif runArgs.perfmon == 'fullmonmt':
            flags.PerfMon.doFullMonMT = True
        else:
            raise RuntimeError(f"Unknown perfmon type: {runArgs.perfmon}")
        flags.PerfMon.OutputJSON = f"perfmonmt_{runArgs.trfSubstepName}.json"
