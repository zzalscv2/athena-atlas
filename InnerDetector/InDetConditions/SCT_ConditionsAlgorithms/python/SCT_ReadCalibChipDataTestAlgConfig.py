"""Define method to configure and test SCT_ReadCalibChipDataTestAlg

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SCT_ReadCalibChipDataTestAlgCfg(flags, name="SCT_ReadCalibChipDataTestAlg", **kwargs):
    """Return a configured SCT_ReadCalibChipDataTestAlg"""
    acc = ComponentAccumulator()
    from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_ReadCalibChipDataCfg
    kwargs.setdefault("SCT_ReadCalibChipDataTool", acc.popToolsAndMerge(SCT_ReadCalibChipDataCfg(flags)))
    acc.addEventAlgo(CompFactory.SCT_ReadCalibChipDataTestAlg(name, **kwargs))
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

    algkwargs = {}
    ## Modules to test:
    algkwargs["ModuleOfflinePosition"] = [-2, 0, 1, 11, 0, 706] #(EC/B, Disk/Layer, eta, phi, side, strip)
    algkwargs["DoTestmyConditionsSummary"] = True
    algkwargs["DoTestmyDataSummary"] = True
    algkwargs["OutputLevel"] = INFO
    cfg.merge(SCT_ReadCalibChipDataTestAlgCfg(flags, **algkwargs))

    cfg.run(maxEvents=20)
