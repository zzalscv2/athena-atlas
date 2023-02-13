# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
import TrigLongLivedParticlesHypo.TrigLongLivedParticlesHypoMonitoring as HypoMon

def TrigLongLivedParticlesHypoToolFromDict(flags, chainDict):
    """
    Generates the Hypo Tool from the chain dictionary
    """
    name = chainDict['chainName']

    ## Initialize default-configured HypoTool as 'tool'
    from TrigLongLivedParticlesHypo.TrigLongLivedParticlesHypoConfig import MuonClusterHypoToolConfig
    return MuonClusterHypoToolConfig(flags, name)


def MuonClusterHypoAlgConfig(flags, name="MuonClusterHypoAlgConfig"):
    """Monitoring Tool Configuration for HypoAlg"""
    return CompFactory.MuonClusterHypoAlg(name,
        MonTool = HypoMon.trigMuonClusterHypoAlgMonitoring(flags))


def MuonClusterHypoToolConfig(flags, name="MuonClusterHypoToolConfig"):
    """Monitoring Tool Configuration for HypoTool"""
    return CompFactory.MuonClusterHypoTool(
        MonTool = HypoMon.trigMuonClusterHypoToolMonitoring(flags),
        acceptAll = False)
