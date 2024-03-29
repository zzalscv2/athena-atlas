#!/bin/sh
#
# art-description: MC23-style simulation using FullG4MT_QS (13 TeV ttbar input - needs updating)
# art-include: 23.0/Athena
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: *.pool.root
# art-output: log.*
# art-output: Config*.pkl

Sim_tf.py \
--CA True \
--conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
--simulator 'FullG4MT_QS' \
--postInclude 'PyJobTransforms.TransformUtils.UseFrontier' \
--preInclude 'EVNTtoHITS:Campaigns.MC23SimulationSingleIoV' \
--geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
--inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1' \
--outputHITSFile 'test.CA.HITS.pool.root' \
--maxEvents '10' \
--postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
--imf False

rc=$?
status=$rc
mv log.EVNTtoHITS log.EVNTtoHITS_CA
echo  "art-result: $rc simCA"

Sim_tf.py \
--conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
--simulator 'FullG4MT_QS' \
--postInclude 'default:PyJobTransforms/UseFrontier.py' \
--preInclude 'EVNTtoHITS:Campaigns/MC23SimulationSingleIoV.py' \
--geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
--inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1' \
--outputHITSFile 'test.CA.HITS.pool.root' \
--maxEvents '10' \
--imf False \
--athenaopts '"--config-only=ConfigSimCG.pkl"'

Sim_tf.py \
--conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
--simulator 'FullG4MT_QS' \
--postInclude 'default:PyJobTransforms/UseFrontier.py' \
--preInclude 'EVNTtoHITS:Campaigns/MC23SimulationSingleIoV.py' \
--geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
--inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1' \
--outputHITSFile 'test.HITS.pool.root' \
--maxEvents '10' \
--imf False

rc2=$?
if [ $status -eq 0 ]
then
    status=$rc2
fi
mv log.EVNTtoHITS log.EVNTtoHITS_OLD
echo  "art-result: $rc2 simOLD"

rc3=-9999
if [ $status -eq 0 ]
then
    acmd.py diff-root test.HITS.pool.root test.CA.HITS.pool.root --error-mode resilient --mode=semi-detailed --order-trees --ignore-leaves RecoTimingObj_p1_EVNTtoHITS_timings index_ref
    rc3=$?
    status=$rc3
fi
echo  "art-result: $rc3 OLDvsCA"

rc4=-9999
if [ $rc2 -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.HITS.pool.root
    rc4=$?
    if [ $status -eq 0 ]
    then
        status=$rc4
    fi
fi
echo  "art-result: $rc4 regression"
exit $status
