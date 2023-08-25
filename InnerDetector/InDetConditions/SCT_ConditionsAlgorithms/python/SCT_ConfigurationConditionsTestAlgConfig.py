"""Define method to configure and test SCT_ConfigurationConditionsTestAlg

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SCT_ConfigurationConditionsTestAlgCfg(flags, name="SCT_ConfigurationConditionsTestAlg", **kwargs):
    """Return a configured SCT_ConfigurationConditionsTestAlg"""
    acc = ComponentAccumulator()
    from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_ConfigurationConditionsToolCfg
    kwargs.setdefault("SCT_ConfigurationConditionsTool", acc.popToolsAndMerge(SCT_ConfigurationConditionsToolCfg(flags)))
    acc.addEventAlgo(CompFactory.SCT_ConfigurationConditionsTestAlg(name, **kwargs))
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
    flags.Input.RunNumber = 310000 # MC16e 2018 run number
    flags.Input.TimeStamp = 1550000000 # MC16e 2018 time stamp
    flags.IOVDb.GlobalTag = "OFLCOND-MC16-SDR-RUN2-01"
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.Detector.GeometrySCT = True
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    cfg.merge(McEventSelectorCfg(flags))

    algkwargs = {}
    algkwargs["OutputLevel"] = INFO
    cfg.merge(SCT_ConfigurationConditionsTestAlgCfg(flags, **algkwargs))

    cfg.run(maxEvents=20)
