#!/bin/sh
#
# art-description: MC23-style Run3 simulation using FullG4MT_QS (13.6 TeV Zprime input) in AthenaMT
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
#/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/EVNT.04607198._000001.pool.root.1 # Older Zprime input

export ATHENA_CORE_NUMBER=8

# RUN3 setup
# ATLAS-R3S-2021-03-02-00 and OFLCOND-MC21-SDR-RUN3-07
Sim_tf.py \
    --CA \
    --multithreaded \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23aSimulationMultipleIoV' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/mc23_13p6TeV.800030.Py8EG_A14NNPDF23LO_flatpT_Zprime_Extended.evgen.EVNT.e8514.33116368._004446.pool.root.1' \
    --outputHITSFile 'test.CA.HITS.pool.root' \
    --maxEvents 50 \
    --skipEvents 50 \
    --jobNumber 1 \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False

rc=$?
mv log.EVNTtoHITS log.EVNTtoHITS.CA
echo  "art-result: $rc simCA"
status=$rc

Sim_tf.py \
    --multithreaded \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC23aSimulationMultipleIoV.py' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/mc23_13p6TeV.800030.Py8EG_A14NNPDF23LO_flatpT_Zprime_Extended.evgen.EVNT.e8514.33116368._004446.pool.root.1' \
    --outputHITSFile 'test.CA.HITS.pool.root' \
    --maxEvents 50 \
    --skipEvents 50 \
    --jobNumber 1 \
    --imf False \
    --athenaopts '"--config-only=ConfigSimCG.pkl"'

Sim_tf.py \
    --multithreaded \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC23aSimulationMultipleIoV.py' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/mc23_13p6TeV.800030.Py8EG_A14NNPDF23LO_flatpT_Zprime_Extended.evgen.EVNT.e8514.33116368._004446.pool.root.1' \
    --outputHITSFile 'test.CG.HITS.pool.root' \
    --maxEvents 50 \
    --skipEvents 50 \
    --jobNumber 1 \
    --imf False

rc2=$?
mv log.EVNTtoHITS log.EVNTtoHITS.CG
echo  "art-result: $rc2 simOLD"
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
    art.py compare grid ${ArtPackage} ${ArtJobName} --order-trees --mode=semi-detailed --diff-root --file=test.CG.HITS.pool.root
    rc4=$?
    status=$rc4
fi
echo  "art-result: $rc4 regression"

exit $status
