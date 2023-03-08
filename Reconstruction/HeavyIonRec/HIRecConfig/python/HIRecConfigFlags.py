# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags


def createHIRecConfigFlags():
  flags=AthConfigFlags()
  flags.addFlag("HeavyIon.doGlobal", True)
  flags.addFlag("HeavyIon.Global.doEventShapeSummary", True)

  flags.addFlag("HeavyIon.doJet", True)
  flags.addFlag("HeavyIon.Jet.doTrackJetSeed", True)
  flags.addFlag("HeavyIon.Jet.ApplyTowerEtaPhiCorrection", True)
  flags.addFlag("HeavyIon.Jet.HarmonicsForSubtraction", [2, 3, 4])
  flags.addFlag("HeavyIon.Jet.SeedPtMin", 25000)
  flags.addFlag("HeavyIon.Jet.RecoOutputPtMin", 25000)
  flags.addFlag("HeavyIon.Jet.TrackJetPtMin", 7000)

  flags.addFlag("HeavyIon.doEgamma", True)
  # expand as needed
  return flags
