# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# File: CoolLumiUtilities/python/OnlineLumiCalibrationCondAlgConfig.py
# Created: May 2019, sss
# Purpose: Configure OnlineLumiCalibrationCondAlg.
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from IOVDbSvc.IOVDbSvcConfig import addFolders


def OnlineLumiCalibrationCondAlgCfg (flags):
    name = 'OnlineLumiCalibrationCondAlg'
    result = ComponentAccumulator()

    if flags.Input.isMC:
        return result

    # For both runs 1 and 2
    folder = '/TDAQ/OLC/CALIBRATIONS'
    result.merge (addFolders (flags, folder, 'TDAQ',
                              className='CondAttrListCollection'))

    OnlineLumiCalibrationCondAlg=CompFactory.OnlineLumiCalibrationCondAlg
    alg = OnlineLumiCalibrationCondAlg (name,
                                        CalibrationFolderInputKey = folder,
                                        LumiCalibOutputKey = 'OnlineLumiCalibrationCondData')

    result.addCondAlgo (alg)
    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    print ('--- data')
    flags1 = initConfigFlags()
    flags1.Input.Files = defaultTestFiles.RAW_RUN2
    flags1.lock()
    acc1 = OnlineLumiCalibrationCondAlgCfg (flags1)
    acc1.printCondAlgs(summariseProps=True)
    print ('IOVDbSvc:', acc1.getService('IOVDbSvc').Folders)
    acc1.wasMerged()

    print ('--- mc')
    flags2 = initConfigFlags()
    flags2.Input.Files = defaultTestFiles.ESD
    flags2.lock()
    acc2 = OnlineLumiCalibrationCondAlgCfg (flags2)
    acc2.printCondAlgs(summariseProps=True)
    print ('IOVDbSvc:', acc2.getServices())
    acc2.wasMerged()
