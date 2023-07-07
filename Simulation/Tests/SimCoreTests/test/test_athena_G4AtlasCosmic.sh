#!/bin/sh
#
# art-description: Run cosmics simulation outside ISF from the athena command-line, generating events on-the-fly
# art-include: 21.0/Athena
# art-include: 23.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: test.HITS.pool.root

athena G4AtlasApps/jobOptions.G4Cosmic.py

rc=$?
rc2=-9999
echo  "art-result: $rc simulation"
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries -1 ${ArtPackage} ${ArtJobName} --mode=semi-detailed
    rc2=$?
fi

echo  "art-result: $rc2 regression"
