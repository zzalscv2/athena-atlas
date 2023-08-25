"""Define method to configure and test SCT_SensorsTestAlg

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SCT_MajorityConditionsTestAlgCfg(flags, name="SCT_SensorsTestAlg", **kwargs):
    """Return a configured SCT_SensorsTestAlg"""
    acc = ComponentAccumulator()
    from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_SensorsCfg
    kwargs.setdefault("SCT_SensorsTool", acc.popToolsAndMerge(SCT_SensorsCfg(flags, **kwargs)))
    acc.addEventAlgo(CompFactory.SCT_SensorsTestAlg(name))
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
    flags.Input.RunNumber = 142913
    flags.Input.TimeStamp = 1260630000 # During run 142913, 12/12/2009 @ 3:00pm (UTC)
    flags.IOVDb.GlobalTag = "COMCOND-BLKPA-RUN1-09"
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN1_2012
    flags.Detector.GeometrySCT = True
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    cfg.merge(McEventSelectorCfg(flags))

    kwargs = {}
    kwargs["folderTag"] = "SctSensors-Sep03-14"

    cfg.merge(SCT_MajorityConditionsTestAlgCfg(flags, **kwargs))

    cfg.run(maxEvents=20)
