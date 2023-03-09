# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# File: LumiBlockComps/python/TrigLiveFractionCondAlgConfig.py
# Created: Jun 2019, sss, from existing TrigLivefractionToolDefault.
# Purpose: Configure TrigLiveFractionCondAlg.
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from IOVDbSvc.IOVDbSvcConfig import addFolders


# Configuration for TrigLiveFractionCondAlg
def TrigLiveFractionCondAlgCfg (flags):
    name = 'TrigLiveFractionCondAlg'
    result = ComponentAccumulator()

    kwargs = {}
    if flags.IOVDb.DatabaseInstance == 'COMP200':
        folder = '/TRIGGER/LUMI/PerBcidDeadtime'

        # Mistakenly created as multi-version folder, must specify HEAD
        result.merge (addFolders (flags, folder, 'TRIGGER', tag='HEAD',
                                  className='AthenaAttributeList'))

        kwargs['DeadtimeFolderInputKey'] = folder
        kwargs['LuminosityInputKey'] = 'LuminosityCondData'

        from LumiBlockComps.LuminosityCondAlgConfig import LuminosityCondAlgCfg
        result.merge (LuminosityCondAlgCfg (flags))

    else:
        kwargs['DeadtimeFolderInputKey'] = ''
        kwargs['LuminosityInputKey'] = ''


    TrigLiveFractionCondAlg=CompFactory.TrigLiveFractionCondAlg
    alg = TrigLiveFractionCondAlg (name,
                                   TrigLiveFractionOutputKey = 'TrigLiveFractionCondData',
                                   **kwargs)
    result.addCondAlgo (alg)
    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    print ('--- run2')
    flags1 = initConfigFlags()
    flags1.Input.Files = defaultTestFiles.RAW_RUN2
    flags1.lock()
    acc1 = TrigLiveFractionCondAlgCfg (flags1)
    acc1.printCondAlgs(summariseProps=True)
    acc1.wasMerged()

    print ('--- run1')
    flags3 = initConfigFlags()
    flags3.Input.Files = defaultTestFiles.RAW_RUN2
    flags3.Input.ProjectName = 'data12_8TeV'
    flags3.lock()
    acc3 = TrigLiveFractionCondAlgCfg (flags3)
    acc3.printCondAlgs(summariseProps=True)
    print ('IOVDbSvc:', acc3.getService('IOVDbSvc').Folders)
    acc3.wasMerged()

    print ('--- mc')
    flags4 = initConfigFlags()
    flags4.Input.Files = defaultTestFiles.ESD
    flags4.lock()
    acc4 = TrigLiveFractionCondAlgCfg (flags4)
    acc4.printCondAlgs(summariseProps=True)
    acc4.wasMerged()
