# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from ..Base.L1MenuFlags import L1MenuFlags
from ..Base.MenuConfObj import TopoMenuDef
import TriggerMenuMT.L1.Menu.Menu_Physics_pp_run3_v1_inputs as phys_menu_inputs


def defineInputsMenu():
    
    phys_menu_inputs.defineInputsMenu()

    for boardName, boardDef in L1MenuFlags.boards().items():
        if "connectors" in boardDef:
           for conn in boardDef["connectors"]:

              # Add more multiplicity inputs

              # Topo1Opt3
              if conn["name"] == "Topo1Opt3":
                  conn["thresholds"] += [
                       ('jXEPerf100',1),
                  ]              

              # Add more decision algorithms
              if conn["name"] == "Topo2El":
                  for group in conn["algorithmGroups"]:
                      if group["fpga"]==0 and group["clock"]==1:
                         group["algorithms"] += [
                             TopoMenuDef( '5DETA99-5DPHI99-2MU3VFab',  outputbits = 10),
                             TopoMenuDef( '0DR04-MU5VFab-CjJ90ab',     outputbits = 11), #Bjet, TODO: not a primary
                             TopoMenuDef( '2DISAMB-jJ40ab-0DR10-eTAU20ab-eTAU12ab',    outputbits = 12), 
                             TopoMenuDef( '2DISAMB-jJ55ab-4DR28-eTAU30abm-eTAU20abm',  outputbits = 13),
                             TopoMenuDef( '2DISAMB-jJ55ab-4DR32-eTAU30abm-eTAU20abm',  outputbits = 14),
                             TopoMenuDef( '2DISAMB-jJ55ab-10DR32-eTAU30abm-eTAU20abm', outputbits = 15),
                             TopoMenuDef( '0DETA24-4DPHI99-eTAU30abm-eTAU20abm',       outputbits = 16),
                             TopoMenuDef( '0DETA24-10DPHI99-eTAU30abm-eTAU12abm',      outputbits = 17),
                         ]
   

    #----------------------------------------------

    def remapThresholds():
        # remap thresholds. TODO: add checks in case the remap does not fulfill HW constraints?
        for boardName, boardDef in L1MenuFlags.boards().items():
            if "connectors" in boardDef:
                for conn in boardDef["connectors"]:
                    if "thresholds" in conn:
                        thresholdsToRemove = []
                        for thrIndex, thrName in enumerate(conn["thresholds"]):
                            nBits = 0
                            if type(thrName)==tuple:
                                (thrName,nBits) = thrName
                            if thrName in L1MenuFlags.ThresholdMap():
                                if (L1MenuFlags.ThresholdMap()[thrName] != ''):
                                    if nBits > 0:
                                        conn["thresholds"][thrIndex] = (L1MenuFlags.ThresholdMap()[thrName],nBits)
                                    else:
                                        conn["thresholds"][thrIndex] = L1MenuFlags.ThresholdMap()[thrName]
                                else:
                                    thresholdsToRemove.append(thrIndex) 
                        for i in reversed(thresholdsToRemove):
                            del conn["thresholds"][i]

    #----------------------------------------------

    remapThresholds()

