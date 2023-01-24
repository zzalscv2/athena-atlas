# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from GeneratorModules.EvgenAlg import EvgenAlg
from ParticleGun.samplers import ParticleSampler
from ParticleGun.samplers import * # noqa: F401, F403 (import into our namespace)
# commenting out the HepMC import for now
#try:
#          from AthenaPython.PyAthena import HepMC3  as HepMC
#except ImportError:
#          from AthenaPython.PyAthena import HepMC   as HepMC  

from AthenaPython.PyAthena import StatusCode
import ROOT,random

__author__ = "Andy Buckley <andy.buckley@cern.ch>"

class ParticleGun(EvgenAlg):
    """
    A simple but flexible algorithm for generating events from simple distributions.
    """

    def __init__(self, name="ParticleGun", randomStream="ParticleGun", randomSeed=None):
        super(ParticleGun, self).__init__(name=name)
        self.samplers = [ParticleSampler()]
        self.randomStream = randomStream
        self.randomSeed = randomSeed

    @property
    def sampler(self):
        "Get the first (and presumed only) sampler"
        return self.samplers[0] if self.samplers else None
    @sampler.setter
    def sampler(self, s):
        "Set the samplers list to include only a single sampler, s"
        self.samplers = [s]


    def initialize(self):
        return StatusCode.Success


    def fillEvent(self, evt):
        """
        Sample a list of particle properties, which are then used to create a new GenEvent in StoreGate.
        """
        # set the random seed
        offset = self.randomSeed if self.randomSeed is not None else 0
        seed = ROOT.ATHRNG.calculateSeedsPython(self.randomStream, self._ctx.eventID().event_number(), self._ctx.eventID().run_number(), offset)

        if seed is None:
            self.msg.warning("ParticleGun: Failed to find a seed for the random stream named '%s' ", self.randomStream)
            seed = self.randomSeed
        else:
            self.msg.debug("ParticleGun: set random seed to, %s", str(seed))
        if seed is not None:
            random.seed(seed)
            self.msg.debug("ParticleGun: randomSeed property set")
        else:
            self.msg.error("ParticleGun: randomSeed property not set")
            return StatusCode.Failure

        ## Set event weight(s)
        # TODO: allow weighted sampling?
        try:
          from AthenaPython.PyAthena import HepMC3  as HepMC
          evt.set_units(HepMC.Units.MEV, HepMC.Units.MM)
        except ImportError:
          from AthenaPython.PyAthena import HepMC   as HepMC
        evt.weights().push_back(1.0)

        ## Make and fill particles
        for s in self.samplers:
            particles = s.shoot()
            for p in particles:
                ## Debug printout of particle properties
                #print("DEBUG0 ", p.pid, p.mom.E(), p.mom.Pt(), p.mom.M())
                #print "DEBUG1 (px,py,pz,E) = (%0.2e, %0.2e, %0.2e, %0.2e)" % (p.mom.Px(), p.mom.Py(), p.mom.Pz(), p.mom.E())
                #print "DEBUG2 (eta,phi,pt,m) = (%0.2e, %0.2e, %0.2e, %0.2e)" % (p.mom.Eta(), p.mom.Phi(), p.mom.Pt(), p.mom.M())
                #print "DEBUG3 (x,y,z,t) = (%0.2e, %0.2e, %0.2e, %0.2e)" % (p.pos.X(), p.pos.Y(), p.pos.Z(), p.pos.T())

                ## Make particle-creation vertex
                # TODO: do something cleverer than one vertex per particle?
                pos = HepMC.FourVector(p.pos.X(), p.pos.Y(), p.pos.Z(), p.pos.T())
                gv = HepMC.GenVertex(pos)
                ROOT.SetOwnership(gv, False)
                evt.add_vertex(gv)

                ## Make particle with status == 1
                mom = HepMC.FourVector(p.mom.Px(), p.mom.Py(), p.mom.Pz(), p.mom.E())
                gp = HepMC.GenParticle()
                gp.set_status(1)
                gp.set_pdg_id(p.pid)
                gp.set_momentum(mom)
                if p.mass is not None:
                    gp.set_generated_mass(p.mass)
                ROOT.SetOwnership(gp, False)
                gv.add_particle_out(gp)

        return StatusCode.Success


## PyAthena HepMC notes
#
## evt.print() isn't valid syntax in Python2 due to reserved word
# TODO: Add a Pythonisation, e.g. evt.py_print()?
#getattr(evt, 'print')()
#
## How to check that the StoreGate key exists and is an McEventCollection
# if self.sg.contains(McEventCollection, self.sgkey):
#     print self.sgkey + " found!"
#
## Modifying an event other than that supplied as an arg
# mcevts = self.sg[self.sgkey]
# for vtx in mcevts[0].vertices: # only way to get the first vtx?!
#     gp2 = HepMC.GenParticle()
#     gp2.set_momentum(HepMC.FourVector(1,2,3,4))
#     gp2.set_status(1)
#     vtx.add_particle_out(gp2)
#     break
