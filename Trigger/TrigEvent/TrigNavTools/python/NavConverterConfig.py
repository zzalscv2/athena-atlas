#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
# from AthenaCommon.Constants import DEBUG  # uncomment for the easy usage while testing with DEBUG
from AthenaCommon.Logging import logging
log = logging.getLogger("NavConverterConfig")


def NavConverterCfg(flags, chainsFilter = []):
    """Configures Run 1/2 to Run 3 navigation conversion algorithm for all triggers"""

    # this is intentionally commented and left for further filtering
    # chainsFilter = list(filter(lambda x : "tau" not in x, chainList))
    chainsFilter = ['HLT_mu4'] # single chain passing tests overwriting chainsFilter

    acc = ComponentAccumulator()

    if not flags.Trigger.doEDMVersionConversion:
        return acc

    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg, getRun3NavigationContainerFromInput
    tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))

    r2ToR3OutputName = getRun3NavigationContainerFromInput(flags)

    cnvAlg = CompFactory.Run2ToRun3TrigNavConverterV2("TrigRun2ToRun3NavConverter") # optional: OutputLevel = DEBUG
    cnvAlg.TrigDecisionTool = tdt
    cnvAlg.TrigNavReadKey = ""
    cnvAlg.TrigConfigSvc = tdt.TrigConfigSvc
    cnvAlg.OutputNavKey = r2ToR3OutputName
    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD, addToESD
    collections = [f"xAOD::TrigCompositeContainer#{r2ToR3OutputName}", f"xAOD::TrigCompositeAuxContainer#{r2ToR3OutputName}Aux."]
    acc.merge(addToAOD(flags, collections))
    acc.merge(addToESD(flags, collections))

    cnvAlg.Rois = ["initialRoI","forID","forID1","forID2","forMS","forSA","forTB","forMT","forCB"]

    from TrigEDMConfig.TriggerEDM import getTriggerEDMList
    edm = getTriggerEDMList("AODCONV", flags.Trigger.EDMVersion)
    types = [ t for t in edm ]
    log.info("Assuming these collections are relevant for trigger: %s", " ".join(types))
    cnvAlg.Collections = types
    cnvAlg.Chains = chainsFilter
    cnvAlg.doCompression = False # set True for compression
    acc.addEventAlgo(cnvAlg)

    checker = CompFactory.Trig.NavigationTesterAlg(FailOnDifference = True, TrigDecisionTool = tdt) # optional: OutputLevel = DEBUG
    checker.RetrievalToolRun2Nav = CompFactory.Trig.IParticleRetrievalTool()
    
    # in conversion job  Run2 TDT is setup as default, we need to setup an alternative to access Run 3 format
    run3tdt = CompFactory.Trig.TrigDecisionTool("Run3TrigDecisionTool",
                                                HLTSummary = r2ToR3OutputName,
                                                NavigationFormat = 'TrigComposite',
                                                AcceptMultipleInstance=True,
                                                TrigConfigSvc = tdt.TrigConfigSvc)
    acc.addPublicTool(run3tdt)
    checker.RetrievalToolRun3Nav = CompFactory.Trig.R3IParticleRetrievalTool(TrigDecisionTool = run3tdt)
    checker.Chains = chainsFilter
    acc.addEventAlgo(checker)

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.Trigger.doEDMVersionConversion = True
    flags.lock()

    acc = NavConverterCfg(flags)
    acc.printConfig(withDetails=True, summariseProps=True)
    acc.wasMerged()
