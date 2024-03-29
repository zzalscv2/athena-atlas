# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

def LArConfigRun3PileUp(flags):
   flags.LAr.ROD.UseHighestGainAutoCorr = True
   flags.LAr.ROD.FirstSample = 0  # default
   flags.LAr.ROD.nSamples = 4     # default
   flags.LAr.ROD.NumberOfCollisions = 60  # Run-3 default
   
   flags.Digitization.HighGainEMECIW = False
   flags.Digitization.HighGainFCal = False


def LArConfigRun3NoPileUp(flags):
   flags.LAr.ROD.UseHighestGainAutoCorr = False
   flags.LAr.ROD.FirstSample = 0  # default
   flags.LAr.ROD.nSamples = 4     # default
   flags.LAr.ROD.NumberOfCollisions = 0
   
   flags.Digitization.HighGainEMECIW = True
   flags.Digitization.HighGainFCal = True
