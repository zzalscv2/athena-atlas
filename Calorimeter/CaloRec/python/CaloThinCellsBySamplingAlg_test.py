#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: CaloRec/python/CaloThinCellsBySamplingAlg_test.py
# Author: scott snyder
# Date: Nov, 2019
# Brief: Test for CaloThinCellsBySamplingAlg.
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaPython.PyAthenaComps import Alg, StatusCode
import ROOT


def make_calo_cells (mgr):
    ccc = ROOT.CaloCellContainer()
    for i in range (mgr.element_size()):
        elt = mgr.get_element (ROOT.IdentifierHash (i))
        if not elt: break
        cc = ROOT.CaloCell (elt, 0, 0, 0, 0)
        ccc.push_back (cc)
        ROOT.SetOwnership (cc, False)
    ccc.order()
    ccc.updateCaloIterators()
    return ccc


class CreateDataAlg (Alg):
    def execute (self):
        ctx = self.getContext()
        mgr = self.condStore['CaloDetDescrManager'].find (ctx.eventID())
        ccc = make_calo_cells (mgr)
        self.evtStore.record (ccc, 'AllCalo', False)
        return StatusCode.Success


class CheckThinningAlg (Alg):
    def execute (self):
        ctx = self.getContext()
        mgr = self.condStore['CaloDetDescrManager'].find (ctx.eventID())
        dec = self.evtStore['AllCalo_THINNED_StreamAOD.thinAlg']

        for i in range (dec.size()):
            elt = mgr.get_element (ROOT.IdentifierHash (i))
            if elt.getSampling() == 3 or elt.getSampling() == 17:
                assert not dec.thinned(i)
            else:
                assert dec.thinned(i)
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

    CaloThinCellsBySamplingAlg=CompFactory.CaloThinCellsBySamplingAlg
    result.addEventAlgo (CaloThinCellsBySamplingAlg ('thinAlg',
                                                     StreamName = 'StreamAOD',
                                                     SamplingCellsName = ['EMB3',
                                                                          'TileGap3']))

    result.addEventAlgo (CheckThinningAlg ('CheckThinningAlg'))
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
acc=MainServicesCfg(flags)

from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
acc.merge (McEventSelectorCfg (flags))

acc.merge (testCfg (flags))
acc.run(1)

    
