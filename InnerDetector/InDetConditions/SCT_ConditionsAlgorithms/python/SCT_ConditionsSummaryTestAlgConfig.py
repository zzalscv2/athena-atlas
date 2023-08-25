"""Define method to configure and test SCT_ConditionsSummaryTestAlg

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SCT_ConditionsSummaryTestAlgCfg(flags, name="SCT_ConditionsSummaryTestAlg", **kwargs):
    """Return a configured SCT_ConditionsSummaryTestAlg"""
    acc = ComponentAccumulator()
    from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_ConditionsSummaryToolCfg
    InDetSCT_ConditionsSummaryToolWithoutFlagged = acc.popToolsAndMerge(SCT_ConditionsSummaryToolCfg(flags,withFlaggedCondTool=False))
    kwargs.setdefault("SCT_ConditionsSummaryTool", InDetSCT_ConditionsSummaryToolWithoutFlagged)
    acc.addEventAlgo(CompFactory.SCT_ConditionsSummaryTestAlg(name, **kwargs))
    return acc

if __name__=="__main__":
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Input.isMC = False
    flags.Input.ProjectName = "data17_13TeV"
    flags.Input.RunNumber = [310809]
    flags.Input.TimeStamp = 1476741326 # LB 18 of run 310809, 10/17/2016 @ 9:55pm (UTC)
    flags.IOVDb.GlobalTag = "CONDBR2-BLKPA-2017-06"
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.Detector.GeometrySCT = True
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    cfg.merge(McEventSelectorCfg(flags))

    cfg.merge(SCT_ConditionsSummaryTestAlgCfg(flags))

    cfg.run(maxEvents=20)
