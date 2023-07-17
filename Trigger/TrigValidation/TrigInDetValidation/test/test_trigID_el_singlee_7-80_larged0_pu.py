#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: art job for el_singlee_7-80_larged0_pu
# art-type: grid
# art-include: main/Athena
# art-include: 23.0/Athena
# art-input: valid1.901969.PG_e_Et7to80_vertxy20.recon.RDO.e8544_e8528_s4159_s4114_r14800_tid34086068_00
# art-input-nfiles: 10
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


Slices  = ['electron']
Events  = 20000 
Threads = 8 
Slots   = 8
Input   = 'Single_el_larged0_pu40'    # defined in TrigValTools/share/TrigValInputs.json
GridFiles=True

Jobs = [ ( "Truth",       " TIDAdata-run3-larged0-el.dat                    -o data-hists.root -p 11",   "Test_bin_larged0.dat" ),
         ( "Offline",     " TIDAdata-run3-offline-larged0-el.dat -r Offline -o data-hists-offline.root", "Test_bin_larged0.dat" ) ]

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
