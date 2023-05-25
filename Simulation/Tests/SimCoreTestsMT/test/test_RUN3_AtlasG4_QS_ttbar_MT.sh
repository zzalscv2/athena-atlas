#!/bin/sh
#
# art-description: Run G4AtlasAlg simulation, reading ttbar events, writing HITS, using RUN3 geometry and conditions and quasi-stable particle simulation
# art-include: 23.0/Athena
# art-include: 23.0/AthSimulation
# art-include: master/Athena
# art-include: master/AthSimulation
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-output: log.*
# art-output: *.pool.root
# art-output: Config*.pkl

export ATHENA_CORE_NUMBER=8

AtlasG4_tf.py \
    --CA \
    --multithreaded \
    --simulator 'AtlasG4_QS' \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --preInclude 'AtlasG4Tf:Campaigns.MC23aSimulationMultipleIoV' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
    --outputHITSFile "test.CA.HITS.pool.root" \
    --maxEvents 50 \
    --jobNumber 857 \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False

rc=$?
mv log.AtlasG4Tf log.AtlasG4Tf_CA
echo  "art-result: $rc simCA"
status=$rc

AtlasG4_tf.py \
    --multithreaded \
    --simulator 'AtlasG4_QS' \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'AtlasG4Tf:Campaigns/MC23aSimulationMultipleIoV.py' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
    --outputHITSFile "test.CA.HITS.pool.root" \
    --maxEvents 50 \
    --jobNumber 857 \
    --imf False \
    --athenaopts '"--config-only=ConfigSimCG.pkl"'

AtlasG4_tf.py \
    --multithreaded \
    --simulator 'AtlasG4_QS' \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'AtlasG4Tf:Campaigns/MC23aSimulationMultipleIoV.py' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
    --outputHITSFile "test.HITS.pool.root" \
    --maxEvents 50 \
    --jobNumber 857 \
    --imf False
rc2=$?
mv log.AtlasG4Tf log.AtlasG4Tf_OLD
echo  "art-result: $rc2 simOLD"
if [ $status -eq 0 ]
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
    art.py compare grid --entries 50 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.HITS.pool.root --order-trees
    rc4=$?
    status=$rc4
fi
echo  "art-result: $rc4 regression"

exit $status
