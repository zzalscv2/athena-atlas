# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# File: CoolLumiUtilities/python/FillParamsCondAlgConfig.py
# Created: May 2019, sss
# Purpose: Configure FillParamsCondAlg.
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from IOVDbSvc.IOVDbSvcConfig import addFolders

def FillParamsCondAlgCfg (flags):
    name = 'FillParamsCondAlg'
    result = ComponentAccumulator()

    # Should only be used for Run 1.
    if flags.IOVDb.DatabaseInstance != 'COMP200':
        return result

    folder = '/TDAQ/OLC/LHC/FILLPARAMS'
    # Mistakenly created as multi-version folder, must specify HEAD 
    result.merge (addFolders (flags, folder, 'TDAQ', tag='HEAD',
                              className='AthenaAttributeList'))

    FillParamsCondAlg=CompFactory.FillParamsCondAlg
    alg = FillParamsCondAlg (name,
                             FillParamsFolderInputKey = folder,
                             FillParamsOutputKey = 'FillParamsCondData')

    result.addCondAlgo (alg)
    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    print ('--- data')
    flags1 = initConfigFlags()
    flags1.Input.Files = defaultTestFiles.RAW_RUN2
    flags1.Input.ProjectName = 'data12_8TeV'
    flags1.lock()
    acc1 = FillParamsCondAlgCfg (flags1)
    acc1.printCondAlgs(summariseProps=True)
    print ('IOVDbSvc:', acc1.getService('IOVDbSvc').Folders)
    acc1.wasMerged()

    print ('--- default')
    flags2 = initConfigFlags()
    flags2.Input.Files = defaultTestFiles.RAW_RUN2
    flags2.lock()
    acc2 = FillParamsCondAlgCfg (flags2)
    acc2.printCondAlgs(summariseProps=True)
    print ('IOVDbSvc:', acc2.getServices())
    acc2.wasMerged()
