# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# File: CoolLumiUtilities/python/BunchLumisCondAlgConfig.py
# Created: May 2019, sss
# Purpose: Configure BunchLumisCondAlg.
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from IOVDbSvc.IOVDbSvcConfig import addFolders


def BunchLumisCondAlgCfg (flags):
    name = 'BunchLumisCondAlg'
    result = ComponentAccumulator()

    # Should only be used for Run 1.
    if flags.IOVDb.DatabaseInstance != 'COMP200':
        return result

    folder = '/TDAQ/OLC/BUNCHLUMIS'
    if flags.Common.isOverlay:
        # Load reduced channel list for overlay jobs to try to reduce COOL access
        # Need Lucid AND, OR, HitOR, BcmH OR, BcmV OR
        folder = '<channelSelection>101,102,103,201,211</channelSelection> ' + folder

    result.merge (addFolders (flags, folder, 'TDAQ',
                              className='CondAttrListCollection'))

    from CoolLumiUtilities.FillParamsCondAlgConfig import FillParamsCondAlgCfg
    result.merge (FillParamsCondAlgCfg(flags))
    fpalg = result.getCondAlgo ('FillParamsCondAlg')

    BunchLumisCondAlg=CompFactory.BunchLumisCondAlg
    alg = BunchLumisCondAlg (name,
                             BunchLumisFolderInputKey = folder,
                             FillParamsInputKey = fpalg.FillParamsOutputKey,
                             BunchLumisOutputKey = 'BunchLumisCondData')

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
    acc1 = BunchLumisCondAlgCfg (flags1)
    acc1.printCondAlgs(summariseProps=True)
    print ('IOVDbSvc:', acc1.getService('IOVDbSvc').Folders)
    acc1.wasMerged()

    print ('--- data+overlay')
    flags2 = initConfigFlags()
    flags2.Input.Files = defaultTestFiles.RAW_RUN2
    flags2.Input.ProjectName = 'data12_8TeV'
    flags2.Common.ProductionStep = ProductionStep.Overlay
    flags2.lock()
    acc2 = BunchLumisCondAlgCfg (flags2)
    acc2.printCondAlgs(summariseProps=True)
    print ('IOVDbSvc:', acc2.getService('IOVDbSvc').Folders)
    acc2.wasMerged()

    print ('--- default')
    flags3 = initConfigFlags()
    flags3.Input.Files = defaultTestFiles.RAW_RUN2
    flags3.lock()
    acc3 = BunchLumisCondAlgCfg (flags3)
    acc3.printCondAlgs(summariseProps=True)
    print ('IOVDbSvc:', acc3.getServices())
    acc3.wasMerged()
