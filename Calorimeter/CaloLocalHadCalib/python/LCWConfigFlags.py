# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def addLCWFlags(flags):

   flags.addFlag("LCW.doClassification",False)
   flags.addFlag("LCW.doWeighting",False)
   flags.addFlag("LCW.doOutOfCluster",False)
   flags.addFlag("LCW.doDeadMaterial",False)

   flags.addFlag("LCW.outFileNameLCC","classify.root")
   flags.addFlag("LCW.outFileNameLCW","weights.root")
   flags.addFlag("LCW.outFileNameLCO","ooc.root")
   flags.addFlag("LCW.outFileNameLCDM","dmc.root")
