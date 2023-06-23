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
                try:
                    sp  = asarray(  tp.getConstDataSpan[ fmt ]( var ) )
                except Exception:
                    # This is for the case arrays are either empty or have some trouble when accessed via getConstDataSpan. Used in excepction as makes the code slower
                    sp  = asarray( [ getattr(element, var)() for element in tp ] )
                
                if "ArrayFloat3" in fmt and len(sp) > 0:  # Needs extra coding when dealing with xAOD::ArrayFloat3 instead of standard C++ array
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

    from InDetMeasurementUtilities.CSV_DictFormats import CSV_DictFormats 

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
    
    Dumper = CSVDumper(inputAOD=args.inputAOD,
                       outputDir=args.outputDir,
                       dict_variables_types=CSV_DictFormats,
                       treename=args.treename,
                       nEvents=args.nEvents)
    Dumper.Run()
