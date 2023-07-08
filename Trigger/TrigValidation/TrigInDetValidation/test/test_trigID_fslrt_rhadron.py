#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: art job for fslrt_rhadron
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


Slices  = ['FSLRT']
Events  = 8000
Threads = 8
Slots   = 8
Input   = 'RHadron'    # defined in TrigValTools/share/TrigValInputs.json
GridFiles = False
ExtraAna = ' -c LRT="True" '
Release = "current"

preinclude_file = 'RDOtoRDOTrigger:TrigInDetValidation/TIDAlrt_preinclude.py'

Jobs = [ ( "Truth",  " TIDAdata-run3-fslrt.dat -o data-hists.root ", "Test_bin_lrt.dat" ),
         ( "Offline",    " TIDAdata-run3-offline-fslrt.dat -r Offline+InDetLargeD0TrackParticles -o data-hists-offline.root", "Test_bin_lrt.dat" ) ]

Comp = [ ( "L2FSLRT",  "L2FSLRT",  "data-hists.root",  " -c TIDAhisto-panel.dat -d HLTL2-plots -sx Reference Truth " ),
         ( "EFFSLRT",  "EFFSLRT", "data-hists.root",   " -c TIDAhisto-panel.dat -d HLTEF-plots -sx Reference Truth   " ),
         ( "L2FSLRToffline",   "L2FSLRT","data-hists-offline.root",   " -c TIDAhisto-panel.dat -d HLTL2-plots-offline -sx Reference Offline " ),
         ( "EFFSLRToffline",   "EFFSLRT", "data-hists-offline.root",   " -c TIDAhisto-panel.dat -d HLTEF-plots-offline -sx Reference Offline " )
       ]


from AthenaCommon.Include import include
include("TrigInDetValidation/TrigInDetValidation_Base.py")
