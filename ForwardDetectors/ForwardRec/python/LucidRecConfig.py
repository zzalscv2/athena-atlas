# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from OutputStreamAthenaPool.OutputStreamConfig import addToESD
from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg


def LucidRecCfg(flags):
    result=ComponentAccumulator()

    if flags.Input.isMC: 
        result.addEventAlgo(CompFactory.LUCID_DigitRawDataCnv("LUCID_DigitRawDataCnv"))
    else:
        result.merge(ByteStreamReadCfg(flags))
        result.addEventAlgo(CompFactory.LUCID_ByteStreamRawDataCnv("LUCID_ByteStreamRawDataCnv"))
        result.merge(addToESD(flags,["LUCID_RawDataContainer#Lucid_RawData",]))

    return result




if __name__=="__main__":
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    log.setLevel(DEBUG)

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    ConfigFlags.Input.Files = defaultTestFiles.RAW
    ConfigFlags.lock()

    acc = MainServicesCfg( ConfigFlags )
    acc.merge(LucidRecCfg(ConfigFlags))

    acc.run(10)
