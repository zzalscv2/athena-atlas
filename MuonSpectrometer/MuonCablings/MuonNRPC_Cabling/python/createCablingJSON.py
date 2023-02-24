# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
import json
from copy import deepcopy

###     Simple script to generate a BIS78 cabling map as used for the
###     Monte Carlo processing. The channel numbers are place holders
###     for the final ones
station_name = 1 #BIS
eta_index = 7  # To represent BIS78
doubletR = 1   # Chamber mounted below the Mdts
doubletPhi = 1  # Single phi module
sub_detector = 666 #
json_dict = []
for sector in range(1, 9):
    for doubZ in [1, 2]:
        for measPhi in [False, True]:
            for gasGap in [1, 2, 3]:
                cabling_data = {
                    ### Offline part of the identifier
                    "station": station_name,
                    "eta": eta_index,
                    "phi" : sector,
                    "doubletR" : doubletR,
                    "doubletZ" : doubZ,
                    "doubletPhi": doubletPhi,
                    "measPhi": measPhi,
                    "gasGap": gasGap,
                    ### Online part
                    "subDetector": sub_detector,
                    "tdcSector" : sector,                    
                }
                ### TDC 
                if doubZ == 1:
                    cabling_data["firstStrip"] = 1
                    cabling_data["lastStrip"] = 32
                    cabling_data["tdc"] = gasGap + 3*measPhi
                elif doubZ == 2: ## BIS8
                    cabling_data["firstStrip"] = 1
                    cabling_data["lastStrip"] = 16 + measPhi * 16
                    cabling_data["tdc"] = gasGap + (6+ 3*measPhi)
                
                json_dict.append(deepcopy(cabling_data))

                if doubZ == 1:
                    cabling_data["tdc"] = gasGap + (12 + measPhi *3)
                    if not measPhi:
                       cabling_data["firstStrip"] = 33
                       cabling_data["lastStrip"] = 40
                    else:
                       cabling_data["firstStrip"] = 33
                       cabling_data["lastStrip"] = 64
                else:
                    cabling_data["tdc"] = gasGap + (15 + measPhi *3)
                    if measPhi:
                       cabling_data["firstStrip"] = 33
                       cabling_data["lastStrip"] = 64
                    else: continue
                ## Append the object
                json_dict.append(deepcopy(cabling_data))
print (len(json_dict))
with open("CablingFile.json", "w") as my_file:
    my_file.write(json.dumps(json_dict))
