# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# File: LumiBlockComps/python/LBDurationCondAlgConfig.py
# Created: May 2019, sss
# Purpose: Configure LBDurationCondAlg.
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from IOVDbSvc.IOVDbSvcConfig import addFolders

def LBDurationCondAlgCfg (flags):
    name = 'LBDurationCondAlg'
    result = ComponentAccumulator()

    folder = "/TRIGGER/LUMI/LBLB"
    result.merge (addFolders (flags, folder, 'TRIGGER',
                              className = 'AthenaAttributeList'))

    LBDurationCondAlg=CompFactory.LBDurationCondAlg
    alg = LBDurationCondAlg (name,
                             LBLBFolderInputKey = folder,
                             LBDurationOutputKey = 'LBDurationCondData')

    result.addCondAlgo (alg)
    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    print ('--- data')
    flags1 = initConfigFlags()
    flags1.Input.Files = defaultTestFiles.RAW_RUN2
    flags1.lock()
    acc1 = LBDurationCondAlgCfg (flags1)
    acc1.printCondAlgs(summariseProps=True)
    print ('IOVDbSvc:', acc1.getService('IOVDbSvc').Folders)
    acc1.wasMerged()
