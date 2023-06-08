#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from ROOT import xAOD, TFile
import os
import csv
from argparse import ArgumentParser
from numpy import asarray, array

class CSVDumper:

    def __init__(self, inputAOD, outputDir, dict_variables_types, treename="CollectionTree", nEvents=-1):

        xAOD.Init()    # Setting up ROOT tools
        self.tree            = xAOD.MakeTransientTree( TFile(inputAOD), treename )
        self.n_entries       = self.tree.GetEntriesFast()
        self.outputDir       = outputDir

        if nEvents < 0 or nEvents > self.n_entries:   # Getting number of events to run
            self.nEvents = self.n_entries
        else:
            self.nEvents = nEvents
        print(f"Running on {self.nEvents} events")

        os.system("mkdir -p "+self.outputDir)   # Creating directory to store files

        # Container and Variables with their types that should be stored in csv format. Should be something like: 
        #{     
        #    "ContainerName": { "var1": "int",
        #                       "var2": "float",
        #                     }
        #}
        self.dict_variables_types = dict_variables_types
        
    def WriteCSV(self, filename, dictionary):
    
        with open(filename, "w") as out:
            writer = csv.writer(out)
            writer.writerow(dictionary.keys())
            writer.writerows( zip(*dictionary.values()) )
        print(f"New file saved: {filename}")

    def ArrayFloat3_to_CppArray(self, ArrayFloat3): # Check ArrayFloat3 in https://gitlab.cern.ch/atlas/athena/-/blob/master/Event/xAOD/xAODInDetMeasurement/xAODInDetMeasurement/versions/SpacePoint_v1.h
        arr = ArrayFloat3.data()
        arr.reshape((3,))
        return list(arr)
        
    
    def ProcessEvent(self, evt):

        # Getting aux variables based on the implementation of https://gitlab.cern.ch/atlas/athena/-/commit/fc5baf9fd2bb28c56f115fc2107a3ff159f1945d
        print("--> Event "+str(evt))
        self.tree.GetEntry(evt)
        EventNumber = self.tree.EventInfo.mcEventNumber()

        for container in self.dict_variables_types.keys():
            dict_lists = {}
            dict_container = self.dict_variables_types[container]
            try:                    
                tp  = getattr(self.tree, container)
            except Exception:
                print(".. Missing ", container)
                continue

            for var,fmt in dict_container.items():
                sp  = asarray(  tp.getConstDataSpan[ fmt ]( var ) )
                
                if "ArrayFloat3" in fmt:  # Needed extra coding with dealing with xAOD::ArrayFloat3 instead of standard C++ array
                    sp = array( list( map(self.ArrayFloat3_to_CppArray, sp) ) )
                    
                if sp.ndim == 1:
                    dict_lists[var] = sp
                else:
                    # Then we have an array. Each element of the array will be a column in the csv file. [We want things flat]
                    for column in range(sp.shape[1]):
                        dict_lists[var+f"_at{column}"] = sp.T[column] 
                
            self.WriteCSV( filename=f"{self.outputDir}/{container}_event_{EventNumber}.csv", dictionary=dict_lists )


    def Run(self):

        for evt in range(self.nEvents):
            self.ProcessEvent(evt)    


if __name__ == "__main__":

    parser = ArgumentParser()
    parser.add_argument('--inputAOD', type=str, default="")
    parser.add_argument('--outputDir', type=str, default="")
    parser.add_argument('--treename', type=str, default="CollectionTree")
    parser.add_argument('--nEvents', type=int, default=-1)
    args = parser.parse_args()

    if args.inputAOD == "":
        raise Exception("No inputAOD was provided!")
    
    if args.outputDir == "":
        raise Exception("No outputDir was provided!")
    
    dict_variables_types = {
        #All obtained AuxElement content using the HitsToxAODCopier tool
        "PixelHits": { "col": "int", "row": "int", "tot": "int", "eta_module": "int", "phi_module": "int", "layer_disk": "int", "barrel_ec": "int", "detid": "unsigned long"},
        "StripHits": { "strip": "int", "side": "int", "eta_module": "int", "phi_module": "int", "layer_disk": "int", "barrel_ec": "int", "detid": "unsigned long"},
        
        #https://gitlab.cern.ch/atlas/athena/-/blob/master/Event/xAOD/xAODInDetMeasurement/xAODInDetMeasurement/versions/PixelClusterAuxContainer_v1.h
        "ITkPixelClusters": {"globalPosition":"std::array<float,3>", "channelsInPhi":"int", "channelsInEta":"int", "widthInEta":"float", "omegaX":"float", "omegaY":"float", "totalToT":"int", "totalCharge":"float",   "energyLoss":"float", "splitProbability1":"float", "splitProbability2":"float", "lvl1a":"int", "localPosition":"std::array<float,3>", "localCovariance":"std::array<float,9>"},
        # ITkPixelClusters.isSplit is char with all ''? Keeping out for the moment

        #https://gitlab.cern.ch/atlas/athena/-/blob/master/Event/xAOD/xAODInDetMeasurement/xAODInDetMeasurement/versions/StripClusterAuxContainer_v1.h
        "ITkStripClusters": {"globalPosition":"std::array<float,3>", "channelsInPhi":"int", "localPosition":"std::array<float,3>", "localCovariance":"std::array<float,9>"},

        #https://gitlab.cern.ch/atlas/athena/-/blob/master/Event/xAOD/xAODInDetMeasurement/xAODInDetMeasurement/versions/SpacePoint_v1.h
        "ITkStripSpacePoints": {"globalPosition":"std::array<float,3>", "radius":"float", "varianceR":"float", "varianceZ":"float", "topHalfStripLength":"float", "bottomHalfStripLength":"float", "topStripDirection":"xAOD::ArrayFloat3", "bottomStripDirection":"xAOD::ArrayFloat3", "stripCenterDistance":"xAOD::ArrayFloat3", "topStripCenter":"xAOD::ArrayFloat3"},

        #https://gitlab.cern.ch/atlas/athena/-/blob/master/Event/xAOD/xAODInDetMeasurement/xAODInDetMeasurement/versions/SpacePoint_v1.h
        "ITkPixelSpacePoints": {"globalPosition":"std::array<float,3>", "radius":"float", "varianceR":"float", "varianceZ":"float"},

    }

    Dumper = CSVDumper(inputAOD=args.inputAOD,
                       outputDir=args.outputDir,
                       dict_variables_types=dict_variables_types,
                       treename=args.treename,
                       nEvents=args.nEvents)
    Dumper.Run()
