# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# File: CaloRec/python/CaloBCIDLumiCondAlgConfig.py
# Created: Mar 2020, sss
# Purpose: Configure CaloBCIDLumiCondAlg.

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def CaloBCIDLumiCondAlgCfg (flags):
    result = ComponentAccumulator()

    from CaloRec.CaloBCIDCoeffsCondAlgConfig import CaloBCIDCoeffsCondAlgCfg
    result.merge (CaloBCIDCoeffsCondAlgCfg (flags))

    if flags.Input.isMC is False:
        from LumiBlockComps.LuminosityCondAlgConfig import LuminosityCondAlgCfg
        result.merge (LuminosityCondAlgCfg (flags))

    else:
        from LumiBlockComps.BunchCrossingCondAlgConfig import BunchCrossingCondAlgCfg
        result.merge (BunchCrossingCondAlgCfg(flags))


    CaloBCIDLumiCondAlg = CompFactory.CaloBCIDLumiCondAlg # CaloRec
    alg = CaloBCIDLumiCondAlg ('CaloBCIDLumiCondAlg',
                               CoeffsKey = 'CaloBCIDCoeffs',
                               BunchCrossingCondDataKey = 'BunchCrossingData',
                               LuminosityCondDataKey = 'LuminosityCondData',
                               isMC = flags.Input.isMC,
                               OutputLumiKey = 'CaloBCIDLumi')
    result.addCondAlgo (alg)

    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles

    only = ['CaloBCIDCoeffsCondAlg',
            'CaloBCIDLumiCondAlg',
            'LuminosityCondAlg',
            'BunchCrossingCondAlg']

    print ('--- data')
    flags1 = initConfigFlags()
    flags1.Input.Files = defaultTestFiles.RAW_RUN2
    flags1.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags1.lock()
    acc1 = CaloBCIDLumiCondAlgCfg (flags1)
    acc1.printConfig(summariseProps=True, onlyComponents=only)
    print ('IOVDbSvc:', acc1.getService('IOVDbSvc').Folders)
    acc1.wasMerged()

    print ('--- mc')
    flags2 = initConfigFlags()
    flags2.Input.Files = defaultTestFiles.ESD
    flags2.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags2.lock()
    acc2 = CaloBCIDLumiCondAlgCfg (flags2)
    acc2.printConfig(summariseProps=True, onlyComponents=only)
    print ('IOVDbSvc:', acc2.getService('IOVDbSvc').Folders)
    acc2.wasMerged()
