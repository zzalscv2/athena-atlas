#!/bin/bash
#
# art-description: Run digitization of an mc20e ttbar sample with 2018 geometry and conditions, 25ns pile-up
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-include: 21.0/Athena
# art-include: 21.3/Athena
# art-include: 21.9/Athena
# art-include: master/Athena
# art-output: mc16e_ttbar_beamspotsizereweight.*.RDO.pool.root

DigiOutFileNameCG="mc16e_ttbar_beamspotsizereweight.CG.RDO.pool.root"
DigiOutFileNameCA="mc16e_ttbar_beamspotsizereweight.CA.RDO.pool.root"

InputHitsFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.simul.HITS.e4993_s3091/HITS.10504490._000425.pool.root.1"
HighPtMinbiasHitsFiles="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.361239.Pythia8EvtGen_A3NNPDF23LO_minbias_inelastic_high.merge.HITS.e4981_s3087_s3089/*"
LowPtMinbiasHitsFiles="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.361238.Pythia8EvtGen_A3NNPDF23LO_minbias_inelastic_low.merge.HITS.e4981_s3087_s3089/*"


# config only
Digi_tf.py \
    --conditionsTag default:OFLCOND-MC16-SDR-RUN2-09 \
    --digiSeedOffset1 170 --digiSeedOffset2 170 \
    --digiSteeringConf 'StandardSignalOnlyTruth' \
    --geometryVersion default:ATLAS-R2-2016-01-00-01 \
    --inputHITSFile ${InputHitsFile} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
    --jobNumber 568 \
    --maxEvents 25 \
    --outputRDOFile ${DigiOutFileNameCG} \
    --postExec 'HITtoRDO:conddb.addOverride("/Indet/Beampos","IndetBeampos-13TeV-MC16-002")' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preExec 'HITtoRDO:from Digitization.DigitizationFlags import digitizationFlags; digitizationFlags.OldBeamSpotZSize=42;' \
    --preInclude 'all:Campaigns/MC20a.py' 'HITtoRDO:Campaigns/PileUpMC20a.py' \
    --skipEvents 0 \
--athenaopts '"--config-only=DigiPUConfigCG.pkl"'

# full run
Digi_tf.py \
    --conditionsTag default:OFLCOND-MC16-SDR-RUN2-09 \
    --digiSeedOffset1 170 --digiSeedOffset2 170 \
    --digiSteeringConf 'StandardSignalOnlyTruth' \
    --geometryVersion default:ATLAS-R2-2016-01-00-01 \
    --inputHITSFile ${InputHitsFile} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
    --jobNumber 568 \
    --maxEvents 25 \
    --outputRDOFile ${DigiOutFileNameCG} \
    --postExec 'HITtoRDO:conddb.addOverride("/Indet/Beampos","IndetBeampos-13TeV-MC16-002")' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preExec 'HITtoRDO:from Digitization.DigitizationFlags import digitizationFlags; digitizationFlags.OldBeamSpotZSize=42;' \
    --preInclude 'all:Campaigns/MC20a.py' 'HITtoRDO:Campaigns/PileUpMC20a.py' \
    --skipEvents 0

rc=$?
status=$rc
echo "art-result: $rc digiOLD"
mv runargs.HITtoRDO.py runargs.legacy.HITtoRDO.py
mv log.HITtoRDO legacy.HITtoRDO


Digi_tf.py \
    --CA \
    --conditionsTag default:OFLCOND-MC16-SDR-RUN2-09 \
    --digiSeedOffset1 170 --digiSeedOffset2 170 \
    --digiSteeringConf 'StandardSignalOnlyTruth' \
    --geometryVersion default:ATLAS-R2-2016-01-00-01 \
    --inputHITSFile ${InputHitsFile} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
    --jobNumber 568 \
    --maxEvents 25 \
    --outputRDOFile ${DigiOutFileNameCA} \
    --postExec 'all:from IOVDbSvc.IOVDbSvcConfig import addOverride;cfg.merge(addOverride(flags, "/Indet/Beampos", "IndetBeampos-13TeV-MC16-002"))' \
    --postInclude 'PyJobTransforms.UseFrontier' 'HITtoRDO:Digitization.DigitizationSteering.DigitizationTestingPostInclude' \
    --preInclude 'HITtoRDO:Campaigns.MC20a' \
    --preExec 'HITtoRDO:flags.Digitization.InputBeamSigmaZ=42;' \
    --skipEvents 0

    rc2=$?
if [[ $status -eq 0 ]]
then
    status=$rc2
fi
echo "art-result: $rc2 digiCA"

rc3=-9999
if [[ $status -eq 0 ]]
then
    acmd.py diff-root ${DigiOutFileNameCG} ${DigiOutFileNameCA} \
        --mode=semi-detailed --error-mode resilient --order-trees \
        --ignore-leaves RecoTimingObj_p1_HITStoRDO_timings index_ref
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
    art.py compare ref --mode=semi-detailed --no-diff-meta "$DigiOutFileNameCG" "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DigitizationTests/ReferenceFiles/$DigitizationTestsVersion/$CMTCONFIG/$DigiOutFileNameCG"
    rc4=$?
    status=$rc4
fi
echo "art-result: $rc4 OLDvsFixedRef"

if [[ $rc -eq 0 ]]
then
    art.py compare grid --entries 10 "$1" "$2" --mode=semi-detailed --file=${DigiOutFileNameCG}
    rc5=$?
    status=$rc5
fi
echo "art-result: $rc5 regression"

exit $status
