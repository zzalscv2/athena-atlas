#!/bin/sh
#
# art-description: ReSimulation Workflow running with MC21 conditions/geometry and variable beamspot
# art-include: master/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: log.*
# art-output: Config*.pkl
# art-output: original.HITS.pool.root
# art-output: resim.*.HITS.pool.root

INPUTEVNTFILE="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1"
MAXEVENTS=50

rc=-9999
if [ -f "original.HITS.pool.root" ]
then
    rc=0
    echo "skipping initial-simOLD step"
else
    Sim_tf.py \
        --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-05' \
        --simulator 'FullG4MT' \
        --postInclude 'default:PyJobTransforms/UseFrontier.py' \
        --preInclude 'EVNTtoHITS:Campaigns/MC21SimulationMultipleIoV.py' \
        --geometryVersion 'default:ATLAS-R3S-2021-03-00-00' \
        --inputEVNTFile $INPUTEVNTFILE \
        --outputHITSFile "original.HITS.pool.root" \
        --maxEvents $MAXEVENTS \
        --jobNumber 29 \
        --imf False

    rc=$?
    mv log.EVNTtoHITS log.EVNTtoHITS.initial
fi
status=$rc
echo "art-result: $rc initial-simOLD"

rc1=-9999
if [ $rc -eq 0 ]
then
    ReSim_tf.py \
        --CA \
        --conditionsTag 'ReSim:OFLCOND-MC21-SDR-RUN3-05' \
        --simulator 'FullG4MT_QS' \
        --postInclude 'ReSim:PyJobTransforms.UseFrontier' \
        --preInclude 'ReSim:Campaigns.MC21SimulationNoIoV' \
        --geometryVersion 'ReSim:ATLAS-R3S-2021-03-00-00' \
        --inputHITSFile "original.HITS.pool.root" \
        --outputHITS_RSMFile "resim.CA.HITS.pool.root" \
        --maxEvents $MAXEVENTS \
        --postExec 'ReSim:with open("ConfigReSimCA.pkl", "wb") as f: cfg.store(f)' \
        --imf False

    rc1=$?
    status=$rc1
    mv log.ReSim log.ReSim.CA
fi
echo "art-result: $rc1 resimCA"

rc2=-9999
if [ $rc -eq 0 ]
then
    ReSim_tf.py \
        --conditionsTag 'ReSim:OFLCOND-MC21-SDR-RUN3-05' \
        --simulator 'FullG4MT_QS' \
        --postInclude 'ReSim:PyJobTransforms/UseFrontier.py' \
        --preInclude 'ReSim:Campaigns/MC21SimulationNoIoV.py' \
        --geometryVersion 'ReSim:ATLAS-R3S-2021-03-00-00' \
        --inputHITSFile "original.HITS.pool.root" \
        --outputHITS_RSMFile "resim.CA.HITS.pool.root" \
        --maxEvents $MAXEVENTS \
        --imf False \
        --athenaopts '"--config-only=ConfigReSimCG.pkl"'

    ReSim_tf.py \
        --conditionsTag 'ReSim:OFLCOND-MC21-SDR-RUN3-05' \
        --simulator 'FullG4MT_QS' \
        --postInclude 'ReSim:PyJobTransforms/UseFrontier.py' \
        --preInclude 'ReSim:Campaigns/MC21SimulationNoIoV.py' \
        --geometryVersion 'ReSim:ATLAS-R3S-2021-03-00-00' \
        --inputHITSFile "original.HITS.pool.root" \
        --outputHITS_RSMFile "resim.CG.HITS.pool.root" \
        --maxEvents $MAXEVENTS \
        --imf False

    rc2=$?
    status=$rc2
    mv log.ReSim log.ReSim.CG
fi
echo "art-result: $rc2 resimOLD"

rc3=-9999
if [ $status -eq 0 ]
then
  # Compare the outputs
  acmd.py diff-root resim.CG.HITS.pool.root resim.CA.HITS.pool.root \
    --error-mode resilient \
    --mode semi-detailed \
    --order-trees \
    --ignore-leaves RecoTimingObj_p1_EVNTtoHITS_timings index_ref
  rc3=$?
  status=$rc3
fi

echo "art-result: $rc3 OLDvsCA"

rc4=-9999
if [ $rc2 -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName}  --order-trees --diff-root --mode=semi-detailed --ignore-leave RecoTimingObj_p1_EVNTtoHITS_timingsOLD --file=resim.CG.HITS.pool.root
    rc4=$?
    status=$rc4
fi
echo  "art-result: $rc4 regression"

exit $status
