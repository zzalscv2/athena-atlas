#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: CaloRec/python/CaloCellContainerAliasAlg_test.py
# Author: scott snyder
# Date: Nov, 2019
# Brief: Test for CaloCellContainerAliasAlg.
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaPython.PyAthenaComps import Alg, StatusCode
import ROOT


class CreateDataAlg (Alg):
    def execute (self):
        ccc = ROOT.CaloCellContainer()
        self.evtStore.record (ccc, 'AllCalo', False)
        return StatusCode.Success


class CheckAliasAlg (Alg):
    def execute (self):
        ccc1 = self.evtStore['AllCalo']
        ccc2 = self.evtStore['CellAlias']
        assert (repr(ccc1) == repr(ccc2))
        return StatusCode.Success


def testCfg (flags):
    result = ComponentAccumulator()

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    from TileGeoModel.TileGMConfig import TileGMCfg
    result.merge(LArGMCfg(flags))
    result.merge(TileGMCfg(flags))

    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
    result.merge(LArOnOffIdMappingCfg(flags))

    result.addEventAlgo (CreateDataAlg ('CreateDataAlg'))

    CaloCellContainerAliasAlg=CompFactory.CaloCellContainerAliasAlg
    result.addEventAlgo (CaloCellContainerAliasAlg ('aliasAlg',
                                                    Cells = 'AllCalo',
                                                    Alias = 'CellAlias'))

    result.addEventAlgo (CheckAliasAlg ('CheckAliasAlg'))
    return result


from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.TestDefaults import defaultTestFiles
flags = initConfigFlags()
flags.Input.Files = defaultTestFiles.RDO_RUN2
flags.Input.TimeStamps = [1000]
flags.Detector.GeometryLAr = True
flags.Detector.GeometryTile = True
flags.needFlagsCategory('Tile')
flags.needFlagsCategory('LAr')

flags.lock()
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg(flags)

from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
acc.merge (McEventSelectorCfg (flags))

acc.merge (testCfg (flags))
acc.run(1)
