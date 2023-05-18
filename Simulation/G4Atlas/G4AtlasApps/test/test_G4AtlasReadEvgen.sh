#!/bin/sh
#
# art-description: G4Atlas Read Evgen example
# art-type: build
# art-include: 23.0/Athena
# art-include: 23.0/AthSimulation
# art-include: master/Athena
# art-include: master/AthSimulation

athena G4AtlasApps/jobOptions.G4Atlas_ReadEvgen.py

echo  "art-result: $? simulation"
