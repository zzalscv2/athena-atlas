#!/bin/sh
#
# art-description: MC23-style RUN2 simulation using FullG4MT in serial Athena
# art-include: 23.0/Athena
# art-include: 23.0/AthSimulation
# art-include: main/Athena
# art-include: main/AthSimulation
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: test.CG.HITS.pool.root
# art-output: test.CA.HITS.pool.root
# art-output: Config*

# RUN3 setup
# ATLAS-R2-2016-01-02-01 and OFLCOND-MC21-SDR-RUN3-07
  Sim_tf.py \
      --CA \
      --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
      --simulator 'FullG4MT' \
      --postExec 'with open("ConfigCA.pkl", "wb") as f: cfg.store(f)' \
      --postInclude 'default:PyJobTransforms.UseFrontier' \
      --preInclude 'EVNTtoHITS:Campaigns.MC23SimulationNoIoV' \
      --DataRunNumber 284500 \
      --geometryVersion 'default:ATLAS-R2-2016-01-02-01' \
      --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
      --outputHITSFile "test.CA.HITS.pool.root" \
      --maxEvents 3 \
      --imf False

rc=$?
mv log.EVNTtoHITS log.EVNTtoHITS_CA
echo  "art-result: $rc simCA"
status=$rc

Sim_tf.py \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --simulator 'FullG4MT' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC23SimulationNoIoV.py' \
    --DataRunNumber 284500 \
    --geometryVersion 'default:ATLAS-R2-2016-01-02-01_VALIDATION' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
    --outputHITSFile "test.CA.HITS.pool.root" \
    --maxEvents 3 \
    --imf False \
    --athenaopts '"--config-only=ConfigCG.pkl"'

Sim_tf.py \
    --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' \
    --simulator 'FullG4MT' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC23SimulationNoIoV.py' \
    --DataRunNumber 284500 \
    --geometryVersion 'default:ATLAS-R2-2016-01-02-01_VALIDATION' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
    --outputHITSFile "test.CG.HITS.pool.root" \
    --maxEvents 3 \
    --imf False

rc2=$?
mv log.EVNTtoHITS log.EVNTtoHITS_OLD
echo "art-result: $rc2 simOLD"
if [ $status -eq 0 ]
then
    status=$rc2
fi

rc3=-9999
if [ $status -eq 0 ]
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
    art.py compare grid --entries 3 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.CG.HITS.pool.root
    rc4=$?
    if [ $status -eq 0 ]
    then
        status=$rc4
    fi
fi
echo  "art-result: $rc4 regression"

exit $status
