# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from TrigmuRoI.TrigmuRoIMonitoring import TrigmuRoIMonitoring
from TrigMuonRoITools.TrigMuonRoIToolsConfig import TrigMuonRoIToolCfg

def TrigmuRoIConfig(flags, name="TrigmuRoI"):
    alg = CompFactory.TrigmuRoI(name,
                                MonTool = TrigmuRoIMonitoring(flags),
                                RoITool = TrigMuonRoIToolCfg())
    return alg
