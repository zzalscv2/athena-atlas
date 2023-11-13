# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def LArConfigRun1PileUp(flags):
   flags.LAr.ROD.UseHighestGainAutoCorr = False  # default in Run1
   flags.LAr.ROD.FirstSample = 0  # default
   flags.LAr.ROD.nSamples = 5     # default in Run-1
   flags.LAr.ROD.NumberOfCollisions = 20  # Run-1 default
   flags.LAr.ROD.UseDelta = 3 # it was used in Run-1

   flags.Digitization.HighGainEMECIW = True
   flags.Digitization.HighGainFCal = False


def LArConfigRun1NoPileUp(flags):
   flags.LAr.ROD.UseHighestGainAutoCorr = False # default in Run1
   flags.LAr.ROD.FirstSample = 0  # default
   flags.LAr.ROD.nSamples = 5  # default in Run1
   flags.LAr.ROD.NumberOfCollisions = 0 #  no pileup

   flags.Digitization.HighGainEMECIW = True # use high gain in EMEC IW in nopileup case
   flags.Digitization.HighGainFCal = False  # use high gain in Fcal in nopileup case
