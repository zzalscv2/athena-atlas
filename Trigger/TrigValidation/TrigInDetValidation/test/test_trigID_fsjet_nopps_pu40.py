#!/usr/bin/env python

# art-description: art job for fsjet_pu40_new
# art-type: grid
# art-include: master/Athena
# art-input-nfiles: 3
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

import os
os.system("echo 'ftf = findAlgorithm(topSequence, \"TrigFastTrackFinder__jet\")' > dopps.py")
os.system("echo 'ftf.TripletDoPPS=False' >> dopps.py")

Slices  = ['fsjet']
RunEF   = False
Events  = 2000 
Threads = 8 
Slots   = 8
postinclude_file = 'dopps.py'
Input   = 'ttbar'    # defined in TrigValTools/share/TrigValInputs.json  

Jobs = [ ( "Truth",       " TIDAdata-run3.dat              -o data-hists.root" ), 
         ( "Offline",     " TIDAdata-run3-offline.dat      -o data-hists-offline.root" ),
         ( "OfflineVtx",  " TIDAdata-run3-offline-vtx.dat  -o data-hists-offline-vtx.root" ) ]


Comp = [ ( "FSjet",        "L2fsjet",     "data-hists.root",              " -c TIDAhisto-panel.dat      -d HLTL2-plots " ),
         ( "FSjetoffline", "L2fsjet",     "data-hists-offline.root",      " -c TIDAhisto-panel.dat      -d HLTL2-plots-offline " ),
         ( "FSvtx",        "L2fsjetvtx",  "data-hists-offline-vtx.root",  " -c TIDAhisto-panel-vtx.dat  -d HLTL2-plots-vtx     --ncols 3" ),
         ( "FSvtxall",     "L2fsjetvtx",  "data-hists-offline.root",      " -c TIDAhisto-panel-vtx.dat  -d HLTL2-plots-vtxall  --ncols 3" ) ]


from AthenaCommon.Include import include 
include("TrigInDetValidation/TrigInDetValidation_NewBase.py")


 
