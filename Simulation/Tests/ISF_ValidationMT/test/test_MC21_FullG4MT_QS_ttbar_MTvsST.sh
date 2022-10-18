#!/bin/sh
#
# art-description: Run MT and ST simulation, reading ttbar events, writing HITS, using MC21 geometry and conditions
# art-include: 22.0/Athena
# art-include: 22.0/AthSimulation
# art-include: master/Athena
# art-include: master/AthSimulation
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-output: log.*
# art-output: Config*.pkl
# art-output: test.MT.HITS.pool.root
# art-output: test.ST.HITS.pool.root

export ATHENA_CORE_NUMBER=8

Sim_tf.py \
    --multithreaded \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1" \
    --outputHITSFile "test.MT.HITS.pool.root" \
    --maxEvents 10 \
    --geometryVersion 'default:ATLAS-R3S-2021-03-00-00' \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-05' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC21Simulation.py,SimulationJobOptions/preInclude.ExtraParticles.py,SimulationJobOptions/preInclude.G4ExtraProcesses.py' \
    --imf False \
    --athenaopts '"--config-only=ConfigSimMT.pkl"'

Sim_tf.py \
    --multithreaded \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1" \
    --outputHITSFile "test.MT.HITS.pool.root" \
    --maxEvents 10 \
    --geometryVersion 'default:ATLAS-R3S-2021-03-00-00' \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-05' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC21Simulation.py,SimulationJobOptions/preInclude.ExtraParticles.py,SimulationJobOptions/preInclude.G4ExtraProcesses.py' \
    --imf False

rc=$?
mv log.EVNTtoHITS log.EVNTtoHITS.MT
echo  "art-result: $rc AthenaMT"
status=$rc

rc2=-9999
unset ATHENA_CORE_NUMBER
Sim_tf.py \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1" \
    --outputHITSFile "test.ST.HITS.pool.root" \
    --maxEvents 10 \
    --geometryVersion 'default:ATLAS-R3S-2021-03-00-00' \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-05' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC21Simulation.py,SimulationJobOptions/preInclude.ExtraParticles.py,SimulationJobOptions/preInclude.G4ExtraProcesses.py' \
    --imf False \
    --athenaopts '"--config-only=ConfigSimST.pkl"'

Sim_tf.py \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1" \
    --outputHITSFile "test.ST.HITS.pool.root" \
    --maxEvents 10 \
    --geometryVersion 'default:ATLAS-R3S-2021-03-00-00' \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-05' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC21Simulation.py,SimulationJobOptions/preInclude.ExtraParticles.py,SimulationJobOptions/preInclude.G4ExtraProcesses.py' \
    --imf False

mv log.EVNTtoHITS log.EVNTtoHITS.ST
rc2=$?
if [ $staus -eq 0 ]
then
    status=$rc2
fi
echo  "art-result: $rc2 serial Athena"

rc3=-9999
if [ $rc2 -eq 0 ]
then
    acmd.py diff-root test.MT.HITS.pool.root test.ST.HITS.pool.root \
        --error-mode resilient \
        --mode=semi-detailed \
        --order-trees \
        --ignore-leaves RecoTimingObj_p1_EVNTtoHITS_timings index_ref
    rc3=$?
    status=$rc3
fi
echo  "art-result: $rc3 FullG4MT_STvsMT"

rc4=-9999
if [ $rc2 -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --order-trees --diff-root --mode=semi-detailed --file=test.MT.HITS.pool.root
    rc4=$?
    status=$rc4
fi
echo  "art-result: $rc4 regression"

exit $status
