#!/bin/bash
# art-description: Generation test Pythia8 with two jet filters QCDTruthJetJZ2 with R=0.6 and MultiBJetFilter with R=0.4 
# art-type: build
# art-include: main/AthGeneration
# art-include: main--HepMC2/Athena
# art-include: 22.0/Athena
# art-output: *.root
# art-output: log.generate

## Any arguments are considered overrides, and will be added at the end
export TRF_ECHO=True;
Gen_tf.py --ecmEnergy=13000 --jobConfig=421126 --maxEvents=10 \
    --outputEVNTFile=test_Py8JZ2MultiBJetFilt.EVNT.pool.root \

echo "art-result: $? generate"


