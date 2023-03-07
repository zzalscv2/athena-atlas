# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetSecVtxTrackSelectionTool package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


################################
#####  Configs for SecVtx  #####
################################

def InDetSecVtxTrackSelectionToolCfg(flags, name='InDetSecVtxTrackSelectionTool', **kwargs):

  acc = ComponentAccumulator()
  
  kwargs.setdefault("minNPixelHitsAtZeroTRT", 2)
  kwargs.setdefault("minTotalHits", 0)
  kwargs.setdefault("minD0", 0.1)

  acc.setPrivateTools(CompFactory.InDet.InDetSecVtxTrackSelectionTool(name, **kwargs))
  return acc
