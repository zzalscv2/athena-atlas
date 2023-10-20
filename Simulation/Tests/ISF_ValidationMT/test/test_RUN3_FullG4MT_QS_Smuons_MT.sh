#!/bin/sh
#
# art-description: MC23-style RUN3 simulation of smuon samples using FullG4MT_QS in AthenaMT - input needs to be updated to an official sample when available.
# art-include: 23.0/Athena
# art-include: 23.0/AthSimulation
# art-include: 24.0/Athena
# art-include: 24.0/AthSimulation
# art-include: main/Athena
# art-include: main/AthSimulation
# art-type: grid
# art-athena-mt: 8
# art-architecture:  '#x86_64-intel'
# art-output: test.*.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

export ATHENA_PROC_NUMBER=8
export ATHENA_CORE_NUMBER=8
# RUN3 setup
# ATLAS-R3S-2021-03-02-00 and OFLCOND-MC23-SDR-RUN3-01
Sim_tf.py \
    --CA \
    --inputEVNTFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/mc21_13p6TeV.temp.DisplacedSmuonPairProduction.10events.pool.root" \
    --maxEvents="10" \
    --multithreaded="True" \
    --preInclude "EVNTtoHITS:Campaigns.MC23aSimulationMultipleIoV,G4DebuggingTools.DebugSleptonsLLP" \
    --skipEvents="0" \
    --randomSeed="41" \
    --conditionsTag "default:OFLCOND-MC23-SDR-RUN3-01" \
    --geometryVersion="default:ATLAS-R3S-2021-03-02-00" \
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

Sim_tf.py \
    --inputEVNTFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/mc21_13p6TeV.temp.DisplacedSmuonPairProduction.10events.pool.root" \
    --maxEvents="10" \
    --multithreaded="True" \
    --postInclude "default:PyJobTransforms/UseFrontier.py" \
    --preInclude "EVNTtoHITS:Campaigns/MC23aSimulationMultipleIoV.py,SimulationJobOptions/preInclude.DebugSleptonsLLP.py" \
    --skipEvents="0" \
    --randomSeed="41" \
    --conditionsTag "default:OFLCOND-MC23-SDR-RUN3-01" \
    --geometryVersion="default:ATLAS-R3S-2021-03-02-00" \
    --runNumber="950542" \
    --AMITag="s3890" \
    --jobNumber="41" \
    --firstEvent="40001" \
    --outputHITSFile="test.CA.HITS.pool.root" \
    --simulator="FullG4MT_QS" \
    --imf False \
    --athenaopts '"--config-only=ConfigSimCG.pkl"'

Sim_tf.py \
    --inputEVNTFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/mc21_13p6TeV.temp.DisplacedSmuonPairProduction.10events.pool.root" \
    --maxEvents="10" \
    --multithreaded="True" \
    --postInclude "default:PyJobTransforms/UseFrontier.py" \
    --preInclude "EVNTtoHITS:Campaigns/MC23aSimulationMultipleIoV.py,SimulationJobOptions/preInclude.DebugSleptonsLLP.py" \
    --skipEvents="0" \
    --randomSeed="41" \
    --conditionsTag "default:OFLCOND-MC23-SDR-RUN3-01" \
    --geometryVersion="default:ATLAS-R3S-2021-03-02-00" \
    --runNumber="950542" \
    --AMITag="s3890" \
    --jobNumber="41" \
    --firstEvent="40001" \
    --outputHITSFile="test.CG.HITS.pool.root" \
    --simulator="FullG4MT_QS" \
    --imf False

rc2=$?
mv log.EVNTtoHITS log.EVNTtoHITS.CG
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
    art.py compare grid --entries 4 ${ArtPackage} ${ArtJobName} --order-trees --diff-root --mode=semi-detailed --file=test.CG.HITS.pool.root
    rc4=$?
    status=$rc4
fi
echo  "art-result: $rc4 regression"

exit $status
