#!/bin/sh
source `which FPGATrackSimInputTestSetup.sh`

export InputFPGATrackSimRawHitFile=$FPGATrackSimRawHitFile
export OutputFPGATrackSimRawHitFile="fpgatracksim_loghits_wrap.OUT.root"

athena --evtMax=5  FPGATrackSimInput/FPGATrackSimRawToLogicalHitsWrapperAlg_jobOptions.py

echo "Produced file $OutputFPGATrackSimRawHitFile, now read it back"

export InputFPGATrackSimLogHitFile=$OutputFPGATrackSimRawHitFile
athena --evtMax=5 FPGATrackSimInput/FPGATrackSimReadLogicalHitsAlg_jobOptions.py




