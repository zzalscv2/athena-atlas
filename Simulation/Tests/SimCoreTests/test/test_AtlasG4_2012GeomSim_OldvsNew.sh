#!/bin/sh
#
# art-description: Run simulation outside ISF, using 2012 geometry and conditions, reading single muon events, writing HITS
# art-include: 21.0/Athena
# art-include: 21.0/AthSimulation
# art-include: 21.3/Athena
# art-include: 21.9/Athena
# art-include: master/Athena
# art-include: master/AthSimulation
# art-type: grid
# art-output: test.HITS.pool.root

AtlasG4_tf.py \
--CA \
--conditionsTag 'OFLCOND-RUN12-SDR-19' \
--physicsList 'FTFP_BERT' \
--postInclude 'PyJobTransforms.TransformUtils.UseFrontier' \
--DataRunNumber '212272' \
--geometryVersion 'ATLAS-R1-2012-03-00-00' \
--inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/mu_E200_eta0-25.evgen.pool.root" \
--outputHITSFile "test.NEW.HITS.pool.root" \
--maxEvents '10' \
--skipEvents '0' \
--randomSeed '10' \
--imf False

rc=$?
mv log.AtlasG4Tf log.G4AtlasAlg_AthenaCA
echo  "art-result: $rc G4AtlasAlg_AthenaCA"
rc2=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    AtlasG4_tf.py \
    --conditionsTag 'OFLCOND-RUN12-SDR-19' \
    --physicsList 'FTFP_BERT' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --DataRunNumber '212272' \
    --geometryVersion 'ATLAS-R1-2012-03-00-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/mu_E200_eta0-25.evgen.pool.root" \
    --outputHITSFile "test.OLD.HITS.pool.root" \
    --maxEvents '10' \
    --skipEvents '0' \
    --randomSeed '10' \
    --truthStrategy 'MC15aPlus' \
    --imf False
    mv log.AtlasG4Tf log.G4AtlasAlg_AthenaCA_OLD
    rc2=$?

fi
echo  "art-result: $rc2 G4AtlasAlg_AthenaCA_OLD"
rc4=-9999
if [ $rc2 -eq 0 ]
then
    acmd.py diff-root test.OLD.HITS.pool.root test.NEW.HITS.pool.root --error-mode resilient --mode=semi-detailed --order-trees --ignore-leaves RecoTimingObj_p1_AtlasG4Tf_timings index_ref
    rc4=$?
fi
echo  "art-result: $rc4 FullG4MT_OLDvsCA"
