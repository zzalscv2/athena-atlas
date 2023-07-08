#!/bin/sh
# 
# art-description: MC16-style simulation of decaying light sleptons using FullG4 (tests Sleptons + Gauginos)
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-include: 23.0/Athena
# art-include: 23.0/AthSimulation
# art-include: main/Athena
# art-include: main/AthSimulation
# art-output: *.root
# art-output: PDGTABLE.MeV.*
# art-output: *.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

# MC16 setup
# ATLAS-R2-2016-01-00-01 and OFLCOND-MC16-SDR-14
Sim_tf.py \
--CA \
--conditionsTag 'default:OFLCOND-MC16-SDR-14' \
--physicsList 'FTFP_BERT_ATL' \
--truthStrategy 'MC15aPlusLLP' \
--simulator 'FullG4MT' \
--postInclude 'PyJobTransforms.UseFrontier' \
--preInclude 'Campaigns.MC16Simulation,G4DebuggingTools.DebugSleptonsLLP' \
--DataRunNumber '284500' \
--geometryVersion 'default:ATLAS-R2-2016-01-00-01' \
--inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/mc15_13TeV.399030.MGPy8EG_A14NNPDF23LO_SlepSlep_directLLP_100_0_0p01ns.evgen.EVNT.e7067.EVNT.16242732._000001.pool.root.1" \
--outputHITSFile "CA.HITS.pool.root" \
--maxEvents 10 \
--postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
--imf False

rc=$?
mv PDGTABLE.MeV PDGTABLE.MeV.CA
mv log.EVNTtoHITS log.EVNTtoHITS.CA
echo  "art-result: $rc simCA"
status=$rc

Sim_tf.py \
    --conditionsTag 'default:OFLCOND-MC16-SDR-14' \
    --truthStrategy 'MC15aPlusLLP' \
    --simulator 'FullG4MT' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC16Simulation.py,SimulationJobOptions/preInclude.DebugSleptonsLLP.py' \
    --geometryVersion 'default:ATLAS-R2-2016-01-00-01' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/mc15_13TeV.399030.MGPy8EG_A14NNPDF23LO_SlepSlep_directLLP_100_0_0p01ns.evgen.EVNT.e7067.EVNT.16242732._000001.pool.root.1" \
    --outputHITSFile "CA.HITS.pool.root" \
    --maxEvents 10 \
    --imf False \
    --athenaopts '"--config-only=ConfigSimCG.pkl"'

Sim_tf.py \
    --conditionsTag 'default:OFLCOND-MC16-SDR-14' \
    --truthStrategy 'MC15aPlusLLP' \
    --simulator 'FullG4MT' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC16Simulation.py,SimulationJobOptions/preInclude.DebugSleptonsLLP.py' \
    --geometryVersion 'default:ATLAS-R2-2016-01-00-01' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/mc15_13TeV.399030.MGPy8EG_A14NNPDF23LO_SlepSlep_directLLP_100_0_0p01ns.evgen.EVNT.e7067.EVNT.16242732._000001.pool.root.1" \
    --outputHITSFile "CG.HITS.pool.root" \
    --maxEvents 10 \
    --imf False

rc2=$?
status=$rc2
mv PDGTABLE.MeV PDGTABLE.MeV.CG
mv log.EVNTtoHITS log.EVNTtoHITS.CG
echo  "art-result: $rc2 simOLD"

rc3=-9999
if [ $rc -eq 0 ] && [ $rc2 -eq 0 ]
then
    acmd.py diff-root CG.HITS.pool.root CA.HITS.pool.root --error-mode resilient --mode=semi-detailed --order-trees --ignore-leaves RecoTimingObj_p1_EVNTtoHITS_timings index_ref
    rc3=$?
    status=$rc3
fi
echo  "art-result: $rc3 OLDvsCA"

rc4=-9999
if [ $rc2 -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=CG.HITS.pool.root
    rc4=$?
    status=$rc4
fi
echo  "art-result: $rc4 regression"

exit $status
