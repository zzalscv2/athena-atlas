#!/bin/bash
# art-description: Generation test Epos min_bias 
# art-include: main/AthGeneration
# art-include: main--HepMC2/Athena
# art-include: 22.0/Athena
# art-type: build
# art-output: *.root
# art-output: log.generate
## Any arguments are considered overrides, and will be added at the end
export TRF_ECHO=True;
Gen_tf.py --ecmEnergy=13000 --maxEvents=10 \
    --jobConfig=421102  \
    --outputEVNTFile=test_epos_minbias_inelastic.EVNT.pool.root \

echo "art-result: $? generate"


