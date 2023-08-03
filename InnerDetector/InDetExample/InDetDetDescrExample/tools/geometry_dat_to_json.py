# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# This script converts the geometry.dat file output obtained from the run of RunPrintSiDetElements.py into a JSON file.
import json
   
data = {}
    
with open("geometry.dat") as f:
    for l in f:
            
        if l.startswith('#'):
            continue
            
        bec, ld, phi, eta, side, ID = l.split()[2:8]
        data[ID] = {"BEC" : bec, "LayerDisk" : ld, "PhiModule" : phi, "EtaModule" : eta, "Side" : side }

with open("geometry" + ".json", "w") as f:
    json.dump(data, f, indent = 4)