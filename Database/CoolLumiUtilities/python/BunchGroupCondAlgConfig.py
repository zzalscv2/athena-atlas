# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# File: CoolLumiUtilities/python/BunchGroupCondAlgConfig.py
# Created: May 2019, sss
# Purpose: Configure BunchGroupCondAlg.
#


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from IOVDbSvc.IOVDbSvcConfig import addFolders


def BunchGroupCondAlgCfg (flags):
    name = 'BunchGroupCondAlg'
    result = ComponentAccumulator()

    folder = ''
    if flags.IOVDb.DatabaseInstance == 'COMP200':
        folder = '/TRIGGER/LVL1/BunchGroupContent'

        # Mistakenly created as multi-version folder, must specify HEAD 
        result.merge (addFolders (flags, folder, 'TRIGGER', tag='HEAD',
                                  className='AthenaAttributeList'))

    BunchGroupCondAlg=CompFactory.BunchGroupCondAlg
    alg = BunchGroupCondAlg (name,
                             BunchGroupFolderInputKey = folder,
                             BunchGroupOutputKey = 'BunchGroupCondData')

    result.addCondAlgo (alg)
    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.lock()

    print ('--- data')
    flags1 = flags.clone()
    flags1.Input.Files = defaultTestFiles.RAW_RUN2
    flags1.Input.ProjectName = 'data12_8TeV'
    flags1.lock()
    acc1 = BunchGroupCondAlgCfg (flags1)
    acc1.printCondAlgs(summariseProps=True)
    print ('IOVDbSvc:', acc1.getService('IOVDbSvc').Folders)
    acc1.wasMerged()

    print ('--- default')
    flags2 = flags.clone()
    flags2.Input.Files = defaultTestFiles.RAW_RUN2
    flags2.lock()
    acc2 = BunchGroupCondAlgCfg (flags2)
    acc2.printCondAlgs(summariseProps=True)
    print ('IOVDbSvc:', acc2.getServices())
    acc2.wasMerged()
