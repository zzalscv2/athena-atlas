#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: art job for cosmic
# art-type: grid
# art-include: main/Athena
# art-include: 23.0/Athena
# art-athena-mt: 8
# art-html: https://idtrigger-val.web.cern.ch/idtrigger-val/TIDAWeb/TIDAart/?jobdir=
# art-output: *.txt
# art-output: *.log
# art-output: log.*
# art-output: *.out
# art-output: *.err
# art-output: *.log.tar.gz
# art-output: *.new
# art-output: *.json
# art-output: d*.root
# art-output: e*.root
# art-output: T*.root
# art-output: *.check*
# art-output: HLT*
# art-output: times*
# art-output: cost-perCall
# art-output: cost-perEvent
# art-output: cost-perCall-chain
# art-output: cost-perEvent-chain
# art-output: *.dat 


Slices  = ['cosmic']
Events  = 4000
Threads = 8 
Slots   = 8
Release = "current"
preinclude_file = 'RDOtoRDOTrigger:TrigInDetValidation/TIDAcosmic_preinclude.py'
preexec_reco = [
  "from AthenaCommon.BeamFlags import jobproperties",
  "jobproperties.Beam.beamType.set_Value_and_Lock('cosmics')",
  "from InDetRecExample.InDetJobProperties import InDetFlags",
  "InDetFlags.doCosmics.set_Value_and_Lock(True)",
  "InDetFlags.doTRTStandalone.set_Value_and_Lock(False)",
  "InDetFlags.doR3LargeD0.set_Value_and_Lock(False)",
  "InDetFlags.doForwardTracks.set_Value_and_Lock(False)",
]
Input   = 'mc_cosmics'    # defined in TrigValTools/share/TrigValInputs.json  

Jobs = [ ( "Offline",     " TIDAdata-run3-offline-cosmic.dat      -r Offline -o data-hists-offline.root" ) ]


Comp = [  ("EFcosmic",       "EFcosmic",      "data-hists-offline.root",   " -c TIDAhisto-panel.dat  -d HLTEF-plots " ) ]
   
from AthenaCommon.Include import include 
include("TrigInDetValidation/TrigInDetValidation_Base.py")

