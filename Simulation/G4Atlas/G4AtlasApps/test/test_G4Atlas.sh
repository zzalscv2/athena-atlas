#!/bin/sh
#
# art-description: G4Atlas ParticleGun example
# art-type: build
# art-include: 23.0/Athena
# art-include: master/Athena

athena G4AtlasApps/jobOptions.G4Atlas.py

echo  "art-result: $? simulation"
