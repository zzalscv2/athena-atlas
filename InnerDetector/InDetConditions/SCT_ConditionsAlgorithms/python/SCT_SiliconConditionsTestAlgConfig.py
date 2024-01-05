"""Define method to configure and test SCT_SiliconConditionsTestAlg

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SCT_SiliconConditionsTestAlgCfg(flags, name="SCT_SiliconConditionsTestAlg", **kwargs):
    """Return a configured SCT_SiliconConditionsTestAlg"""
    acc = ComponentAccumulator()
    from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_SiliconConditionsCfg
    kwargs.setdefault("SCT_SiliconConditionsTool", acc.popToolsAndMerge(SCT_SiliconConditionsCfg(flags)))
    acc.addEventAlgo(CompFactory.SCT_SiliconConditionsTestAlg(name, **kwargs))
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
    flags.Input.RunNumbers = [300000] # MC16c 2017 run number
    flags.Input.TimeStamps = [1500000000] # MC16c 2017 time stamp
    flags.IOVDb.GlobalTag = "OFLCOND-MC16-SDR-18"
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.Detector.GeometrySCT = True
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    cfg.merge(McEventSelectorCfg(flags))

    cfg.merge(SCT_SiliconConditionsTestAlgCfg(flags))

    cfg.run(maxEvents=20)
