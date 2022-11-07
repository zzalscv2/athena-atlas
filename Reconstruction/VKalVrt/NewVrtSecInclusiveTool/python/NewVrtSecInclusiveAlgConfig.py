# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Author: Vadim Kostyukhin vadim.kostyukhin@cern.ch

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import INFO

def NewVrtSecInclusiveAlgCfg(flags, algname="NVSI_Alg"):
  
   acc = ComponentAccumulator()
   myargs = {}
   from NewVrtSecInclusiveTool.NewVrtSecInclusiveConfig import SoftBFinderToolCfg
   myargs["BVertexTool"] = acc.popToolsAndMerge(SoftBFinderToolCfg(flags,FillHist=True))
   myargs["OutputLevel"] = INFO
   myargs.setdefault("BVertexContainerName","AllBVertices")

   NVSI_Alg = CompFactory.Rec.NewVrtSecInclusiveAlg(algname, **myargs)
   acc.addEventAlgo(NVSI_Alg)
   return acc

