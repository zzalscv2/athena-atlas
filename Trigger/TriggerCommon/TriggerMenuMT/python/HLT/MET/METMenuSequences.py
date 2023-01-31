#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from .ConfigHelpers import AlgConfig

def metMenuSequence(flags, **recoDict):
    conf = AlgConfig.fromRecoDict(flags, **recoDict)
    return conf.menuSequence
