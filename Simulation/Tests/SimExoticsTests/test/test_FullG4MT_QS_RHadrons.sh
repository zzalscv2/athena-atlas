#!/bin/sh
# 
# art-description: MC23-style simulation of decaying Rhadrons using FullG4MT_QS (tests RHadrons package)
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-include: 23.0/Athena
# art-include: 23.0/AthSimulation
# art-include: main/Athena
# art-include: main/AthSimulation
# art-output: *.root
# art-output: PDGTABLE.MeV.*
# art-output: *.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

# MC16 setup
# ATLAS-R2-2016-01-00-01 and OFLCOND-MC16-SDR-14
Sim_tf.py \
    --CA True \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23cSimulationMultipleIoV' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/Rhadrons.EVNT.34227998._000001.pool.root.1" \
    --outputHITSFile "test.CA.HITS.pool.root" \
    --maxEvents '50' \
    --jobNumber 1 \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False

rc=$?
mv PDGTABLE.MeV PDGTABLE.MeV.CA
mv log.EVNTtoHITS log.EVNTtoHITS.CA
echo  "art-result: $rc simCA"
status=$rc

Sim_tf.py \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC23cSimulationMultipleIoV.py' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/Rhadrons.EVNT.34227998._000001.pool.root.1" \
    --outputHITSFile 'test.CA.HITS.pool.root' \
    --maxEvents '50' \
    --jobNumber 1 \
    --imf False \
    --athenaopts '"--config-only=ConfigSimCG.pkl"'

Sim_tf.py \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC23cSimulationMultipleIoV.py' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/Rhadrons.EVNT.34227998._000001.pool.root.1" \
    --outputHITSFile 'test.CG.HITS.pool.root' \
    --maxEvents '50' \
    --jobNumber 1 \
    --imf False

rc2=$?
status=$rc2
mv PDGTABLE.MeV PDGTABLE.MeV.CG
mv log.EVNTtoHITS log.EVNTtoHITS.CG
echo  "art-result: $rc2 simOLD"

rc3=-9999
if [ $rc -eq 0 ] && [ $rc2 -eq 0 ]
then
    acmd.py diff-root test.CG.HITS.pool.root test.CA.HITS.pool.root --error-mode resilient --mode=semi-detailed --order-trees
    rc3=$?
    status=$rc3
fi
echo  "art-result: $rc3 OLDvsCA"

rc4=-9999
if [ $rc2 -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 50 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.CG.HITS.pool.root
    rc4=$?
    status=$rc4
fi
echo  "art-result: $rc4 regression"

exit $status
