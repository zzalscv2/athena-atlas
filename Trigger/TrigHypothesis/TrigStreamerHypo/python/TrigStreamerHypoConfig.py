# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory

def StreamerHypoToolGenerator(chainDict):
    """ Configure streamer tool from chainDict """
    return CompFactory.TrigStreamerHypoTool( chainDict['chainName'] )
