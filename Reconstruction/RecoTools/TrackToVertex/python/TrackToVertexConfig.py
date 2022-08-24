# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of TrackToVertex package
from AthenaConfiguration.ComponentFactory import CompFactory

def TrackToVertexCfg(flags, name="AtlasTrackToVertexTool", **kwargs):
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    result = BeamSpotCondAlgCfg(flags) # To produce InDet::BeamSpotData CondHandle
    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", result.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))
    result.setPrivateTools(CompFactory.Reco.TrackToVertex(name, **kwargs))
    return result

