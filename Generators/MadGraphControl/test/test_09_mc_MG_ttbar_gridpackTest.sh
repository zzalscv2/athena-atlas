#!/bin/sh

# art-include: 21.6/AthGeneration
# art-description: MadGraph Event Generation Test - LO Gridpack
# art-type: grid
# art-output: test_lhe_events.events
# art-output: output_hists.root

mkdir run_makeGridpack
cd run_makeGridpack

Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --firstEvent=1 --randomSeed=123456 --outputTXTFile=fake_lhe_events --jobConfig=950109 --outputFileValidation=False

echo "art-result: $? gridpack_creation"

cd ../
mkdir run_generateFromGridpack
cd run_generateFromGridpack

# In order to test out the new gridpack, we must run off of local files
cp -r /cvmfs/atlas.cern.ch/repo/sw/Generators/MCJobOptions/950xxx/950109/ .
# Wildcard match in order to check for errors in naming
cp ../run_makeGridpack/mc*.tar.gz 950109/

Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --firstEvent=1 --randomSeed=123456 --outputTXTFile=test_lhe_events --jobConfig=./950109

echo "art-result: $? Gen_tf"

simple_lhe_plotter.py test_lhe_events.events

echo "art-result: $? Plot"

cp output_hists.root test_lhe_events.events ../
