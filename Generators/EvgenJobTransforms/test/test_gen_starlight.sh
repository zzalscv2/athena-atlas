#!/bin/bash
# art-description: Generation test Starlight
# art-type: build
# art-include: main/AthGeneration
# art-include: main--HepMC2/Athena
# art-include: 22.0/Athena
# art-output: *.root
# art-output: log.generate
 
## Any arguments are considered overrides, and will be added at the end
export TRF_ECHO=True;
Gen_tf.py --ecmEnergy=5020 --jobConfig=421120 --maxEvents=10 \
    --outputEVNTFile=test_starlight_gammagamma2ee.EVNT.pool.root \

echo "art-result: $? generate"




