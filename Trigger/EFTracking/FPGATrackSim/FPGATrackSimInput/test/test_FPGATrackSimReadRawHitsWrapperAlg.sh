#!/bin/sh
source `which FPGATrackSimInputTestSetup.sh`

export InputFPGATrackSimRawHitFile=$FPGATrackSimRawHitFile
export OutputFPGATrackSimRawHitFile="httsim_rawhits_wrap.OUT.root"
athena --evtMax=5  FPGATrackSimInput/FPGATrackSimReadRawHitsWrapperAlg_jobOptions.py

