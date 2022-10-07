#!/usr/bin/env python
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# art-description: art job for el_zee_tier0_pu40
# art-type: grid
# art-include: master/Athena
# art-include: 22.0/Athena
# art-input: valid1.601189.PhPy8EG_AZNLO_Zee.recon.RDO.e8453_e8455_s3873_s3874_r13929_tid30659542_00
# art-input-nfiles: 2
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
Events  = 4000
Threads = 8
Slots   = 8
Release = "current"

ExtraAna = " -c doTIDATier0=True "

Input   = 'Zee'    # defined in TrigValTools/share/TrigValInputs.json  
GridFiles = True

Jobs = [] 

Comp = [ ( "L2electron",   "L2electron",  "data-hists-tier0.root",   " -b HLT/TRIDT/Egamma/Expert   -s '_HLT_IDTrack' '/HLT_IDTrack' -c TIDAhisto-tier0.dat --ncols 3 -d HLTL2-plots-electron " ),
         ( "EFelectron",   "EFelectron",  "data-hists-tier0.root",   " -b HLT/TRIDT/Egamma/Expert   -s '_HLT_IDTrack' '/HLT_IDTrack' -c TIDAhisto-tier0.dat --ncols 3 -d HLTEF-plots-electron " ) ]
   
from AthenaCommon.Include import include 
include("TrigInDetValidation/TrigInDetValidation_Base.py")
