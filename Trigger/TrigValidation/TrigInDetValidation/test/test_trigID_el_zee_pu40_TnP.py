#!/usr/bin/env python

# art-description: art job for el_zee_pu40
# art-type: grid
# art-include: master/Athena
# art-input: mc15_13TeV.361106.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Zee.recon.RDO.e3601_s2665_s2183_r7191
# art-input-nfiles: 8
# art-athena-mt: 8
# art-memory: 4096
# art-html: https://idtrigger-val.web.cern.ch/idtrigger-val/TIDAWeb/TIDAart/?jobdir=
# art-output: *.txt
# art-output: *.log
# art-output: log.*
# art-output: *.out
# art-output: *.err
# art-output: *.log.tar.gz
# art-output: *.new
# art-output: *.json
# art-output: *.root
# art-output: *.check*
# art-output: HLT*
# art-output: times*
# art-output: cost-perCall
# art-output: cost-perEvent
# art-output: cost-perCall-chain
# art-output: cost-perEvent-chain
# art-output: *.dat 


Slices  = ['electron-tnp']
Events  = 16000
Threads = 8 
Slots   = 8
Input   = 'Zee_pu40'    # defined in TrigValTools/share/TrigValInputs.json
Release = "current"
GridFiles = True
preinclude_file = 'all:TrigInDetValidation/TIDV_cond_fix.py' #conditions fix for ATR-23982. In future find a more recent RDO 

Jobs = [( "Offline",     " TIDAdata-run3-offline-TnP.dat -r Offline -o data-hists-offline-TnP.root" ) ]

Comp = [( "TagnProbe",          "TagnProbe",       "data-hists-offline-TnP.root", " -c TIDAhisto-panel-TnP.dat  -d HLT-plots-TnP " ) ]




from AthenaCommon.Include import include 
include("TrigInDetValidation/TrigInDetValidation_Base.py")
