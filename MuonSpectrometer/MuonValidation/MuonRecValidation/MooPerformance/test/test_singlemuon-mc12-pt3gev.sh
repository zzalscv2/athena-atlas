#!/bin/sh
#
# art-description: Art test job for MooPerformance package, migrated from RTT job: SingleMuons-mc12-Pt3GeV
# 
# art-type: grid
# art-input: user.zhidong.SingleMuon_3GeV_task12089572.nopileup.HITStoRDO.r9707.20170907.v01_EXT0
# art-input-nfiles: 10
# art-output: trk*.txt

fileList="['${ArtInFile//,/', '}��]"
set -e

athena.py -c "from AthenaCommon.AthenaCommonFlags import athenaCommonFlags; athenaCommonFlags.EvtMax=20000; athenaCommonFlags.FilesInput = $fileList" -b MooPerformance/MooPerformance_topOptions.py
