#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: art job for mu_bphys
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


Slices  = ['bphys']
Events  = 6000 
Threads = 8 
Slots   = 8
Input   = 'Bphys_JpsiPhi'    # defined in TrigValTools/share/TrigValInputs.json

preinclude_file = 'RDOtoRDOTrigger:TrigInDetValidation/TIDV_cond_fix.py' #conditions fix for ATR-23982. In future find a more recent RDO
postinclude_file = 'RAWtoALL:TrigInDetValidation.TIDV_cond_fix'

ExtraAna = " -c 'parentpdgid=531' "

Jobs = [ ( "Truth",       " TIDAdata-run3-larged0.dat                    -o data-hists.root " ),
         ( "Offline",     " TIDAdata-run3-offline-larged0.dat -r Offline -o data-hists-offline.root" ) ]

Comp = [ ( "L2bphys",             "L2bphys",    "data-hists.root",         " -c TIDAhisto-panel.dat  -d HLTL2-plots " ),
         ( "L2bphysoffline",      "L2bphys",    "data-hists-offline.root", " -c TIDAhisto-panel.dat  -d HLTL2-plots-offline " ),
         ( "EFbphys",             "EFbphys",    "data-hists.root",         " -c TIDAhisto-panel.dat  -d HLTEF-plots " ),
         ( "EFbphysoffline",      "EFbphys",    "data-hists-offline.root", " -c TIDAhisto-panel.dat  -d HLTEF-plots-offline " ) ]


from AthenaCommon.Include import include 
include("TrigInDetValidation/TrigInDetValidation_Base.py")



 
