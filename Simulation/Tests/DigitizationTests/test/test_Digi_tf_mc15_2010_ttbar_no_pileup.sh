#!/bin/bash
#
# art-description: Run digitization of an MC15 ttbar sample with 2010 geometry and conditions, without pile-up
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: mc15_2010_ttbar_no_pileup.RDO.pool.root
# art-output: mc15_2010_ttbar_no_pileup.CA.RDO.pool.root
# art-output: ConfigDigi*.pkl

DigiOutFileNameCG="mc15_2010_ttbar_no_pileup.RDO.pool.root"
DigiOutFileNameCA="mc15_2010_ttbar_no_pileup.CA.RDO.pool.root"

Digi_tf.py \
    --CA True \
    --inputHITSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DigitizationTests/ttbar.ATLAS-R1-2010-02-00-00.HITS.pool.root \
    --outputRDOFile ${DigiOutFileNameCA} \
    --maxEvents 25 \
    --skipEvents 0  \
    --digiSeedOffset1=11 \
    --digiSeedOffset2=22 \
    --geometryVersion ATLAS-R1-2010-02-00-00 \
    --conditionsTag OFLCOND-RUN12-SDR-31-01 \
    --DataRunNumber 155697 \
    --preExec 'HITtoRDO:flags.Beam.NumberOfCollisions=0;flags.Sim.TRTRangeCut=0.05' \
    --preInclude 'default:LArConfiguration.LArConfigRun1.LArConfigRun1NoPileUp' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --postExec 'with open("ConfigDigiCA.pkl", "wb") as f: cfg.store(f)'

rc=$?
status=$rc
echo "art-result: $rc digiCA"

Digi_tf.py \
    --inputHITSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DigitizationTests/ttbar.ATLAS-R1-2010-02-00-00.HITS.pool.root \
    --outputRDOFile ${DigiOutFileNameCG} \
    --maxEvents 25 \
    --skipEvents 0  \
    --digiSeedOffset1=11 \
    --digiSeedOffset2=22 \
    --geometryVersion ATLAS-R1-2010-02-00-00 \
    --conditionsTag OFLCOND-RUN12-SDR-31-01 \
    --DataRunNumber 155697 \
    --preExec 'HITtoRDO:from Digitization.DigitizationFlags import digitizationFlags;digitizationFlags.TRTRangeCut.set_Value_and_Lock(0.05);' \
    --preInclude 'default:LArConfiguration/LArConfigRun1Old_NoPileup.py' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --athenaopts '"--config-only=ConfigDigiCG.pkl"'

Digi_tf.py \
    --inputHITSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DigitizationTests/ttbar.ATLAS-R1-2010-02-00-00.HITS.pool.root \
    --outputRDOFile ${DigiOutFileNameCG} \
    --maxEvents 25 \
    --skipEvents 0  \
    --digiSeedOffset1=11 \
    --digiSeedOffset2=22 \
    --geometryVersion ATLAS-R1-2010-02-00-00 \
    --conditionsTag OFLCOND-RUN12-SDR-31-01 \
    --DataRunNumber 155697 \
    --preExec 'HITtoRDO:from Digitization.DigitizationFlags import digitizationFlags;digitizationFlags.TRTRangeCut.set_Value_and_Lock(0.05);' \
    --preInclude 'default:LArConfiguration/LArConfigRun1Old_NoPileup.py' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py'

rc1=$?
if [ $status -eq 0 ]
then
   status=$rc1
fi
echo "art-result: $rc1 digiOLD"

rc2=-9999
if [ $status -eq 0 ]
then
   acmd.py diff-root ${DigiOutFileNameCG} ${DigiOutFileNameCA} --mode  semi-detailed --error-mode resilient
   rc2=$?
   if [ $status -eq 0 ]
   then
       status=$rc2
    fi
fi
echo "art-result: $rc2 OLDvsCA"

rc3=-9999
rc4=-9999
rc5=-9999

# get reference directory
source DigitizationCheckReferenceLocation.sh
echo "Reference set being used: ${DigitizationTestsVersion}"

if [ $rc1 -eq 0 ]
then
    # Do reference comparisons
    art.py compare ref --mode=semi-detailed --no-diff-meta "$DigiOutFileNameCG" "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DigitizationTests/ReferenceFiles/$DigitizationTestsVersion/$CMTCONFIG/$DigiOutFileNameCG"
    rc3=$?
    if [ $status -eq 0 ]
    then
       status=$rc3
    fi
fi
echo "art-result: $rc3 OLDvsFixedRef"

if [ $rc1 -eq 0 ]
then
    checkFile "$DigiOutFileNameCG"
    rc4=$?
    if [ $status -eq 0 ]
    then
       status=$rc4
    fi
fi
echo "art-result: $rc4 checkFile"

if [ $rc1 -eq 0 ]
then
    art.py compare grid --entries 10 "$1" "$2" --mode=semi-detailed --file="$DigiOutFileNameCG"
    rc5=$?
    if [ $status -eq 0 ]
    then
       status=$rc5
    fi
fi
echo "art-result: $rc5 regression"

exit $status
