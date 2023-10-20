#!/bin/sh
#
# art-description: MC23-style RUN2 simulation using 21.0-compatible geometry and ATLFAST3_QS
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 4
# art-output: test.HITS.pool.root
# art-output: truth.root

# MC16 setup
# ATLAS-R2-2016-01-00-01 and OFLCOND-MC23-SDR-RUN3-01

unset ATHENA_CORE_NUMBER

Sim_tf.py \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --simulator 'ATLFAST3_QS' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC23aSimulationSingleIoV.py' \
    --DataRunNumber '284500' \
    --geometryVersion 'default:ATLAS-R2-2016-01-00-01' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
    --outputHITSFile "test.HITS.pool.root" \
    --maxEvents 20 \
    --imf False

rc=$?
rc2=-9999
echo  "art-result: $rc simOLD"
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 4 ${ArtPackage} ${ArtJobName} --mode=semi-detailed
    rc2=$?
fi

echo  "art-result: $rc2 regression"
