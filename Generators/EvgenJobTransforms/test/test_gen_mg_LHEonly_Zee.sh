#!/bin/bash
# art-description: Generation test MadGraph LHE-only for Z->ee
# art-type: build
# art-include: main/AthGeneration
# art-include: main--HepMC2/Athena
# art-include: 22.0/Athena
# art-output: *.root
# art-output: log.generate

## Any arguments are considered overrides, and will be added at the end
export TRF_ECHO=True;
rm *;
Gen_tf.py --ecmEnergy=13000 --jobConfig=421005 --maxEvents=100 \
    --outputTXTFile=test_mg_Zee.TXT.tar.gz \

echo "art-result: $? generate"


