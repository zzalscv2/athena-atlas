#!/bin/sh
#
# art-description: MC21-style simulation of monopole samples using FullG4MT_QS
# art-include: 22.0/Athena
# art-include: 22.0/AthSimulation
# art-include: master/Athena
# art-include: master/AthSimulation
# art-type: grid
# art-athena-mt: 8
# art-architecture:  '#x86_64-intel'
# art-output: test.*.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

export ATHENA_PROC_NUMBER=8
export ATHENA_CORE_NUMBER=8
Sim_tf.py \
    --CA \
    --inputEVNTFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/mc21_13p6TeV.950542.MGPy8EG_DYSpinHalfMonopoles_1gD_1500GeV_valid.merge.EVNT.e8467_e8455.29631249._000005.pool.root.1" \
    --maxEvents="100" \
    --multithreaded="True" \
    --preInclude "EVNTtoHITS:Campaigns.MC21SimulationMultiBeamSpot,G4DebuggingTools.DebugMonopole" \
    --skipEvents="0" \
    --randomSeed="41" \
    --DBRelease="all:300.0.4" \
    --conditionsTag "default:OFLCOND-MC21-SDR-RUN3-05" \
    --geometryVersion="default:ATLAS-R3S-2021-03-00-00" \
    --runNumber="950542" \
    --AMITag="s3890" \
    --jobNumber="41" \
    --firstEvent="40001" \
    --outputHITSFile="test.CA.HITS.pool.root" \
    --simulator="FullG4MT_QS" \
    --imf False \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)'

rc=$?
mv log.EVNTtoHITS log.EVNTtoHITS.CA
echo  "art-result: $rc simCA"
status=$rc

rc2=-9999
if [ $rc -eq 0 ]
then
    Sim_tf.py \
        --inputEVNTFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/mc21_13p6TeV.950542.MGPy8EG_DYSpinHalfMonopoles_1gD_1500GeV_valid.merge.EVNT.e8467_e8455.29631249._000005.pool.root.1" \
        --maxEvents="100" \
        --multithreaded="True" \
        --postInclude "default:PyJobTransforms/UseFrontier.py" \
        --preInclude "EVNTtoHITS:Campaigns/MC21SimulationMultiBeamSpot.py,SimulationJobOptions/preInclude.ExtraParticles.py,SimulationJobOptions/preInclude.G4ExtraProcesses.py,SimulationJobOptions/preInclude.DebugMonopoles.py" \
        --skipEvents="0" \
        --randomSeed="41" \
        --DBRelease="all:300.0.4" \
        --conditionsTag "default:OFLCOND-MC21-SDR-RUN3-05" \
        --geometryVersion="default:ATLAS-R3S-2021-03-00-00_VALIDATION" \
        --runNumber="950542" \
        --AMITag="s3890" \
        --jobNumber="41" \
        --firstEvent="40001" \
        --outputHITSFile="test.CA.HITS.pool.root" \
        --simulator="FullG4MT_QS" \
        --imf False \
        --athenaopts '"--config-only=ConfigSimCG.pkl"'

    Sim_tf.py \
        --inputEVNTFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/mc21_13p6TeV.950542.MGPy8EG_DYSpinHalfMonopoles_1gD_1500GeV_valid.merge.EVNT.e8467_e8455.29631249._000005.pool.root.1" \
        --maxEvents="100" \
        --multithreaded="True" \
        --postInclude "default:PyJobTransforms/UseFrontier.py" \
        --preInclude "EVNTtoHITS:Campaigns/MC21SimulationMultiBeamSpot.py,SimulationJobOptions/preInclude.ExtraParticles.py,SimulationJobOptions/preInclude.G4ExtraProcesses.py,SimulationJobOptions/preInclude.DebugMonopoles.py" \
        --skipEvents="0" \
        --randomSeed="41" \
        --DBRelease="all:300.0.4" \
        --conditionsTag "default:OFLCOND-MC21-SDR-RUN3-05" \
        --geometryVersion="default:ATLAS-R3S-2021-03-00-00_VALIDATION" \
        --runNumber="950542" \
        --AMITag="s3890" \
        --jobNumber="41" \
        --firstEvent="40001" \
        --outputHITSFile="test.CG.HITS.pool.root" \
        --simulator="FullG4MT_QS" \
        --imf False

    rc2=$?
    status=$rc2
    mv log.EVNTtoHITS log.EVNTtoHITS.CG
fi
echo "art-result: $rc2 simOLD"

rc3=-9999
if [ $rc2 -eq 0 ]
then
  # Compare the outputs
  acmd.py diff-root test.CG.HITS.pool.root test.CA.HITS.pool.root \
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
    art.py compare grid --entries 4 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.CG.HITS.pool.root
    rc4=$?
    status=$rc4
fi
echo  "art-result: $rc4 regression"

exit $status
