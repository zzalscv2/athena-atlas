# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Simple script to run a
# Calo/Tracking/Egamma job
#
# Usefull for quick testing
# run with
#
# athena --CA runEgammaOnly.py
# or
# python runEgammaOnly.py

import sys


def _run():
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    # input
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Exec.MaxEvents = 20
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep = ProductionStep.Reconstruction

    # output
    flags.Output.ESDFileName = "myESD.pool.root"
    flags.Output.AODFileName = "myAOD.pool.root"

    # egamma Only
    from egammaConfig.egammaOnlyFromRawFlags import egammaOnlyFromRaw
    egammaOnlyFromRaw(flags)

    flags.lock()
    flags.dump()

    from RecJobTransforms.RecoSteering import RecoSteering
    acc = RecoSteering(flags)

    from AthenaConfiguration.Utils import setupLoggingLevels
    setupLoggingLevels(flags, acc)

    # running
    statusCode = acc.run()
    return statusCode


if __name__ == "__main__":
    statusCode = None
    statusCode = _run()
    assert statusCode is not None, "Issue while running"
    sys.exit(not statusCode.isSuccess())
