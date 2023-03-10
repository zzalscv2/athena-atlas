# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def HTTMapMakerCfg(flags):
    acc = ComponentAccumulator()
    from TrigHTTConfTools.HTTAnalysisConfig import HTTReadInputCfg
    alg = CompFactory.HTTMapMakerAlg(
        OutFileName=flags.OutFileName,
        KeyString=flags.KeyString,
        nSlices=flags.nSlices,
        region=flags.region,
        trim=flags.trim,
        InputTool = acc.getPrimaryAndMerge(HTTReadInputCfg(flags))
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


    flags.Input.Files = ['HTTWrapper.singlemu_Pt10.root']
    flags.fillFromArgs()
    flags.lock()

    acc=MainServicesCfg(flags)
    acc.store(open('HTTMapMakerConfig.pkl','wb'))
    acc.merge(HTTMapMakerCfg(flags))

    from AthenaConfiguration.Utils import setupLoggingLevels
    setupLoggingLevels(flags, acc)

    statusCode = acc.run()
    assert statusCode.isSuccess() is True, "Application execution did not succeed"

