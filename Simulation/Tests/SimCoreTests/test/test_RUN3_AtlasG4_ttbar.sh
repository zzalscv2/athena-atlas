#!/bin/sh
#
# art-description: Run G4AtlasAlg simulation, reading ttbar events, writing HITS, using RUN2 geometry and conditions
# art-include: 23.0/Athena
# art-include: 23.0/AthSimulation
# art-include: master/Athena
# art-include: master/AthSimulation
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: log.*
# art-output: *.pool.root
# art-output: Config*.pkl

AtlasG4_tf.py \
    --CA \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --preInclude 'AtlasG4Tf:Campaigns.MC23SimulationMultipleIoV' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
    --outputHITSFile "test.CA.HITS.pool.root" \
    --maxEvents 50 \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False

rc=$?
mv log.AtlasG4Tf log.AtlasG4Tf_CA
echo  "art-result: $rc simCA"
status=$rc

AtlasG4_tf.py \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'AtlasG4Tf:Campaigns/MC23SimulationMultipleIoV.py' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
    --outputHITSFile "test.CA.HITS.pool.root" \
    --maxEvents 50 \
    --imf False \
    --athenaopts '"--config-only=ConfigSimCG.pkl"'

AtlasG4_tf.py \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'AtlasG4Tf:Campaigns/MC23SimulationMultipleIoV.py' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
    --outputHITSFile "test.HITS.pool.root" \
    --maxEvents 50 \
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
    acmd.py diff-root test.HITS.pool.root test.CA.HITS.pool.root --error-mode resilient --mode=semi-detailed
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
