#!/bin/sh
#
# art-description: Test muon reconstruction on best knowledge Run4 muon layout
# 
# art-type: grid
# art-include: main/Athena
# art-include: 22.0/Athena
# art-include: 21.9/Athena
# art-output: trkPerformance_MuonSpectrometerTracks.txt
# art-output: trkPerformance_ExtrapolatedMuonTracks.txt
# art-output: trkPerformance_MSOnlyExtrapolatedMuonTracks.txt
# art-output: trkPerformance_CombinedMuonTracks.txt
# art-output: muonPerformance_segments.txt
# art-output: muonPerformance_xAOD.txt
# art-output: warningCount.txt
# art-output: NSWRecoCheck.txt
# art-output: NSWPRDValAlg.reco.ntuple.root

#####################################################################
# run reconstruction on 2000 di-muon events (0.9<|eta|<2.8) using the best knowledge Run4 muon layout (MuonSpectrometer-R.10.01) on top of the symmetric Run3 setup (ATLAS-R3S-2021-01-00-02)
# the input RDO was produced from /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/EVNT_DiMuon_10k__AbsEta_09_28__Pt_10_1000GeV.root
# simulation/digitisation was executed in Athena,22.0.34
Reco_tf.py --inputRDOFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/Run4/RDO/RDO_DiMuon_Endcap_R4LatestLayout_v1.root \
           --preExec "from MuonRecExample.MuonRecFlags import muonRecFlags;muonRecFlags.setDefaults();muonRecFlags.useLooseErrorTuning.set_Value_and_Lock(True);muonRecFlags.doTrackPerformance=True;muonRecFlags.TrackPerfSummaryLevel=2;muonRecFlags.TrackPerfDebugLevel=5;from RecExConfig.RecFlags import rec;rec.doTrigger=False;rec.doEgamma=True;rec.doLucid=False;rec.doZdc=False;rec.doJetMissingETTag=True;from MuonRecExample.MuonStandaloneFlags import muonStandaloneFlags;muonStandaloneFlags.printSummary=True;" \
           --autoConfiguration everything \
           --conditionsTag "default:OFLCOND-MC21-SDR-RUN4-01" \
           --imf False \
           --postInclude MuonPRDTest/NSWPRDValAlg.reco.py \
           --postExec 'database_layout="MuonSpectrometer-R.10.01";include("MuonGeoModel/InitGeoFromLocal_postIncl.py");conddb.addOverride("/MDT/RTBLOB","MDTRT_Sim-Run4-01");conddb.addOverride("/MDT/T0BLOB","MDTT0_Sim-Run4-01")' \
           --outputESDFile OUT_ESD.root
exit_code=$?
echo  "art-result: ${exit_code} Reco_tf.py"
if [ ${exit_code} -ne 0 ]
then
    exit ${exit_code}
fi
# check the log file for WARNING/ERROR/FATAL
LOG_RECO="log.RAWtoESD"
NWARNING="$(cat ${LOG_RECO} | grep WARNING | wc -l)"
NERROR="$(cat ${LOG_RECO} | grep ERROR | wc -l)"
NFATAL="$(cat ${LOG_RECO} | grep FATAL | wc -l)"
echo "Found ${NWARNING} WARNING, ${NERROR} ERROR and ${NFATAL} FATAL messages in ${LOG_RECO}" > warningCount.txt
#####################################################################
# check the NSW validation ntuple
python $Athena_DIR/bin/checkNSWValTree.py -i NSWPRDValAlg.reco.ntuple.root --checkPRD &> NSWRecoCheck.txt
exit_code=$?
echo  "art-result: ${exit_code} NSWRecoCheck"
if [ ${exit_code} -ne 0 ]
then
    exit ${exit_code}
fi
#####################################################################

echo "art-result: $?"
