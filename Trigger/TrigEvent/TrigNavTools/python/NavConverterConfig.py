#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging
log = logging.getLogger("NavConverterConfig")


def NavConverterCfg(flags):
    """Configures Run 1/2 to Run 3 navigation conversion algorithm for all triggers"""
    acc = ComponentAccumulator()
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))

    cnvAlg = CompFactory.Run2ToRun3TrigNavConverterV2("TrigRun2ToRun3NavConverter")
    cnvAlg.TrigDecisionTool = tdt
    cnvAlg.TrigNavReadKey = ""
    cnvAlg.TrigConfigSvc = tdt.TrigConfigSvc
    outputName = "HLTNav_R2ToR3Summary"
    cnvAlg.OutputNavKey = outputName
    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD, addToESD
    collections = [f"xAOD::TrigCompositeContainer#{outputName}", f"xAOD::TrigCompositeAuxContainer#{outputName}Aux."]
    acc.merge(addToAOD(flags, collections))
    acc.merge(addToESD(flags, collections))

    cnvAlg.Rois = ["initialRoI","forID","forID1","forID2","forMS","forSA","forTB","forMT","forCB"]

    from TrigEDMConfig.TriggerEDM import getTriggerEDMList
    edm = getTriggerEDMList("AODCONV", flags.Trigger.EDMVersion)
    types = [ t for t in edm ]
    log.info("Assuming these collections are relevant for trigger: %s", " ".join(types))
    cnvAlg.Collections = types
    # example of chain collection: comment if all chains are to be processed
    cnvAlg.Chains = ["HLT_mu4"]
    cnvAlg.doCompression = True
    acc.addEventAlgo(cnvAlg)

    checker = CompFactory.Trig.NavigationTesterAlg()
    checker.RetrievalToolRun2Nav = CompFactory.Trig.IParticleRetrievalTool()
    # in conversion job  Run2 TDT is setup as default, we need to setup an alternative to access Run 3 format

    run3tdt = CompFactory.Trig.TrigDecisionTool("Run3TrigDecisionTool",
                                                HLTSummary = "HLTNav_R2ToR3Summary",
                                                NavigationFormat = 'TrigComposite',
                                                AcceptMultipleInstance=True)
    acc.addPublicTool(run3tdt)
    checker.RetrievalToolRun3Nav = CompFactory.Trig.R3IParticleRetrievalTool(TrigDecisionTool = run3tdt)
    checker.Chains=['HLT_e26_lhtight_nod0_e15_etcut_L1EM7_Zee', 'HLT_mu4'] #TODO automate this
    acc.addEventAlgo(checker)

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.lock()

    acc = NavConverterCfg(flags)
    acc.printConfig(withDetails=True, summariseProps=True)
    acc.wasMerged()
