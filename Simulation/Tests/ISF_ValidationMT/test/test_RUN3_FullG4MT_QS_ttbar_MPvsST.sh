#!/bin/sh
#
# art-description: Run MP and ST simulation, reading ttbar events, writing HITS, using MC23a geometry and conditions
# art-include: 23.0/Athena
# art-include: 23.0/AthSimulation
# art-include: master/Athena
# art-include: master/AthSimulation
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-output: log.*
# art-output: Config*.pkl
# art-output: test.MP.HITS.pool.root
# art-output: test.ST.HITS.pool.root

export ATHENA_CORE_NUMBER=8

Sim_tf.py \
    --multiprocess \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1" \
    --outputHITSFile "test.MP.HITS.pool.root" \
    --maxEvents 50 \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC23SimulationSingleIoV.py' \
    --imf False \
    --athenaopts '"--config-only=ConfigSimMP.pkl"'

Sim_tf.py \
    --multiprocess \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1" \
    --outputHITSFile "test.MP.HITS.pool.root" \
    --maxEvents 50 \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC23SimulationSingleIoV.py' \
    --imf False

rc=$?
mv log.EVNTtoHITS log.EVNTtoHITS.MP
echo  "art-result: $rc AthenaMP"
status=$rc

rc2=-9999
unset ATHENA_CORE_NUMBER
Sim_tf.py \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1" \
    --outputHITSFile "test.MP.HITS.pool.root" \
    --maxEvents 50 \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC23SimulationSingleIoV.py' \
    --imf False \
    --athenaopts '"--config-only=ConfigSimST.pkl"'

Sim_tf.py \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1" \
    --outputHITSFile "temp.ST.HITS.pool.root" \
    --maxEvents 50 \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC23SimulationSingleIoV.py' \
    --imf False

mv log.EVNTtoHITS log.EVNTtoHITS.ST
rc2=$?
if [ $status -eq 0 ]
then
    status=$rc2
fi
echo  "art-result: $rc2 serial Athena"

rc3=-9999
if [ $rc2 -eq 0 ]
then
    rm PoolFileCatalog.xml
    # Run a dummy merge on the full hits file to deal with lossy compression:
    HITSMerge_tf.py --inputHITSFile 'temp.ST.HITS.pool.root' --outputHITS_MRGFile 'test.ST.HITS.pool.root'
    rc3=$?
    status=$rc3
fi
echo "art-result: $rc3 serial merge"

rc4=-9999
if [ $status -eq 0 ]
then
    acmd.py diff-root test.MP.HITS.pool.root test.ST.HITS.pool.root \
        --error-mode resilient \
        --mode=semi-detailed \
        --order-trees
    rc4=$?
    status=$rc4
fi
echo  "art-result: $rc4 STvsMP"

rc5=-9999
if [ $rc3 -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --order-trees --mode=semi-detailed --file=test.ST.HITS.pool.root
    rc5=$?
    status=$rc5
fi
echo  "art-result: $rc5 regression"

exit $status
