#!/bin/bash
#
# art-description: Run 4 pile-up pre-mixing MP tests
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-include: master/Athena
# art-output: RUN4_presampling_SP.RDO.pool.root
# art-output: RUN4_presampling_MP_fork_evt0.RDO.pool.root
# art-output: RUN4_presampling_MP_fork_evt1.RDO.pool.root

export ATHENA_CORE_NUMBER=4

DigiOutFileNameSP="RUN4_presampling_SP.RDO.pool.root"
DigiOutFileNameMP0="RUN4_presampling_MP_fork_evt0.RDO.pool.root"
DigiOutFileNameMP1="RUN4_presampling_MP_fork_evt1.RDO.pool.root"

Events=50
HSHitsFile="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-01-01-00/mc21_14TeV.900149.PG_single_nu_Pt50.simul.HITS.e8481_s4038/HITS.32496427._001002.pool.root.1"
HighPtMinbiasHitsFiles="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-01-01-00/mc21_14TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8481_s4038_s4045/*"
LowPtMinbiasHitsFiles="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-01-01-00/mc21_14TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8481_s4038_s4045/*"


Digi_tf.py \
--CA \
--PileUpPresampling True \
--conditionsTag default:OFLCOND-MC15c-SDR-14-05 \
--digiSeedOffset1 170 --digiSeedOffset2 170 \
--digiSteeringConf 'StandardSignalOnlyTruth' \
--geometryVersion default:ATLAS-P2-RUN4-01-01-00 \
--inputHITSFile ${HSHitsFile} \
--inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
--inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
--jobNumber 568 \
--maxEvents ${Events} \
--outputRDOFile ${DigiOutFileNameSP} \
--preInclude 'HITtoRDO:Campaigns.PhaseIIPileUp200' \
--postExec 'HITtoRDO:cfg.getService("PileUpEventLoopMgr").AllowSerialAndMPToDiffer=False' \
--postInclude 'PyJobTransforms.UseFrontier' \
--skipEvents 0

rc=$?
status=$rc
echo "art-result: $rc Digi_tf.py SP"

Digi_tf.py \
--multiprocess --athenaMPEventsBeforeFork 0 \
--CA \
--PileUpPresampling True \
--conditionsTag default:OFLCOND-MC15c-SDR-14-05 \
--digiSeedOffset1 170 --digiSeedOffset2 170 \
--digiSteeringConf 'StandardSignalOnlyTruth' \
--geometryVersion default:ATLAS-P2-RUN4-01-01-00 \
--inputHITSFile ${HSHitsFile} \
--inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
--inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
--jobNumber 568 \
--maxEvents ${Events} \
--outputRDOFile ${DigiOutFileNameMP0} \
--preInclude 'HITtoRDO:Campaigns.PhaseIIPileUp200' \
--postExec 'HITtoRDO:cfg.getService("PileUpEventLoopMgr").AllowSerialAndMPToDiffer=False' \
--postInclude 'PyJobTransforms.UseFrontier' \
--skipEvents 0

rc2=$?
if [ $status -eq 0 ]; then
  status=$rc2
fi
echo "art-result: $rc2 Digi_tf.py MP fork after 0"

Digi_tf.py \
--multiprocess --athenaMPEventsBeforeFork 1 \
--CA \
--PileUpPresampling True \
--conditionsTag default:OFLCOND-MC15c-SDR-14-05 \
--digiSeedOffset1 170 --digiSeedOffset2 170 \
--digiSteeringConf 'StandardSignalOnlyTruth' \
--geometryVersion default:ATLAS-P2-RUN4-01-01-00 \
--inputHITSFile ${HSHitsFile} \
--inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
--inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
--jobNumber 568 \
--maxEvents ${Events} \
--outputRDOFile ${DigiOutFileNameMP1} \
--preInclude 'HITtoRDO:Campaigns.PhaseIIPileUp200' \
--postExec 'HITtoRDO:cfg.getService("PileUpEventLoopMgr").AllowSerialAndMPToDiffer=False' \
--postInclude 'PyJobTransforms.UseFrontier' \
--skipEvents 0

rc3=$?
if [[ $status -eq 0 ]]; then
    status=$rc3
fi
echo "art-result: $rc3 Digi_tf.py MP fork after 1"

rc4=-9999
if [[ $rc -eq 0 ]] && [[ $rc2 -eq 0 ]]
then
    acmd.py diff-root ${DigiOutFileNameSP} ${DigiOutFileNameMP0} \
        --mode=semi-detailed --error-mode resilient --order-trees \
        --ignore-leaves RecoTimingObj_p1_Bkg_HITStoRDO_timings index_ref
    rc4=$?
    if [[ $status -eq 0 ]]; then
        status=$rc4
    fi
fi
echo "art-result: $rc4 SP vs MP fork after 0"

rc5=-9999
if [[ $rc -eq 0 ]] && [[ $rc3 -eq 0 ]]
then
    acmd.py diff-root ${DigiOutFileNameSP} ${DigiOutFileNameMP1} \
        --mode=semi-detailed --error-mode resilient --order-trees \
        --ignore-leaves RecoTimingObj_p1_Bkg_HITStoRDO_timings index_ref
    rc5=$?
    if [[ $status -eq 0 ]]; then
        status=$rc5
    fi
fi
echo "art-result: $rc5 SP vs MP fork after 1"

exit $status
