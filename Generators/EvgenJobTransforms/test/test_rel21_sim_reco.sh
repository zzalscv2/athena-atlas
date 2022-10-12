#!/bin/sh

# art-include: 21.6/AthGeneration
# art-include: master/Athena
# art-description: Pythia event generation -- Zprime to tautau
# art-architecture: '#x86_64-intel'
# art-type: grid
# art-output: *.root

Sim_tf.py --inputEVNTFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/EVGEN_21p6/mgpy8eg_ttbar_13p6.EVGEN.root --maxEvents 10 --runNumber 421452 --firstEvent 759001 --randomSeed 760 --outputHITSFile HITS.28723600._001356.pool.root.1 --simulator 'FullG4MT_QS' --conditionsTag 'OFLCOND-MC21-SDR-RUN3-05' --geometryVersion 'ATLAS-R3S-2021-03-00-00' --preInclude 'EVNTtoHITS:Campaigns/MC21Simulation.py,SimulationJobOptions/preInclude.ExtraParticles.py,SimulationJobOptions/preInclude.G4ExtraProcesses.py' --postInclude 'RecJobTransforms/UseFrontier.py'

echo "art-result: $? Sim_tf"

export ATHENA_CORE_NUMBER=1
Reco_tf.py --inputHITSFile=HITS.28723600._001356.pool.root.1 --athenaMPEventsBeforeFork=1 --deleteIntermediateOutputfiles=True --maxEvents=10 --multithreaded=True --postInclude "default:PyJobTransforms/UseFrontier.py" --preExec "HITtoRDO:userRunLumiOverride={\"run\":410000, \"startmu\":25.0,\"endmu\":52.0,\"stepmu\":1,\"startlb\":1,\"timestamp\":1625000000};" --preInclude "all:Campaigns/MC21a.py" "HITtoRDO:Campaigns/PileUpMC21aSingleBeamspot.py" --skipEvents=0 --autoConfiguration=everything --valid=True --conditionsTag "default:OFLCOND-MC21-SDR-RUN3-07" --geometryVersion="default:ATLAS-R3S-2021-03-00-00" --runNumber=421452 --digiSeedOffset1=10 --digiSeedOffset2=10 --digiSteeringConf='StandardSignalOnlyTruth'  --steering "doRDO_TRIG" "doTRIGtoALL" --inputHighPtMinbiasHitsFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/HITS/mc21_13p6TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8453_e8455_s3879_s3880/500events.HITS.pool.root --inputLowPtMinbiasHitsFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/HITS/mc21_13p6TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8453_s3879_s3880/5000events.HITS.pool.root --numberOfHighPtMinBias=0.103 --numberOfLowPtMinBias=52.397 --pileupFinalBunch=6 --pileupInitialBunch=-32 --outputAODFile=AOD.28723600._001356.pool.root.1 --jobNumber=10 

echo "art-result: $? Reco_tf"
