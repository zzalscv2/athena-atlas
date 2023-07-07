#!/bin/bash
# art-description: Generation test MG+Py8 TT MET or Lepton Filter
# art-include: main/AthGeneration
# art-include: main--HepMC2/Athena
# art-include: 22.0/Athena
# art-type: build
# art-output: *.root
# art-output: log.generate

## Any arguments are considered overrides, and will be added at the end
export TRF_ECHO=True;
rm *;
Gen_tf.py --ecmEnergy=13000 --jobConfig=421343 --maxEvents=10 \
    --outputEVNTFile=test_mgpythia8_TT_Filters.EVNT.pool.root \

echo "art-result: $? generate"


