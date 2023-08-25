"""Define method to configure and test SCT_ConditionsParameterTestAlg

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SCT_ConditionsParameterTestAlgCfg(flags, name="SCT_ConditionsParameterTestAlg", **kwargs):
    """Return a configured SCT_ConditionsParameterTestAlg"""
    acc = ComponentAccumulator()
    from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_ConditionsParameterCfg
    kwargs.setdefault("SCT_ConditionsParameterTool", acc.popToolsAndMerge(SCT_ConditionsParameterCfg(flags)))
    acc.addEventAlgo(CompFactory.SCT_ConditionsParameterTestAlg(name, **kwargs))
    return acc

if __name__=="__main__":
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Input.isMC = False
    flags.Input.ProjectName = "data12_8TeV"
    flags.Input.RunNumber = 215643
    flags.Input.TimeStamp = 1354748400 # LB 469 of run 215643, 2012-12-05 @ 11:00pm (UTC)
    flags.IOVDb.GlobalTag = "COMCOND-BLKPA-RUN1-09"
    flags.IOVDb.DatabaseInstance = "COMP200"
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN1_2012
    flags.Detector.GeometrySCT = True
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    cfg.merge(McEventSelectorCfg(flags))

    cfg.merge(SCT_ConditionsParameterTestAlgCfg(flags))

    cfg.run(maxEvents=20)
