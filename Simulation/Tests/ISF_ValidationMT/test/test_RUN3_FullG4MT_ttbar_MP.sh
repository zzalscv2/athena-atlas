#!/bin/sh
#
# art-description: MC23-style RUN3 simulation using FullG4MT in AthenaMP
# art-include: 23.0/Athena
# art-include: 23.0/AthSimulation
# art-include: master/Athena
# art-include: master/AthSimulation
# art-type: grid
# art-athena-mt: 8
# art-architecture:  '#x86_64-intel'
# art-output: test.*.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

export ATHENA_CORE_NUMBER=8

# RUN3 setup
# ATLAS-R3S-2021-03-01-00 and OFLCOND-MC21-SDR-RUN3-07
Sim_tf.py \
    --CA \
    --multiprocess \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --simulator 'FullG4MT' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23SimulationSingleIoV' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-01-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1" \
    --outputHITSFile "test.CA.HITS.pool.root" \
    --maxEvents 50 \
    --postExec 'EVNTtoHITS:with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False

rc=$?
mv log.EVNTtoHITS log.EVNTtoHITS.CA
mv log.HITSMergeAthenaMP0 log.HITSMergeAthenaMP0.CA
echo  "art-result: $rc simCA"
status=$rc

rc2=-9999
Sim_tf.py \
    --multiprocess \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --simulator 'FullG4MT' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC23SimulationSingleIoV.py' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-01-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1" \
    --outputHITSFile "test.CA.HITS.pool.root" \
    --maxEvents 50 \
    --imf False \
    --athenaopts 'EVNTtoHITS:"--config-only=ConfigSimCG.pkl"'

Sim_tf.py \
    --multiprocess \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --simulator 'FullG4MT' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC23SimulationSingleIoV.py' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-01-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1" \
    --outputHITSFile "test.CG.HITS.pool.root" \
    --maxEvents 50 \
    --imf False

rc2=$?
mv log.EVNTtoHITS log.EVNTtoHITS.CG
mv log.HITSMergeAthenaMP0 log.HITSMergeAthenaMP0.CG
echo "art-result: $rc2 simOLD"
if [ $status -eq 0 ]
then
    status=$rc2
fi

rc3=-9999
if [ $status -eq 0 ]
then
    # Compare the outputs
    acmd.py diff-root test.CG.HITS.pool.root test.CA.HITS.pool.root \
        --error-mode resilient \
        --mode semi-detailed \
        --order-trees
    rc3=$?
    status=$rc3
fi
echo "art-result: $rc3 OLDvsCA"

rc4=-9999
if [ $rc2 -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --order-trees --mode=semi-detailed --diff-root --file=test.CG.HITS.pool.root
    rc4=$?
    status=$rc4
fi
echo  "art-result: $rc4 regression"

exit $status
