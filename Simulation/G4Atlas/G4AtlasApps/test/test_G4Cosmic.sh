#!/bin/sh
#
# art-description: G4Cosmic example
# art-type: build
# art-include: 23.0/Athena
# art-include: master/Athena

athena G4AtlasApps/jobOptions.G4Cosmic.py

echo  "art-result: $? simulation"
