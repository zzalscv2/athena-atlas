#
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
#
# File: FastCaloSim/share/FastShowerCellBuilderTool_test.py
# Author: scott snyder
# Date: Nov, 2018
# Brief: Test for FastShowerCellBuilderTool.
#


import ROOT
ROOT.TH1F
try:
          from ROOT import HepMC3  as HepMC
          def  newGenParticlePtr(v, pdgid, status):
           return ROOT.std.make_shared[HepMC.GenParticle](v, pdgid, status)
          def  newGenVertexPtr():
           return ROOT.std.make_shared[HepMC.GenVertex]()
except ImportError:
          from ROOT import HepMC   as HepMC
          def  newGenParticlePtr(v, pdgid, status):
           return HepMC.GenParticle(v, pdgid, status)
          def  newGenVertexPtr():
           return HepMC.GenVertex()

from AthenaCommon.DetFlags      import DetFlags
DetFlags.detdescr.Tile_setOn()
DetFlags.detdescr.LAr_setOn()

RunNumber = 284500

import sys
import string
import ROOT
import math
from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import GeoModelInit
from AtlasGeoModel import SetupRecoGeometry
include('TileConditions/TileConditions_jobOptions.py')

# Disable Geant version checking.
from AthenaCommon.AlgSequence import AthSequencer
condSeq = AthSequencer("AthCondSeq")
condSeq.TileSamplingFractionCondAlg.G4Version = -1

from GeoModelSvc.GeoModelSvcConf import GeoModelSvc
ServiceMgr += GeoModelSvc()
theApp.CreateSvc += [ "GeoModelSvc"]
from AtlasGeoModel import TileGM
from AtlasGeoModel import LArGM   #LAr needed to get MBTS DD.

from IOVDbSvc.IOVDbSvcConf import IOVDbSvc
IOVDbSvc().GlobalTag = 'OFLCOND-RUN12-SDR-35'

import MagFieldServices.SetupField

from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

theApp.EvtMax=1

from FastCaloSim.FastCaloSimConf import CellInfoContainerCondAlg
topSequence += CellInfoContainerCondAlg("CellInfoContainerCondAlg")


exp_cells = {
    (0, -3,  1,  0,  2,  29) :   873.8,
    (0, -3,  1,  0,  2,  30) :   593.1,
    (0, -3,  1,  0,  3,  28) :  3003.6,
    (0, -3,  1,  0,  3,  29) : 83002.8,
    (0, -3,  1,  0,  3,  30) : 19794.6,
    (0, -3,  1,  0,  4,  28) :  1847.6,
    (0, -3,  1,  0,  4,  29) : 14195.6,
    (0, -3,  1,  0,  4,  30) :  4444.7,
    (0, -3,  1,  0,  5,  29) :   382.2,
    (0, -3,  1,  0,  5,  30) :   321.8,
    (0, -3,  2,  0,  3,  29) :   260.3,
    (0, -2,  0,  0,  0,  28) :   787.8,
    (0, -2,  1,  1,  1,  28) :   546.6,
    (0, -2,  1,  1,  2,  28) :  5108.6,
    (0, -2,  2,  1,  2, 113) :  1382.2,
    (0, -2,  2,  1,  2, 114) :  1784.3,
    (0, -1,  0,  0, 59,  28) :   515.6,
    (0, -1,  0,  0, 60,  28) :   520.8,
    (0,  1,  0,  0, 59,  25) :    69.6,
    (0,  1,  0,  0, 60,  25) :    22.92,
    (0,  2,  0,  0,  0,  24) :    34.7,
    (0,  2,  0,  0,  0,  25) :   191.5,
    (0,  2,  0,  0,  1,  25) :    30.1,
    (0,  2,  1,  1,  1,  25) :   572.2,
    (0,  2,  1,  1,  2,  25) : 12659.3,
    (0,  2,  2,  1,  1, 101) :   581.2,
    (0,  2,  2,  1,  2, 100) :  1557.8,
    (0,  2,  2,  1,  2, 101) :  9685.9,
    (0,  2,  2,  1,  2, 102) :  1053.3,
    (0,  2,  2,  1,  3, 101) :   716.1,
    (0,  2,  2,  1, 29, 149) :  1217.4,
    (0,  2,  2,  1, 29, 150) :  2093.2,
    (0,  2,  2,  1, 29, 151) :   461.8,
    (0,  2,  2,  1, 30, 148) :   492.1,
    (0,  2,  2,  1, 30, 149) :  6877.4,
    (0,  2,  2,  1, 30, 150) : 12497.6,
    (0,  2,  2,  1, 30, 151) :  1599.9,
    (0,  2,  2,  1, 31, 148) :   378.3,
    (0,  2,  2,  1, 31, 149) :  1359.0,
    (0,  2,  2,  1, 31, 150) :  2037.2,
    (0,  2,  2,  1, 31, 151) :   651.7,
    (0,  2,  2,  1, 32, 150) :   294.3,
    (0,  2,  3,  0, 13, 148) :   615.8,
    (0,  2,  3,  0, 13, 149) :  6715.2,
    (0,  2,  3,  0, 13, 150) : 13650.3,
    (0,  2,  3,  0, 13, 151) :  2502.6,
    (0,  2,  3,  0, 14, 148) :   487.0,
    (0,  2,  3,  0, 14, 149) :  1922.1,
    (0,  2,  3,  0, 14, 150) :  3057.3,
    (0,  2,  3,  0, 14, 151) :  1206.9,
    (1,  2,  0,  0,  5,  36) :   615.1,
    (1,  2,  0,  0,  5,  37) :   294.7,
    (1,  2,  0,  0,  6,  36) :   844.9,
    (1,  2,  0,  0,  6,  37) : 12346.6,
    (1,  2,  0,  0,  6,  38) :   513.5,
    (1,  2,  0,  0,  7,  36) :   760.9,
    (1,  2,  0,  0,  7,  37) :  3984.5,
    (1,  2,  0,  0,  7,  38) :  1196.1,
    (1,  2,  0,  0,  8,  37) :   482.5,
    (1,  2,  0,  0,  8,  38) :   369.4,
    (1,  2,  1,  0,  6,  36) :    66.3,
    (1,  2,  1,  0,  6,  37) :  1233.5,
    (1,  2,  1,  0,  6,  38) :   595.2,
    (1,  2,  1,  0,  7,  36) :   109.5,
    (1,  2,  1,  0,  7,  37) :  1353.6,
    (1,  2,  1,  0,  7,  38) :   178.6,
    (3,  3, -1, 28, 15,   3) :  4621.6,
    (3,  3, -1, 29, 15,   3) :   616.5,
    (3,  3,  1, 24, 15,   3) :  1138.1,
    (3,  3,  1, 25, 15,   3) :  5684.5,
    }


###########################################################

from AthenaPython.PyAthenaComps import Alg, StatusCode


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


class TestAlg (Alg):
    def __init__ (self, name):
        Alg.__init__ (self, name)
        return

    def initialize (self):
        ROOT.ICaloCellMakerTool

        def gettool (name):
            tool = ROOT.ToolHandle(ROOT.ICaloCellMakerTool)('FastShowerCellBuilderTool/' + name)
            if not tool.retrieve():
                assert 0
            return tool

        self.tool1 = gettool ('tool1')

        self.idhelper = self.detStore['CaloCell_ID']

        return StatusCode.Success


    def execute (self):
        self.record_mc()
        ctx = self.getContext()
        mgr = self.condStore['CaloDetDescrManager'].find (ctx.eventID())
        ccc = make_calo_cells (mgr)
        if not self.tool1.process (ccc, self.getContext()):
            return StatusCode.Failure

        self.compare_cells (ccc, exp_cells)

        return StatusCode.Success


    def add_particle (self, vtx, pdgid, pt, eta, phi, status=2):
        e = pt * math.cosh(eta)
        pz= pt * math.sinh(eta)
        v = HepMC.FourVector (pt*math.cos(phi), pt*math.sin(phi), pz, e)
        p=newGenParticlePtr(v, pdgid, status)
        vtx.add_particle_out (p)
        return


    def record_mc (self):
        vtx = newGenVertexPtr()
        self.add_particle (vtx,  22, 10*GeV, -1.5, 2.8)
        self.add_particle (vtx,  11, 15*GeV, -2.9, 2.9)
        self.add_particle (vtx, -11, 20*GeV,  1.5, 2.5)

        self.add_particle (vtx, 211, 30*GeV,  2.2, -2.6)
        self.add_particle (vtx,  22,  1*GeV,  0.1,  2.4)
        self.add_particle (vtx,  22, 40*GeV,  3.6,  1.5, 100)

        evt = HepMC.GenEvent()
        evt.add_vertex (vtx)
        mccoll = ROOT.McEventCollection()
        mccoll.push_back (evt)
        self.evtStore.record (mccoll, 'TruthEvent', False)
        return


    def region (self, cid):
        if self.idhelper.sub_calo(cid) == ROOT.CaloCell_ID.LARFCAL:
            return 0
        return self.idhelper.region(cid)


    def compare_cells (self, ccc, exp_cells):
        exp_cells = exp_cells.copy()
        idhelper  = self.idhelper
        tilehelper = idhelper.tile_idHelper()

        for c in ccc:
            lcell = [c.energy(), c.time(),
                     c.quality(), c.provenance(),
                     c.gain()]

            cid = c.ID()
            sub_calo = idhelper.sub_calo(cid)
            if sub_calo == 3:
                addr = (sub_calo,
                        tilehelper.section(cid),
                        tilehelper.side(cid),
                        tilehelper.module(cid),
                        tilehelper.tower(cid),
                        tilehelper.sampling(cid))
            else:
                addr = (sub_calo,
                        idhelper.pos_neg(cid),
                        idhelper.sampling(cid),
                        self.region(cid),
                        idhelper.eta(cid),
                        idhelper.phi(cid))

            l = exp_cells.get (addr)
            if not l:
                if abs(lcell[0]) > 500:
                    print ('xxx unexpected cell', addr, lcell)
                    assert 0
                continue

            if (abs ((lcell[0] - l)/l) > 1e-3 or
                abs (lcell[1]) > 1e-3 or
                lcell[2] != 0 or
                lcell[3] != 0 or
                lcell[4] != 0):
                print ('xxx cell mismatch: ', addr, lcell, l)
                assert 0
            del exp_cells[addr]

        for extra in exp_cells:
            print ('xxx unfound cell', extra)
            assert 0
        return


#########################################################################

from FastCaloSim.FastCaloSimFactory import FastCaloSimFactory

def maketool (name, **kw):
    return FastCaloSimFactory (name, **kw)
ToolSvc += maketool ('tool1')

testalg1 = TestAlg ('testalg1')
topSequence += testalg1


# Suppress useless GeoModelSvc messages.
from AthenaCommon import Constants
GeoModelSvc().OutputLevel=Constants.WARNING
