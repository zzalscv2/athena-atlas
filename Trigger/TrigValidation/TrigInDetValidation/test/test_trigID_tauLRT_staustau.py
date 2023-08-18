#!/usr/bin/env python

# art-description: art job for tauLRT_staustau
# art-type: grid
# art-include: main/Athena
# art-include: 23.0/Athena
# art-input: valid1.516640.MGPy8EG_A14NNPDF23LO_StauStauLLP_100_0_1ns.recon.RDO.e8514_e8528_s4159_s4114_r14844_tid34200275_00
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


Slices  = ['tauLRT']
Events  = 5000
Threads = 8
Slots   = 8
Release = "current"
Input   = 'StauStau'    # defined in TrigValTools/share/TrigValInputs.json
GridFiles = True

ExtraAna = " -c 'LRT=True;parentpdgid=15' "

preinclude_file = 'RDOtoRDOTrigger:TrigInDetValidation/TIDAlrt_preinclude.py'

Jobs = [ ( "Offline",  " TIDAdata-run3-offline-lrt.dat -r Offline+InDetLargeD0TrackParticles -o data-hists-offline-lrt.root", "Test_bin_lrt.dat" ),
         ( "Truth",    " TIDAdata-run3-lrt.dat                    -o data-hists-lrt.root",         "Test_bin_lrt.dat" ) ]

Comp = [ ( "EFtauLRT",       "EFtauLRT",      "data-hists-lrt.root",         " -c TIDAhisto-panel.dat  -d HLTEF-plots " ),
         ( "L2tauLRT",       "L2tauLRT",      "data-hists-lrt.root",         " -c TIDAhisto-panel.dat  -d HLTL2-plots " ),
         ( "EFtauLRTOff",    "EFtauLRT",      "data-hists-offline-lrt.root", " -c TIDAhisto-panel.dat  -d HLTEF-plots-offline " ),
         ( "L2tauLRTOff",    "L2tauLRT",      "data-hists-offline-lrt.root", " -c TIDAhisto-panel.dat  -d HLTL2-plots-offline " ) ]


from AthenaCommon.Include import include
include("TrigInDetValidation/TrigInDetValidation_Base.py")
