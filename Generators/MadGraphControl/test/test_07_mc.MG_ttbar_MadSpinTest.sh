#!/bin/sh

# art-include: 21.6/AthGeneration
# art-description: MadGraph Event Generation Test - LO MadSpin
# art-type: grid

set -e
 
mkdir 999999
get_files -jo mc.MG_ttbar_MadSpinTest.py
mv mc.*py 999999/

Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --runNumber=999999 --firstEvent=1 --randomSeed=123456 --outputEVNTFile=EVNT.root --jobConfig=./999999

echo "art-result: $?"
