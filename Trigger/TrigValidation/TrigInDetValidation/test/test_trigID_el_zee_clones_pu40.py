#!/usr/bin/env python

# art-description: art job for el_zee_clones_pu40
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


Slices  = ['electron']
Events  = 16000
Threads = 8 
Slots   = 8
Input   = 'Zee_pu40'    # defined in TrigValTools/share/TrigValInputs.json
Release = "current"
GridFiles = True
preinclude_file = "TrigInDetValidation/TIDAcloneremoval.py"

Jobs = [ ( "Truth",       " TIDAdata-run3.dat                    -o data-hists.root -p 11" ),
         ( "Offline",     " TIDAdata-run3-offline.dat -r Offline -o data-hists-offline.root" ) ]

Comp = [ ( "L2ele",              "L2electron",      "data-hists.root",         " -c TIDAhisto-panel.dat  -d HLTL2-plots " ),
         ( "L2ele-lowpt",        "L2electronLowpt", "data-hists.root",         " -c TIDAhisto-panel.dat  -d HLTL2-plots-lowpt " ),
         ( "L2eleoffline",       "L2electron",      "data-hists-offline.root", " -c TIDAhisto-panel.dat  -d HLTL2-plots-offline " ),
         ( "L2eleoffline-lowpt", "L2electronLowpt", "data-hists-offline.root", " -c TIDAhisto-panel.dat  -d HLTL2-plots-lowpt-offline " ),
         ( "EFele",              "EFelectron",      "data-hists.root",         " -c TIDAhisto-panel.dat  -d HLTEF-plots " ),
         ( "EFele-lowpt",        "EFelectronLowpt", "data-hists.root",         " -c TIDAhisto-panel.dat  -d HLTEF-plots-lowpt " ),
         ( "EFeleoffline",       "EFelectron",      "data-hists-offline.root", " -c TIDAhisto-panel.dat  -d HLTEF-plots-offline " ),
         ( "EFeleoffline-lowpt", "EFelectronLowpt", "data-hists-offline.root", " -c TIDAhisto-panel.dat  -d HLTEF-plots-lowpt-offline " ) ]


from AthenaCommon.Include import include 
include("TrigInDetValidation/TrigInDetValidation_Base.py")
