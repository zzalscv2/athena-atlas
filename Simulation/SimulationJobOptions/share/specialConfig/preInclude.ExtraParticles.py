#########################################################
#
# SimulationJobOptions/preInclude.ExtraParticles.py
# author: Miha Muskinja
# date:   August 2019
#
# adds extra particles to the Geant4 particle list
# particles are read from the PDGTABLE
#
#
#########################################################
include.block("SimulationJobOptions/preInclude.ExtraParticles.py")

## Add the physics tool
simFlags.PhysicsOptions += ['ExtraParticlesPhysicsTool']
# Trigger the creation of the PDGParser instance before any susy/exotics particles are added to the PDGTABLE.MeV file
from ExtraParticles.ExtraParticlesConfigLegacy import pdgParser # noqa: F401

## Add the additional ExtraParticles to the white list
from G4AtlasApps.SimFlags import SimFlags
if hasattr(simFlags, 'ParticleSimWhiteList'):
    SimFlags.ParticleSimWhiteList.set_Value_and_Lock('ISF_ParticleSimWhiteList_ExtraParticles')
