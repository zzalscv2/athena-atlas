#!/bin/sh
#
# art-description: Recursive geometry test on ATLAS-R2-2015-03-01-00 (MC15 default). Done only on G4 and GeoModel envelopes
# art-include: 21.0/Athena
# art-include: 23.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'

athena G4AtlasTests/test_G4AtlasGeo.py

echo  "art-result: $? simulation"

#TODO need to add a test to grep log files.
