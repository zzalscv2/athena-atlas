# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import HIMode

def createHIRecConfigFlags():
  flags=AthConfigFlags()
  
  flags.addFlag("HeavyIon.doGlobal", True)
  flags.addFlag("HeavyIon.Global.doEventShapeSummary", True)
  flags.addFlag("HeavyIon.Global.EventShape", "HIEventShape")

  flags.addFlag("HeavyIon.doJet", True)
  flags.addFlag("HeavyIon.Jet.doTrackJetSeed", True)
  flags.addFlag("HeavyIon.Jet.ApplyTowerEtaPhiCorrection", lambda prevFlags: prevFlags.Reco.HIMode is HIMode.HI)
  flags.addFlag("HeavyIon.Jet.HarmonicsForSubtraction", lambda prevFlags: [2, 3, 4] if prevFlags.Reco.HIMode is HIMode.HI else [])
  flags.addFlag("HeavyIon.Jet.SeedPtMin", lambda prevFlags: 25000 if prevFlags.Reco.HIMode is HIMode.HI else 8000)
  flags.addFlag("HeavyIon.Jet.RecoOutputPtMin", lambda prevFlags: 25000 if prevFlags.Reco.HIMode is HIMode.HI else 8000)
  flags.addFlag("HeavyIon.Jet.TrackJetPtMin", lambda prevFlags: 7000 if prevFlags.Reco.HIMode is HIMode.HI else 4000)
  flags.addFlag("HeavyIon.Jet.HIClusterGeoWeightFile", "auto")
  flags.addFlag("HeavyIon.Jet.ClusterKey", "HIClusters")
  flags.addFlag("HeavyIon.Jet.Internal.ClusterKey", "HIClusters_temp")
  flags.addFlag("HeavyIon.Jet.WriteHIClusters", lambda prevFlags: prevFlags.Reco.HIMode is not HIMode.UPC)
  flags.addFlag("HeavyIon.Jet.RValues", [2,4])#this are the R's we want to reconstruct
  flags.addFlag("HeavyIon.Jet.CaliRValues", ["2","3","4","10"])#this are the R's that are supported for calibration, if not listed then cali R=0.4 is picked

  flags.addFlag("HeavyIon.Egamma.doSubtractedClusters", lambda prevFlags: prevFlags.Reco.HIMode is HIMode.HI)
  flags.addFlag("HeavyIon.Egamma.EventShape", "HIEventShape_iter_egamma")
  flags.addFlag("HeavyIon.Egamma.SubtractedCells", "SubtractedCells")
  flags.addFlag("HeavyIon.Egamma.UncalibCaloTopoCluster", "SubtractedCaloTopoClusters")
  flags.addFlag("HeavyIon.Egamma.EgammaTopoCluster", "SubtractedEgammaTopoClusters")
  flags.addFlag("HeavyIon.Egamma.CaloTopoCluster", lambda prevFlags: "SubtractedCaloCalTopoClusters" if prevFlags.Calo.TopoCluster.doTopoClusterLocalCalib else prevFlags.HeavyIon.Egamma.UncalibCaloTopoCluster)

  flags.addFlag("HeavyIon.redoTracking", True)
  flags.addFlag("HeavyIon.redoEgamma", True)

  # expand as needed
  return flags
