# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags


def createHIRecConfigFlags():
  flags=AthConfigFlags()
  
  flags.addFlag("HeavyIon.doGlobal", True)
  flags.addFlag("HeavyIon.Global.doEventShapeSummary", True)
  flags.addFlag("HeavyIon.Global.EventShape", "HIEventShape")

  flags.addFlag("HeavyIon.doJet", True)
  flags.addFlag("HeavyIon.Jet.doTrackJetSeed", True)
  flags.addFlag("HeavyIon.Jet.ApplyTowerEtaPhiCorrection", True)
  flags.addFlag("HeavyIon.Jet.HarmonicsForSubtraction", [2, 3, 4])
  flags.addFlag("HeavyIon.Jet.SeedPtMin", 25000)
  flags.addFlag("HeavyIon.Jet.RecoOutputPtMin", 25000)
  flags.addFlag("HeavyIon.Jet.TrackJetPtMin", 7000)
  flags.addFlag("HeavyIon.Jet.HIClusterGeoWeightFile", "auto")
  flags.addFlag("HeavyIon.Jet.ClusterKey", "HIClusters")
  flags.addFlag("HeavyIon.Jet.Internal.ClusterKey", "HIClusters_temp")
  flags.addFlag("HeavyIon.Jet.WriteHIClusters", True)

  flags.addFlag("HeavyIon.Egamma.doSubtractedClusters", lambda prevFlags: prevFlags.Reco.EnableHI)
  flags.addFlag("HeavyIon.Egamma.EventShape", "HIEventShape_iter_egamma")
  flags.addFlag("HeavyIon.Egamma.SubtractedCells", "SubtractedCells")
  flags.addFlag("HeavyIon.Egamma.UncalibCaloTopoCluster", "SubtractedCaloTopoClusters")
  flags.addFlag("HeavyIon.Egamma.EgammaTopoCluster", "SubtractedEgammaTopoClusters")
  flags.addFlag("HeavyIon.Egamma.CaloTopoCluster", lambda prevFlags: "SubtractedCaloCalTopoClusters" if prevFlags.Calo.TopoCluster.doTopoClusterLocalCalib else prevFlags.HeavyIon.Egamma.UncalibCaloTopoCluster)

  flags.addFlag("HeavyIon.redoTracking", True)
  flags.addFlag("HeavyIon.redoEgamma", True)

  # expand as needed
  return flags
