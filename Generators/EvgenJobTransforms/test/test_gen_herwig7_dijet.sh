#!/bin/bash
# art-description: Generation test H7 dijets
# art-include: main/AthGeneration
# art-include: main--HepMC2/Athena
# art-include: 22.0/Athena
# art-type: build
# art-output: *.root
# art-output: log.generate
## Any arguments are considered overrides, and will be added at the end
export TRF_ECHO=True;
Gen_tf.py --ecmEnergy=13000 --jobConfig=421106 --maxEvents=10 \
    --outputEVNTFile=test_herwig7_dijet.EVNT.pool.root \

echo "art-result: $? generate"


