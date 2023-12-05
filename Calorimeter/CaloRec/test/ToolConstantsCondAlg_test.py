#!/usr/bin/env python
#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: CaloRec/python/ToolConstantsCondALg_test.py
# Author: scott snyder
# Date: Jun, 2020
# Brief: Test for ToolConstantsCondAlg.
#


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def testCfg (flags):
    result = ComponentAccumulator()

    from IOVDbSvc.IOVDbSvcConfig import addFolders
    result.merge (addFolders (flags,
                              '/LAR/CellCorrOfl/deadOTX',
                              detDb = 'LAR_OFL',
                              className = 'CondAttrListCollection'))

    from CaloRec.ToolConstantsCondAlgConfig import ToolConstantsCondAlgCfg
    result.merge (ToolConstantsCondAlgCfg (flags,
                                           'deadOTXCorrCtes',
                                           COOLFolder='/LAR/CellCorrOfl/deadOTX'))

    from EventSelectorAthenaPool.CondProxyProviderConfig import CondProxyProviderCfg
    from CaloClusterCorrection.poolfiles import poolfiles
    result.merge (CondProxyProviderCfg (flags,
                                        poolFiles = [poolfiles['caloswcorr_pool_v22']]))
    result.merge (ToolConstantsCondAlgCfg (flags,
                                           'CaloSwClusterCorrections.rfac-v5',
                                           DetStoreKey='CaloSwClusterCorrections.rfac-v5'))

    CaloClusterCorrDumper = CompFactory.CaloClusterCorrDumper # CaloRec
    alg = CaloClusterCorrDumper ('dumper1',
                                 Constants = ['deadOTXCorrCtes',
                                              'CaloSwClusterCorrections.rfac-v5'])
    result.addEventAlgo (alg)
    return result


from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.TestDefaults import defaultTestFiles
flags = initConfigFlags()
flags.Input.Files = defaultTestFiles.RDO_RUN2
flags.Input.TimeStamps = [1000]

flags.lock()
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg (flags)

from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
acc.merge (McEventSelectorCfg (flags))

acc.merge (testCfg (flags))
acc.run(1)
