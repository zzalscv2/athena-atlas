# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonDetailedTrackTruthMakerCfg(flags, name="MuonDetailedTrackTruthMaker", **kwargs):
    result = ComponentAccumulator()
    
    PRD_TruthNames = ["RPC_TruthMap", "TGC_TruthMap", "MDT_TruthMap"]
    if flags.Detector.EnableCSC:
        PRD_TruthNames += ["CSC_TruthMap"]
    if flags.Detector.EnableMM:
        PRD_TruthNames += ["MM_TruthMap"]
    if flags.Detector.EnablesTGC:
        PRD_TruthNames += ["STGC_TruthMap"]

    kwargs.setdefault("PRD_TruthNames", PRD_TruthNames)
    result.addEventAlgo(CompFactory.MuonDetailedTrackTruthMaker(name, **kwargs))
    return result

def MuonTruthDecorationAlgCfg(flags, name="MuonTruthDecorationAlg", **kwargs):
    result = ComponentAccumulator()

    PRD_TruthMaps = ["RPC_TruthMap","TGC_TruthMap","MDT_TruthMap"]
    SDOs = ["RPC_SDO","TGC_SDO","MDT_SDO"]
    CSCSDOs = "CSC_SDO"

    if flags.Detector.EnablesTGC and flags.Detector.EnableMM:
        SDOs += ["MM_SDO","sTGC_SDO"]
        PRD_TruthMaps += ["MM_TruthMap", "STGC_TruthMap"]
    if not flags.Detector.EnableCSC: 
        CSCSDOs = ""
    else:
        PRD_TruthMaps += ["CSC_TruthMap"]
    
    kwargs.setdefault("SDOs", SDOs)
    kwargs.setdefault("CSCSDOs", CSCSDOs)
    kwargs.setdefault("PRD_TruthMaps", PRD_TruthMaps)

    if "MCTruthClassifier" not in kwargs:
        from MCTruthClassifier.MCTruthClassifierConfig import MCTruthClassifierCfg
        kwargs.setdefault("MCTruthClassifier", result.popToolsAndMerge(
            MCTruthClassifierCfg(flags)))

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", result.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))
        
    result.addEventAlgo(CompFactory.Muon.MuonTruthDecorationAlg(name, **kwargs))
    return result

def MuonTruthAssociationAlgCfg(flags, name="MuonTruthAssociationAlg", **kwargs):
    result = ComponentAccumulator()
    result.addEventAlgo(CompFactory.MuonTruthAssociationAlg(name, **kwargs))
    return result
