#!/bin/bash
# art-description: Generation test Powheg LHE-only single top s-channel
# art-type: build
# art-include: main/AthGeneration
# art-include: main--HepMC2/Athena
# art-include: 22.0/Athena
# art-output: *.root
# art-output: log.generate

## Any arguments are considered overrides, and will be added at the end
export TRF_ECHO=True;
rm *;
Gen_tf.py --ecmEnergy=13600 --jobConfig=421358 \
    --outputTXTFile=test_powheg_t.TXT.tar.gz \

echo "art-result: $? generate"


