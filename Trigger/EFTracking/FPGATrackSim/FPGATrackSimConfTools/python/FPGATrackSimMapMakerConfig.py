# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def FPGATrackSimMapMakerCfg(flags):
    acc = ComponentAccumulator()
    from FPGATrackSimConfTools.FPGATrackSimAnalysisConfig import FPGATrackSimReadInputCfg
    alg = CompFactory.FPGATrackSimMapMakerAlg(
        GeometryVersion=flags.GeoModel.AtlasVersion,
        OutFileName=flags.OutFileName,
        KeyString=flags.KeyString,
        nSlices=flags.nSlices,
        region=flags.region,
        trim=flags.trim,
        InputTool = acc.getPrimaryAndMerge(FPGATrackSimReadInputCfg(flags))        
        )

    acc.addEventAlgo(alg)
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    flags = initConfigFlags()
    flags.addFlag("OutFileName", "MMTest")
    flags.addFlag("KeyString", "strip,barrel,2")
    flags.addFlag("nSlices", 10)
    flags.addFlag("trim", 0.1)
    flags.addFlag("region", 0)
    from AthenaCommon.Logging import logging
    log = logging.getLogger(__name__)

    flags.fillFromArgs()
    if not flags.Trigger.FPGATrackSim.wrapperFileName and flags.Input.Files:
        flags.Trigger.FPGATrackSim.wrapperFileName = flags.Input.Files
        log.info("Taken wrapper input files from Input.Files(set via cmd line --filesInput option) property: %s", str(flags.Trigger.FPGATrackSim.wrapperFileName))
    flags.lock()

    acc=MainServicesCfg(flags)
    acc.store(open('FPGATrackSimMapMakerConfig.pkl','wb'))
    acc.merge(FPGATrackSimMapMakerCfg(flags))

    from AthenaConfiguration.Utils import setupLoggingLevels
    setupLoggingLevels(flags, acc)

    statusCode = acc.run()
    assert statusCode.isSuccess() is True, "Application execution did not succeed"

