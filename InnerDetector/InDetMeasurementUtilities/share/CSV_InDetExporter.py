#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from ROOT import xAOD, TFile, SG
import os
import csv
from argparse import ArgumentParser

class CSVDumper:

    def __init__(self, inputAOD, outputDir, dict_variables_types, treename="CollectionTree", nEvents=-1, ContainerName="PixelHits"):

        xAOD.Init()    # Setting up ROOT tools
        self.tree            = xAOD.MakeTransientTree( TFile(inputAOD), treename )
        self.n_entries       = self.tree.GetEntriesFast()
        self.outputDir       = outputDir
        self.ContainerName   = ContainerName

        if nEvents < 0 or nEvents > self.n_entries:   # Getting number of events to run
            self.nEvents = self.n_entries
        else:
            self.nEvents = nEvents
        print(f"Running on {self.nEvents} events")

        os.system("mkdir -p "+self.outputDir)   # Creating directory to store files

        # Variables with their types that should be stored in csv format. Should be something like: 
        #{     
        #    "col": "int",
        #    "detid": "unsigned long",
        #}
        self.dict_variables_types = dict_variables_types
        
        # Creating accessors to, well, access the info in the xAOD
        self.dict_accessors = self.CreateAccessors(self.dict_variables_types)

    def CreateAccessors(self, dict_var_type):

        dict_accessors = {}
        for var, fmt in dict_var_type.items():
            dict_accessors[ var ] = SG.AuxElement.ConstAccessor[fmt]( var )
        
        return dict_accessors

    def WriteCSV(self, filename, dictionary):
    
        with open(filename, "w") as out:
            writer = csv.writer(out)
            writer.writerow(dictionary.keys())
            writer.writerows( zip(*dictionary.values()) )
        print(f"New file saved: {filename}")
        
    
    def ProcessEvent(self, evt):

        self.tree.GetEntry(evt)
        EventNumber = self.tree.EventInfo.mcEventNumber()

        dict_lists = {}
        for variable in self.dict_variables_types.keys():
            tp  = getattr(self.tree, self.ContainerName)
            vec = self.dict_accessors[variable].getDataArray(tp)
            vec.reshape((len(tp),))

            dict_lists[variable] = list(vec)
        
        self.WriteCSV( filename=f"{self.outputDir}/{self.ContainerName}_event_{EventNumber}.csv", dictionary=dict_lists )


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
        raise Exception("No filename was provided!")
    
    if args.outputDir == "":
        raise Exception("No outputDir was provided!")
    
    ContainerName = "PixelHits"
    dict_variables_types = {     
            "col": "int",
            "row": "int",
            "tot": "int",
            "eta_module": "int",
            "phi_module": "int",
            "layer_disk": "int",
            "barrel_ec": "int",
            "detid": "unsigned long",
    }
    # Information about hits<->detector can be obtained from "detid" using ROOT function PixelID
    # https://acode-browser1.usatlas.bnl.gov/lxr/source/athena/InnerDetector/InDetDetDescr/InDetIdentifier/InDetIdentifier/PixelID.h

    Dumper = CSVDumper(inputAOD=args.inputAOD,
                       outputDir=args.outputDir,
                       dict_variables_types=dict_variables_types,
                       treename=args.treename,
                       nEvents=args.nEvents,
                       ContainerName=ContainerName)
    Dumper.Run()
