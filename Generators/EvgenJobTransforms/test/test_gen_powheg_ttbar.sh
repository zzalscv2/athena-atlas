#!/bin/bash
# art-description: Generation test Ph+Py8 ttbar using PDGParams and removing old rwt comments
# art-include: main/AthGeneration
# art-include: main--HepMC2/Athena
# art-include: 22.0/Athena
# art-type: build
# art-output: *.root
# art-output: log.generate
## Any arguments are considered overrides, and will be added at the end
export TRF_ECHO=True;
Gen_tf.py --ecmEnergy=13000 --jobConfig=421355 --maxEvents=100 \
    --outputEVNTFile=test_powheg_ttbar.EVNT.pool.root \

echo "art-result: $? generate"
    


