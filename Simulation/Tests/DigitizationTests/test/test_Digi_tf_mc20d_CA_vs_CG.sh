#!/bin/sh
#
# art-description: Run a digitization example to compare configuration between ConfGetter and the new ComponentAccumulator approach.
# art-type: grid
# art-include: master/Athena
# art-output: mc20d_ttbar.CG.RDO.pool.root
# art-output: mc20d_ttbar.CA.RDO.pool.root
# art-output: log.*
# art-output: DigiPUConfig*

Events=3
DigiOutFileNameCG="mc20d_ttbar.CG.RDO.pool.root"
DigiOutFileNameCA="mc20d_ttbar.CA.RDO.pool.root"
HSHitsFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.simul.HITS.e4993_s3091/HITS.10504490._000425.pool.root.1"
HighPtMinbiasHitsFiles="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.361239.Pythia8EvtGen_A3NNPDF23LO_minbias_inelastic_high.merge.HITS.e4981_s3087_s3089/*"
LowPtMinbiasHitsFiles="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.361238.Pythia8EvtGen_A3NNPDF23LO_minbias_inelastic_low.merge.HITS.e4981_s3087_s3089/*"


# config only
Digi_tf.py \
--conditionsTag default:OFLCOND-MC16-SDR-RUN2-04 \
--digiSeedOffset1 170 --digiSeedOffset2 170 \
--digiSteeringConf "StandardSignalOnlyTruth" \
--geometryVersion default:ATLAS-R2-2016-01-00-01 \
--inputHITSFile ${HSHitsFile} \
--inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
--inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
--jobNumber 1 \
--maxEvents ${Events} \
--outputRDOFile ${DigiOutFileNameCG} \
--postExec 'all:CfgMgr.MessageSvc().setError+=["HepMcParticleLink"]' \
--postInclude 'default:PyJobTransforms/UseFrontier.py' \
--preInclude 'all:Campaigns/MC20d.py' 'HITtoRDO:Campaigns/PileUpMC20d.py' \
--skipEvents 0 \
--athenaopts '"--config-only=DigiPUConfigCG.pkl"'

# full run
Digi_tf.py \
--conditionsTag default:OFLCOND-MC16-SDR-RUN2-04 \
--digiSeedOffset1 170 --digiSeedOffset2 170 \
--digiSteeringConf "StandardSignalOnlyTruth" \
--geometryVersion default:ATLAS-R2-2016-01-00-01 \
--inputHITSFile ${HSHitsFile} \
--inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
--inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
--jobNumber 1 \
--maxEvents ${Events} \
--outputRDOFile ${DigiOutFileNameCG} \
--postExec 'all:CfgMgr.MessageSvc().setError+=["HepMcParticleLink"]' 'HITtoRDO:job+=CfgMgr.JobOptsDumperAlg(FileName="DigiPUConfigCG.txt")' \
--postInclude 'default:PyJobTransforms/UseFrontier.py' \
--preInclude 'all:Campaigns/MC20d.py' 'HITtoRDO:Campaigns/PileUpMC20d.py' \
--skipEvents 0

rc=$?
status=$rc
echo "art-result: $rc CGdigi"
mv runargs.HITtoRDO.py runargs.legacy.HITtoRDO.py 
mv log.HITtoRDO legacy.HITtoRDO

rc2=-9999
if [ $rc -eq 0 ]
then
    Digi_tf.py \
    --CA \
    --conditionsTag default:OFLCOND-MC16-SDR-RUN2-04 \
    --digiSeedOffset1 170 --digiSeedOffset2 170 \
    --digiSteeringConf "StandardSignalOnlyTruth" \
    --geometryVersion default:ATLAS-R2-2016-01-00-01 \
    --inputHITSFile ${HSHitsFile} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
    --jobNumber 1 \
    --maxEvents ${Events} \
    --outputRDOFile ${DigiOutFileNameCA} \
    --postInclude 'PyJobTransforms.UseFrontier' 'HITtoRDO:Digitization.DigitizationSteering.DigitizationTestingPostInclude' \
    --preInclude 'HITtoRDO:Campaigns.MC20d' \
    --skipEvents 0

    rc2=$?
    status=$rc2
fi

echo "art-result: $rc2 CAdigi"

rc3=-9999
if [ $rc2 -eq 0 ]
then
    acmd.py diff-root ${DigiOutFileNameCG} ${DigiOutFileNameCA} \
        --mode=semi-detailed --error-mode resilient --order-trees \
        --ignore-leaves RecoTimingObj_p1_HITStoRDO_timings index_ref
    rc3=$?
    status=$rc3
fi

echo "art-result: $rc3 comparison"

exit $status
