# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Simple script to run a
# Tau job
#
# Usefull for quick testing
# run with
#
# athena --CA runTauOnly.py 
# or
# python runTauOnly.py

import sys

def tauSpecialContent(flags,cfg):
    StreamAOD = cfg.getEventAlgo("OutputStreamAOD")
    newList = [x for x in StreamAOD.ItemList if "Tau" in x]
    StreamAOD.ItemList = newList

    StreamESD = cfg.getEventAlgo("OutputStreamESD")
    newList = [x for x in StreamESD.ItemList if "Tau" in x]
    StreamESD.ItemList = newList

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

    # uncomment given something like export ATHENA_CORE_NUMBER=2
    # flags.Concurrency.NumThreads = 2

    # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, None, use_metadata=True,
                       toggle_geometry=True, keep_beampipe=True)

    # Schedule Tau Reco
    flags.Reco.EnableTrigger = False
    flags.Reco.EnableCombinedMuon = True
    flags.Reco.EnablePFlow = True
    flags.Reco.EnableTau = True
    flags.Reco.EnableJet = True
    flags.Reco.EnableBTagging = False
    flags.Reco.EnableCaloRinger = False
    flags.Reco.PostProcessing.GeantTruthThinning = False
    flags.Reco.PostProcessing.TRTAloneThinning = False
    flags.lock()

    from RecJobTransforms.RecoSteering import RecoSteering
    acc = RecoSteering(flags)

    # keep only tau containers
    tauSpecialContent(flags,acc)

    # Special message service configuration
    from Digitization.DigitizationSteering import DigitizationMessageSvcCfg
    acc.merge(DigitizationMessageSvcCfg(flags))

    from AthenaConfiguration.Utils import setupLoggingLevels
    setupLoggingLevels(flags, acc)

    # Print reco domain status
    from RecJobTransforms.RecoConfigFlags import printRecoFlags
    printRecoFlags(flags)

    # running
    statusCode = acc.run()

    return statusCode


if __name__ == "__main__":
    statusCode = None
    statusCode = _run()
    assert statusCode is not None, "Issue while running"
    sys.exit(not statusCode.isSuccess())


