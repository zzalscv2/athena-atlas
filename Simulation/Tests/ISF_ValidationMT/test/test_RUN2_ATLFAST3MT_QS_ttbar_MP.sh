#!/bin/sh
#
# art-description: MC23-style RUN2 simulation using best knowledge geometry and ATLFAST3MT_QS in AthenaMP
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-athena-mt: 8
# art-architecture:  '#x86_64-intel'
# art-output: test.*.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

export ATHENA_CORE_NUMBER=8

# RUN3 setup
# ATLAS-R2-2016-01-02-01 and OFLCOND-MC23-SDR-RUN3-01
Sim_tf.py \
    --CA \
    --multiprocess \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --simulator 'ATLFAST3MT_QS' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23SimulationNoIoV' \
    --DataRunNumber 284500 \
    --geometryVersion 'default:ATLAS-R2-2016-01-02-01' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc20/EVNT/mc15_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.evgen.EVNT.e6337/EVNT.12458444._006026.pool.root.1" \
    --outputHITSFile "test.CA.HITS.pool.root" \
    --maxEvents 50 \
    --postExec 'EVNTtoHITS:with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False

rc=$?
mv log.EVNTtoHITS log.EVNTtoHITS.CA
mv log.HITSMergeAthenaMP0 log.HITSMergeAthenaMP0.CA
echo  "art-result: $rc simCA"
status=$rc

Sim_tf.py \
    --multiprocess \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --simulator 'ATLFAST3MT_QS' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC23SimulationNoIoV.py' \
    --DataRunNumber 284500 \
    --geometryVersion 'default:ATLAS-R2-2016-01-02-01' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc20/EVNT/mc15_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.evgen.EVNT.e6337/EVNT.12458444._006026.pool.root.1" \
    --outputHITSFile "test.CA.HITS.pool.root" \
    --maxEvents 50 \
    --imf False \
    --athenaopts 'EVNTtoHITS:"--config-only=ConfigSimCG.pkl"'

Sim_tf.py \
    --multiprocess \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --simulator 'ATLFAST3MT_QS' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC23SimulationNoIoV.py' \
    --DataRunNumber 284500 \
    --geometryVersion 'default:ATLAS-R2-2016-01-02-01' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc20/EVNT/mc15_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.evgen.EVNT.e6337/EVNT.12458444._006026.pool.root.1" \
    --outputHITSFile "test.CG.HITS.pool.root" \
    --maxEvents 50 \
    --imf False

rc2=$?
mv log.EVNTtoHITS log.EVNTtoHITS.CG
mv log.HITSMergeAthenaMP0 log.HITSMergeAthenaMP0.CG
echo "art-result: $rc2 simOLD"
if [ $status -eq 0]
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
