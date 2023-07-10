#!/bin/bash
# art-description: Generation test Pythia8B Jpsimumu Zmumu 
# art-include: main/AthGeneration
# art-include: main--HepMC2/Athena
# art-include: 22.0/Athena
# art-type: build
# art-output: *.root
# art-output: log.generate
## Any arguments are considered overrides, and will be added at the end
export TRF_ECHO=True;
Gen_tf.py --ecmEnergy=13600 --maxEvents=10 --jobConfig=421101 \
    --outputEVNTFile=test_bb_Jpsimu4mu4X.EVNT.pool.root \

echo "art-result: $? generate"

