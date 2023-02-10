# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory

def TrigBphysStreamerHypoToolFromDict(flags, chainDict):
    tool = CompFactory.TrigBphysStreamerHypoTool(chainDict['chainName'])
    return tool
