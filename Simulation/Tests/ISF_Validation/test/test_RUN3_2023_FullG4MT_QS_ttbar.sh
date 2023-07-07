#!/bin/sh
#
# art-description: Run simulation using ISF with the FullG4MT_QS simulator, reading 13.6 TeV ttbar events, writing HITS, using the best knowledge RUN3 geometry and MC23 conditions
# art-include: 23.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: *.pool.root
# art-output: log.*
# art-output: Config*.pkl

InputEVNT='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1'

Sim_tf.py \
--CA True \
--conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
--simulator 'FullG4MT_QS' \
--postInclude 'PyJobTransforms.TransformUtils.UseFrontier' \
--preInclude 'EVNTtoHITS:Campaigns.MC23cSimulationMultipleIoV' \
--geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
--inputEVNTFile $InputEVNT \
--outputHITSFile 'test.CA.HITS.pool.root' \
--maxEvents '50' \
--jobNumber 1 \
--postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
--imf False

rc=$?
mv log.EVNTtoHITS log.EVNTtoHITS_CA
echo  "art-result: $rc simCA"
status=$rc

Sim_tf.py \
--conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
--simulator 'FullG4MT_QS' \
--postInclude 'default:PyJobTransforms/UseFrontier.py' \
--preInclude 'EVNTtoHITS:Campaigns/MC23cSimulationMultipleIoV.py' \
--geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
--inputEVNTFile $InputEVNT \
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
--inputEVNTFile $InputEVNT \
--outputHITSFile 'test.HITS.pool.root' \
--maxEvents '50' \
--jobNumber 1 \
--imf False

rc2=$?
mv log.EVNTtoHITS log.EVNTtoHITS_OLD
echo  "art-result: $rc2 simOLD"
if [ $status -eq 0]
then
    status=$rc2
fi

rc3=-9999
if [ $status -eq 0 ]
then
    acmd.py diff-root test.HITS.pool.root test.CA.HITS.pool.root --error-mode resilient --mode=semi-detailed --order-trees
    rc3=$?
    status=$rc3
fi
echo  "art-result: $rc3 OLDvsCA"

rc4=-9999
if [ $rc2 -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 50 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.HITS.pool.root
    rc4=$?
    status=$rc4
fi
echo  "art-result: $rc4 regression"

exit $status
