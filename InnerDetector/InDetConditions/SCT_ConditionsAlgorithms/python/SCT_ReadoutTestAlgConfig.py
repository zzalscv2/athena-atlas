"""Define method to configure and test SCT_ReadoutTestAlg

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SCT_ReadoutTestAlgCfg(flags, name="SCT_ReadoutTestAlg", **kwargs):
    """Return a configured SCT_ReadoutTestAlg"""
    acc = ComponentAccumulator()
    from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_ReadoutToolCfg
    kwargs.setdefault("SCT_ReadoutTool", acc.popToolsAndMerge(SCT_ReadoutToolCfg(flags)))

    # Module type and link status
    # kwargs.setdefault("ModuleId", 143704064) # Endcap (default is barrel)
    # kwargs.setdefault("LinkOStatus", False)  # Link 0 disabled (default is 'good')
    # kwargs.setdefault("Link1Status", False)  # Link 1 disabled (default is 'good')

    # Some possible chip configs strings
    masterChip       = "10110000000001"
    slaveChip        = "10110000000101"
    # slaveChipIn1     = "10110000010101"
    # slaveChipOut1    = "10110000001101"
    endChip          = "10110000000111"
    # masterAndEndChip = "10110000000011"

    # Some test module configs
    defaultLink            = [masterChip] + [slaveChip]*4 + [endChip]
    # noEndLink              = [masterChip] + [slaveChip]*5
    # endAt2Link             = [masterChip] + [slaveChip] + [endChip] + [slaveChip]*3
    # bypass2Link            = [masterChip] + [slaveChipIn1] + [slaveChip] + [slaveChipOut1]  + [slaveChip] + [endChip]
    # master3Link            = [slaveChip]*3 + [masterChip] + [slaveChip] + [endChip]
    # noMapped4Link          = [masterChip] + [slaveChip]*3 + [slaveChipIn1] + [endChip]
    # nothingTalkingTo2Link  = [masterChip] + [slaveChip] + [slaveChipIn1] + [slaveChip]*2 + [endChip]
    # allExcept1Link0Mod     = [masterChip] + [slaveChip]*4 + [slaveChipIn1] + [slaveChipOut1] + [masterAndEndChip] + [slaveChip]*3 + [endChip]
    # allLink0EndcapMod      = [masterChip] + [slaveChip]*10 + [endChip]
    # allLink1EndcapMod      = [slaveChip]*5 + [endChip] + [masterChip] + [slaveChip]*5
    # infiniteEndcapMod      = [masterChip] + [slaveChip]*11

    kwargs.setdefault("ChipConfigurationList", defaultLink*2)
    # kwargs.setdefault("ChipConfigurationList", defaultLink + endAt2Link)
    # kwargs.setdefault("ChipConfigurationList", bypass2Link + defaultLink)
    # kwargs.setdefault("ChipConfigurationList", master3Link + defaultLink)
    # kwargs.setdefault("ChipConfigurationList", defaultLink + noMapped4Link)
    # kwargs.setdefault("ChipConfigurationList", defaultLink + nothingTalkingTo2Link)
    # kwargs.setdefault("ChipConfigurationList", noEndLink + defaultLink)
    # kwargs.setdefault("ChipConfigurationList", allExcept1Link1Mod)
    # kwargs.setdefault("ChipConfigurationList", allLink0EndcapMod)
    # kwargs.setdefault("ChipConfigurationList", allLink1EndcapMod)
    # kwargs.setdefault("ChipConfigurationList", infiniteEndcapMod)

    acc.addEventAlgo(CompFactory.SCT_ReadoutTestAlg(name, **kwargs))
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

    cfg.merge(SCT_ReadoutTestAlgCfg(flags))

    cfg.run(maxEvents=20)
