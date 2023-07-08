#!/bin/bash
# art-description: Generation test Powheg LHE-only ZZ
# art-type: build
# art-include: main/AthGeneration
# art-include: main--HepMC2/Athena
# art-include: 22.0/Athena
# art-output: *.root
# art-output: log.generate

## Any arguments are considered overrides, and will be added at the end
export TRF_ECHO=True;
rm *;
Gen_tf.py --ecmEnergy=13000 --jobConfig=421354 \
    --outputTXTFile=test_powheg_ZZ.TXT.tar.gz \

echo "art-result: $? generate"


