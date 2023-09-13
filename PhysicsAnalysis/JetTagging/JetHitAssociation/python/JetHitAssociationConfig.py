# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# For using GeV units
import AthenaCommon.SystemOfUnits as Units

# Needed to get name of split probability container
from InDetConfig.TrackRecoConfig import ClusterSplitProbabilityContainerName

# We need to add this algorithm to get the TrackMeasurementValidationContainer
from InDetConfig.InDetPrepRawDataToxAODConfig import InDetPixelPrepDataToxAODCfg, InDetSCT_PrepDataToxAODCfg

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def JetHitAssociationCfg(flags, name="JetHitAssociation", **kwargs):

    acc = ComponentAccumulator()
    
    isMC = flags.Input.isMC
    SiHitsRequested = any("SiHitCollection" in collection for collection in flags.Input.TypedCollections)
           
    acc.merge(
        InDetPixelPrepDataToxAODCfg(
            flags,
            ClusterSplitProbabilityName=ClusterSplitProbabilityContainerName(flags),
            WriteSiHits=(isMC and SiHitsRequested),
            WriteSDOs=(isMC and SiHitsRequested),       
            # see ATR-27293 for discussion on why this was disabled
            WriteNNinformation=False
        )
    )

    acc.merge(
        InDetSCT_PrepDataToxAODCfg(
            flags,
            WriteSiHits=(isMC and SiHitsRequested),
            WriteSDOs=(isMC and SiHitsRequested)	     
        )
    )

    acc.addEventAlgo(
        CompFactory.JetHitAssociation(
            "JetHitAssociation",
            jetCollectionName = flags.BTagging.Trackless_JetCollection,
            jetPtThreshold = flags.BTagging.Trackless_JetPtMin * Units.GeV,
            dRmatchHitToJet = flags.BTagging.Trackless_dR
        )
    )

    return acc

