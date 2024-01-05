#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: CaloRec/python/CaloThinCellsByClusterAlg_test.py
# Author: scott snyder
# Date: Nov, 2019
# Brief: Test for CaloThinCellsByClusterAlg.
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaPython.PyAthenaComps import Alg, StatusCode
from math import pi
import ROOT


cell_hashes = set()


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


def make_clusters (mgr, ccc, hashes):
    clc = ROOT.xAOD.CaloClusterContainer()
    clc_store = ROOT.xAOD.CaloClusterAuxContainer()
    clc.setStore (clc_store)

    ids = ROOT.vector(ROOT.IdentifierHash)()

    for i in range(2):
        cl = ROOT.xAOD.CaloCluster()
        clc.push_back (cl)
        ROOT.SetOwnership (cl, False)
        eta = 0.5 - i # 0.5 or -0.5
        phi = 1
        cl.setEta (eta)
        cl.setPhi (phi)
        cl.setClusterSize (ROOT.xAOD.CaloCluster.SW_37ele)
        links = ROOT.CaloClusterCellLink (ccc)
        cl.addCellLink (links)
        ROOT.SetOwnership (links, False)

        mgr.cellsInZone (eta - 0.05, eta + 0.05, phi - 0.05, phi + 0.05, ids)
        for hash in ids:
            elt = mgr.get_element (hash)
            s = elt.getSampling()
            if s == 0 or s == 2:
                idx = ccc.findIndex (hash)
                if idx < 0:
                    print ("??? Can't find cell with hash ", hash)
                else:
                    hashes.add (hash.value())
                    cl.addCell (idx, 1)

    return (clc, clc_store)


class CreateDataAlg (Alg):
    def execute (self):
        ctx = self.getContext()
        mgr = self.condStore['CaloDetDescrManager'].find (ctx.eventID())
        ccc = make_calo_cells (mgr)
        self.evtStore.record (ccc, 'AllCalo', False)

        global cell_hashes
        cell_hashes = set()
        (clc, clc_store) = make_clusters (mgr, ccc, cell_hashes)
        self.evtStore.record (clc, 'Clusters', False)
        self.evtStore.record (clc_store, 'ClustersAux.', False)
        return StatusCode.Success


class CheckThinningAlg (Alg):
    def isCloseTo (self, elt, eta, phi):
        # Assuming no phi wrapping.
        return (abs (elt.eta() - eta) < 3*0.025 and
                abs (elt.phi() - phi) < 7*2*pi/256)

    def execute (self):
        ctx = self.getContext()
        mgr = self.condStore['CaloDetDescrManager'].find (ctx.eventID())
        dec = self.evtStore['AllCalo_THINNED_StreamAOD.thinAlg']

        global cell_hashes
        for i in range (dec.size()):
            elt = mgr.get_element (ROOT.IdentifierHash (i))
            if elt.getSampling() == 3:
                close = (self.isCloseTo (elt,  0.5, 1) or
                         self.isCloseTo (elt, -0.5, 1))
                if not dec.thinned(i):
                    assert close
                else:
                    assert not close
            else:
                if not dec.thinned(i):
                    assert i in cell_hashes
                else:
                    assert i not in cell_hashes
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

    CaloThinCellsByClusterAlg=CompFactory.CaloThinCellsByClusterAlg
    result.addEventAlgo (CaloThinCellsByClusterAlg ('thinAlg',
                                                    StreamName = 'StreamAOD',
                                                    Clusters = 'Clusters',
                                                    SamplingCellsName = ['EMB3']))

    result.addEventAlgo (CheckThinningAlg ('CheckThinningAlg'))
    return result


# Work around issue with cling in root 6.20.06 getting confused
# by forward declarations.
ROOT.xAOD.CaloClusterContainer_v1


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
