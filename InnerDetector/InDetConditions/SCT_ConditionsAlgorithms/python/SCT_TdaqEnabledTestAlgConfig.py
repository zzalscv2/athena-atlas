"""Define method to configure and test SCT_TdaqEnabledTestAlg

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SCT_TdaqEnabledTestAlgCfg(flags, name="SCT_TdaqEnabledTestAlg", **kwargs):
    """Return a configured SCT_TdaqEnabledTestAlg"""
    acc = ComponentAccumulator()
    from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_TdaqEnabledToolCfg
    kwargs.setdefault("SCT_TdaqEnabledTool", acc.popToolsAndMerge(SCT_TdaqEnabledToolCfg(flags)))
    acc.addEventAlgo(CompFactory.SCT_TdaqEnabledTestAlg(name, **kwargs))
    return acc

if __name__=="__main__":
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Input.isMC = False
    flags.Input.ProjectName = "data16_13TeV"
    flags.Input.RunNumbers = [310809]
    flags.Input.TimeStamps = [1476741326] # LB 18 of run 310809, 10/17/2016 @ 9:55pm (UTC)
    flags.IOVDb.GlobalTag = "CONDBR2-BLKPA-2017-06"
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
    cfg.merge(SCT_TdaqEnabledTestAlgCfg(flags, **algkwargs))

    cfg.run(maxEvents=20)
