#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: art job for fsjet_vtx_pu55_MC16
# art-type: grid
# art-include: main/Athena
# art-include: 23.0/Athena
# art-input-nfiles: 3
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


Slices  = ['fsjet']
Events  = 2000 
Threads = 8 
Slots   = 8
Input   = 'ttbar_mc16'    # defined in TrigValTools/share/TrigValInputs.json  
Release = "current"

preinclude_file = "RDOtoRDOTrigger:TrigInDetValidation/TIDAvtx_preinclude.py"


Jobs = [ ( "Truth",       " TIDAdata-run3.dat                        -o data-hists.root" ), 
         ( "Offline",     " TIDAdata-run3-offline.dat     -r Offline -o data-hists-offline.root" ),
         ( "OfflineVtx",  " TIDAdata-run3-offline-vtx.dat -r Offline -o data-hists-offline-vtx.root" ) ]


Comp = [ ( "FSjet",        "L2fsjet",     "data-hists.root",              " -c TIDAhisto-panel.dat      -d HLTL2-plots         -sx Reference Truth" ),
         ( "FSjetoffline", "L2fsjet",     "data-hists-offline.root",      " -c TIDAhisto-panel.dat      -d HLTL2-plots-offline -sx Reference Offline" ),
         ( "FSvtx",        "L2fsjetvtx",  "data-hists-offline-vtx.root",  " -c TIDAhisto-panel-vtx.dat  -d HLTL2-plots-vtx     --ncols 3" ),
         ( "FSvtxall",     "L2fsjetvtx",  "data-hists-offline.root",      " -c TIDAhisto-panel-vtx.dat  -d HLTL2-plots-vtxall  --ncols 3" ) ]

from AthenaCommon.Include import include 
include("TrigInDetValidation/TrigInDetValidation_Base.py")


 
