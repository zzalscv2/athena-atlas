"""Define method to configure and test SCTSiLorentzAngleTestAlg

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AtlasGeoModel.GeoModelConfig import GeoModelCfg

def SCTSiLorentzAngleTestAlgCfg(flags, name="SCTSiLorentzAngleTestAlg", **kwargs):
    """Return a configured SCTSiLorentzAngleTestAlg"""
    acc = ComponentAccumulator()
    acc.merge(GeoModelCfg(flags)) # For SCT_ID used in SCTSiLorentzAngleTestAlg
    from SiLorentzAngleTool.SCT_LorentzAngleConfig import SCT_LorentzAngleToolCfg
    kwargs.setdefault("SCTLorentzAngleTool", acc.popToolsAndMerge(SCT_LorentzAngleToolCfg(flags)))
    acc.addEventAlgo(CompFactory.SCTSiLorentzAngleTestAlg(**kwargs))
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

    cfg.merge(SCTSiLorentzAngleTestAlgCfg(flags))

    cfg.run(maxEvents=20)
