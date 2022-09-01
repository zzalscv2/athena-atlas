#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

'''@file JetTagDQAConfig.py
@author T. Strebler
@date 2022-06-16
@brief Main CA-based python configuration for JetTagDQA
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod

def PhysValBTagCfg(flags, **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("DetailLevel", 10)
    kwargs.setdefault("isData", not flags.Input.isMC)

    import ROOT
    path = ROOT.PathResolver.find_file( 'JetTagDQA/PhysValBtag_VariablesMenu.json', 'DATAPATH' )
    from PhysValMonitoring.PhysValUtils import getHistogramDefinitions
    kwargs.setdefault("HistogramDefinitions", getHistogramDefinitions(path, 'PHYSVAL', 'ALL'))
    kwargs.setdefault("JetEtaCut", 2.5 if flags.GeoModel.Run <= LHCPeriod.Run3 else 4.0)
    kwargs.setdefault("hasJetFitterNN", flags.BTagging.RunJetFitterNN)

    tool = CompFactory.JetTagDQA.PhysValBTag(**kwargs)
    acc.setPrivateTools(tool)
    return acc
