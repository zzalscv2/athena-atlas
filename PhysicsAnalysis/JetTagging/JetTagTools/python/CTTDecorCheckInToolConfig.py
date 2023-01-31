# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Author: Katharina Voss katharina.voss@cern.ch

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import INFO


#------------------------------------
def CTTDecorCheckInToolCfg(flags,name="CTTDecorCheckInTool", **kwargs):
    acc = ComponentAccumulator()
    
    kwargs.setdefault("JetCollection","AntiKt4EMPFlowJets")
    from JetTagTools.ClassifiedTrackTaggerToolConfig import ClassifiedTrackTaggerToolCfg
    kwargs.setdefault("ClassifiedTrackTaggerTool",acc.popToolsAndMerge(ClassifiedTrackTaggerToolCfg(flags,name="ClassifiedTrackTaggerTool",JetCollection=kwargs["JetCollection"])))
    kwargs.setdefault("OutputLevel", INFO)                     

    acc.addEventAlgo(CompFactory.CTTDecorCheckInTool(name, **kwargs))
    return acc






