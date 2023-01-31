# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Author: Katharina Voss katharina.voss@cern.ch

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

#------------------------------------
def ClassifiedTrackTaggerToolCfg(flags,name="ClassifiedTrackTaggerTool", **kwargs):
 
    acc = ComponentAccumulator()
    
    kwargs.setdefault('useFivePtJetBinTCT',flags.BTagging.TrkClassFiveBinMode)
    kwargs.setdefault('JetCollection',"AntiKt4EMPFlowJets")

    from TrkConfig.TrkVKalVrtFitterConfig import TrkVKalVrtFitterCfg
    VertexFitterTool = acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags,"VertexFitterTool"))
    from InDetConfig.InDetVKalVxInJetToolConfig import InDetTrkInJetTypeCfg
    trackClassificationTool = acc.popToolsAndMerge(InDetTrkInJetTypeCfg(flags,name='TrkInJetType',JetCollection=kwargs["JetCollection"],VertexFitterTool=VertexFitterTool))
    kwargs.setdefault('TrackClassificationTool',trackClassificationTool)

    ClassifiedTrackTagger = CompFactory.Analysis.ClassifiedTrackTaggerTool(name,**kwargs)
    acc.setPrivateTools(ClassifiedTrackTagger)

    return acc






