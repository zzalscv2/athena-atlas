# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AthConfigFlags import AthConfigFlags

from AthenaCommon.Logging import logging
log = logging.getLogger('TrigVrtSecIclusiveHypoTool')

from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool

def createTrigVSIHypoAlgCfg(flags: AthConfigFlags, name: str, **kwargs) ->CompFactory:

    # Setup the hypothesis algorithm
    theHypoAlg = CompFactory.TrigVSIHypoAlg(name, **kwargs)

    # monitoring
    monTool = GenericMonitoringTool(flags, "IM_MonTool"+name)
    #
    monTool.defineHistogram("nVtx",         type='TH1F', path='EXPERT', title="Nr of TrigVSI vertices;N TrigVSI vertices size;Nevents", xbins=50, xmin=0, xmax=500)
    monTool.defineHistogram("preselNVtx",   type='TH1F', path='EXPERT', title="Nr of vertices passed preselection on hypo;N vertices;Nevents", xbins=50, xmin=0, xmax=500)
    monTool.defineHistogram("maxVtxNTrk",   type='TH1F', path='EXPERT', title="Max Ntracks of a vertex in each event;Max Ntracks;Nevents", xbins=15, xmin=0, xmax=15)
    monTool.defineHistogram("maxVtxMass",   type='TH1F', path='EXPERT', title="Max vertex mass in each event;Max mass;Nevents", xbins=50, xmin=0, xmax=100000.)
    #
    monTool.HistPath = 'TrigVSIHypoAlg'
    theHypoAlg.MonTool = monTool

    return theHypoAlg


def TrigVSIHypoToolFromDict( chainDict ):
    """ Use menu decoded chain dictionary to configure the tool """

    name = chainDict['chainName']
    from AthenaConfiguration.ComponentFactory import CompFactory
    tool = CompFactory.TrigVSIHypoTool(name)

    return tool
