"""Define method to configure and test SCT_ModuleVetoTestAlg

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SCT_StripVetoTestAlgCfg(flags, name="SCT_ModuleVetoTestAlg", **kwargs):
    """Return a configured SCT_ModuleVetoTestAlg"""
    acc = ComponentAccumulator()
    from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_ModuleVetoCfg
    acc.addEventAlgo(CompFactory.SCT_ModuleVetoTestAlg(name,
                                                       ModuleVetoTool=acc.popToolsAndMerge(SCT_ModuleVetoCfg(flags, **kwargs))))
    return acc

if __name__=="__main__":
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Input.isMC = True
    flags.Input.ProjectName = "mc16_13TeV"
    flags.Input.RunNumber = 300000 # MC16c 2017 run number
    flags.Input.TimeStamp = 1500000000 # MC16c 2017 time stamp
    flags.IOVDb.GlobalTag = "OFLCOND-MC16-SDR-18"
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.Detector.GeometrySCT = True
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    cfg.merge(McEventSelectorCfg(flags))

    kwargs = {}

    ### Use COOL database for SCT_ModuleVetoTool
    kwargs["useDB"] = True # False
    if kwargs["useDB"]:
        kwargs["folderTag"] = "SCTManualBadModules-000-00"
        kwargs["BadModuleIdentifiers"] = ["database"]
    else:
        kwargs["BadModuleIdentifiers"] = ["1", "2"]

    cfg.merge(SCT_StripVetoTestAlgCfg(flags, **kwargs))

    cfg.run(maxEvents=20)
