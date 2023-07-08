#!/bin/bash
#
# art-description: Run a digitization example to compare configuration between ConfGetter and the new ComponentAccumulator approach.
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-include: 22.0/Athena
# art-include: 23.0/Athena
# art-include: main/Athena
# art-output: mc21a_presampling.VarBSTruth.CG.RDO.pool.root
# art-output: mc21a_presampling.VarBSTruth.CA.RDO.pool.root
# art-output: log.*
# art-output: legacy.*
# art-output: DigiPUConfig*

if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

Events=2000
DigiOutFileNameCG="mc21a_presampling.VarBSTruth.CG.RDO.pool.root"
DigiOutFileNameCA="mc21a_presampling.VarBSTruth.CA.RDO.pool.root"
HSHitsFile="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc21/HITS/mc21_13p6TeV.900149.PG_single_nu_Pt50.simul.HITS.e8453_s3864/HITS.29241942._001453.pool.root.1"
HighPtMinbiasHitsFiles1="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc21/HITS/mc21_13p6TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8453_e8455_s3876_s3880/*"
HighPtMinbiasHitsFiles2="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc21/HITS/mc21_13p6TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8453_e8455_s3877_s3880/*"
HighPtMinbiasHitsFiles3="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc21/HITS/mc21_13p6TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8453_e8455_s3878_s3880/*"
HighPtMinbiasHitsFiles4="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc21/HITS/mc21_13p6TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8453_e8455_s3879_s3880/*"
LowPtMinbiasHitsFiles1="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc21/HITS/mc21_13p6TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8453_s3876_s3880/*"
LowPtMinbiasHitsFiles2="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc21/HITS/mc21_13p6TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8453_s3877_s3880/*"
LowPtMinbiasHitsFiles3="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc21/HITS/mc21_13p6TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8453_s3878_s3880/*"
LowPtMinbiasHitsFiles4="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc21/HITS/mc21_13p6TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8453_s3879_s3880/*"


# config only
Digi_tf.py \
    --detectors Truth \
    --PileUpPresampling True \
    --splitConfig 'HITtoRDO:Campaigns.BeamspotSplitMC21a' \
    --conditionsTag default:OFLCOND-MC21-SDR-RUN3-07 \
    --digiSeedOffset1 170 --digiSeedOffset2 170 \
    --digiSteeringConf 'StandardSignalOnlyTruth' \
    --geometryVersion default:ATLAS-R3S-2021-03-00-00 \
    --inputHITSFile ${HSHitsFile} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles1} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles2} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles3} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles4} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles1} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles2} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles3} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles4} \
    --jobNumber 1 \
    --maxEvents ${Events} \
    --outputRDOFile ${DigiOutFileNameCA} \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'HITtoRDO:Campaigns/PileUpPresamplingMC21a.py' \
    --skipEvents 0 \
    --athenaopts '"--config-only=DigiPUConfigCG.pkl"'

# full run
Digi_tf.py \
    --detectors Truth \
    --PileUpPresampling True \
    --splitConfig 'HITtoRDO:Campaigns.BeamspotSplitMC21a' \
    --conditionsTag default:OFLCOND-MC21-SDR-RUN3-07 \
    --digiSeedOffset1 170 --digiSeedOffset2 170 \
    --digiSteeringConf 'StandardSignalOnlyTruth' \
    --geometryVersion default:ATLAS-R3S-2021-03-00-00 \
    --inputHITSFile ${HSHitsFile} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles1} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles2} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles3} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles4} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles1} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles2} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles3} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles4} \
    --jobNumber 1 \
    --maxEvents ${Events} \
    --outputRDOFile ${DigiOutFileNameCG} \
    --postExec 'HITtoRDO:job+=CfgMgr.JobOptsDumperAlg(FileName="DigiPUConfigCG.txt")' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'HITtoRDO:Campaigns/PileUpPresamplingMC21a.py' \
    --skipEvents 0

rc=$?
status=$rc
echo "art-result: $rc digiOLD"
mv runargs.HITtoRDOExecutorStep0.py runargs.legacy.HITtoRDOExecutorStep0.py
mv runargs.HITtoRDOExecutorStep1.py runargs.legacy.HITtoRDOExecutorStep1.py
mv runargs.HITtoRDOExecutorStep2.py runargs.legacy.HITtoRDOExecutorStep2.py
mv runargs.RDOMergeAthenaMP0.py runargs.legacy.RDOMergeAthenaMP0.py
mv log.HITtoRDOExecutorStep0 legacy.HITtoRDOExecutorStep0
mv log.HITtoRDOExecutorStep1 legacy.HITtoRDOExecutorStep1
mv log.HITtoRDOExecutorStep2 legacy.HITtoRDOExecutorStep2
mv log.RDOMergeAthenaMP0 legacy.RDOMergeAthenaMP0

rc2=-9999
Digi_tf.py \
    --CA \
    --detectors Truth \
    --splitConfig 'HITtoRDO:Campaigns.BeamspotSplitMC21a' \
    --PileUpPresampling True \
    --conditionsTag default:OFLCOND-MC21-SDR-RUN3-07 \
    --digiSeedOffset1 170 --digiSeedOffset2 170 \
    --digiSteeringConf 'StandardSignalOnlyTruth' \
    --geometryVersion default:ATLAS-R3S-2021-03-00-00 \
    --inputHITSFile ${HSHitsFile} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles1} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles2} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles3} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles4} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles1} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles2} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles3} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles4} \
    --jobNumber 1 \
    --maxEvents ${Events} \
    --outputRDOFile ${DigiOutFileNameCA} \
    --postInclude 'PyJobTransforms.UseFrontier' 'HITtoRDO:Digitization.DigitizationSteering.DigitizationTestingPostInclude' \
    --preInclude 'HITtoRDO:Campaigns.MC21a' \
    --skipEvents 0

rc2=$?
if [[ $status -eq 0 ]]
then
    status=$rc2
fi
echo "art-result: $rc2 digiCA"

rc3=-9999
if [ $status -eq 0 ]
then
    acmd.py diff-root ${DigiOutFileNameCG} ${DigiOutFileNameCA} \
        --mode=semi-detailed --error-mode resilient --order-trees \
        --ignore-leaves RecoTimingObj_p1_Bkg_HITStoRDO_timings index_ref
    rc3=$?
    status=$rc3
fi
echo "art-result: $rc3 OLDvsCA"

# get reference directory
source DigitizationCheckReferenceLocation.sh
echo "Reference set being used: ${DigitizationTestsVersion}"

rc4=-9999
if [[ $rc -eq 0 ]]
then
    # Do reference comparisons
    art.py compare ref --mode=semi-detailed --no-diff-meta "$DigiOutFileNameCG" "${ATLAS_REFERENCE_DATA}/DigitizationTests/ReferenceFiles/$DigitizationTestsVersion/$CMTCONFIG/$DigiOutFileNameCG"
    rc4=$?
    status=$rc4
fi
echo "art-result: $rc4 OLDvsFixedRef"

rc5=-9999
if [[ $rc -eq 0 ]]
then
    checkFile "$DigiOutFileNameCG"
    rc5=$?
    status=$rc5
fi
echo "art-result: $rc5 checkFile"

rc6=-9999
if [[ $rc -eq 0 ]]
then
    art.py compare grid --entries 10 "$1" "$2" --mode=semi-detailed --file="$DigiOutFileNameCG"
    rc6=$?
    status=$rc6
fi
echo "art-result: $rc6 regression"

exit $status
