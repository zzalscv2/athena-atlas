"""Define method to configure and test SCT_RODVetoTestAlg

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SCT_RODVetoTestAlgCfg(flags, name="SCT_RODVetoTestAlg", **kwargs):
    """Return a configured SCT_RODVetoTestAlg"""
    acc = ComponentAccumulator()
    from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_RODVetoCfg
    algArgs = {}
    algArgs["SCT_RODVetoTool"] = acc.popToolsAndMerge(SCT_RODVetoCfg(flags, **kwargs))
    acc.addEventAlgo(CompFactory.SCT_RODVetoTestAlg(name, **algArgs))
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
    flags.Input.TimeStamps = [1500000000]
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
    kwargs["BadRODIds"] = [0x240100, 0x240030] # RODs to be vetoed
    cfg.merge(SCT_RODVetoTestAlgCfg(flags, **kwargs))

    cfg.run(maxEvents=20)
