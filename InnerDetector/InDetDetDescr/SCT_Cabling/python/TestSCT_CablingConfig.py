# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# https://twiki.cern.ch/twiki/bin/viewauth/AtlasComputing/AthenaJobConfigRun3

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def SCT_TestCablingAlgCfg(flags):
    cfg=ComponentAccumulator()

    from SCT_Cabling.SCT_CablingConfig import SCT_CablingCondAlgCfg
    cfg.merge(SCT_CablingCondAlgCfg(flags))

    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    geoCfg=GeoModelCfg(flags)
    cfg.merge(geoCfg)

    from AthenaCommon.Constants import INFO
    SCT_CablingTool = CompFactory.SCT_CablingTool()
    SCT_CablingTool.DataSource = "COOLVECTOR"
    SCT_CablingTool.OutputLevel = INFO

    SCT_TestCablingAlg=CompFactory.SCT_TestCablingAlg
    testAlg = SCT_TestCablingAlg(SCT_CablingTool = SCT_CablingTool,
                                 OutputLevel = INFO)
    cfg.addEventAlgo(testAlg)

    return cfg


if __name__=="__main__":
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    log.setLevel(DEBUG)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Input.isMC = True
    flags.Input.RunNumbers = [300000]
    flags.Input.TimeStamps = [1500000000]
    # https://twiki.cern.ch/twiki/bin/viewauth/AtlasComputing/ConditionsRun1RunNumbers
    flags.IOVDb.GlobalTag = "OFLCOND-RUN12-SDR-25"
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.Detector.GeometrySCT = True
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg=MainServicesCfg(flags)

    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    cfg.merge(McEventSelectorCfg(flags))

    cfg.merge(SCT_TestCablingAlgCfg(flags))

    # IOVDbSvc = cfg.getService("IOVDbSvc")
    #
    ## To dump database in JSON files (c.f. SCT_CablingWriteToFile.py in the old job configuration)
    # IOVDbSvc.OutputToFile = True
    #
    ## To use CREST database (c.f. TestSCT_CablingFromCrest.py in the old job configuration)
    ## together with flags.IOVDb.GlobalTag="CREST-RUN12-SDR-25-MC" for MC
    # IOVDbSvc.Source = "CREST"

    cfg.run(maxEvents=20)
