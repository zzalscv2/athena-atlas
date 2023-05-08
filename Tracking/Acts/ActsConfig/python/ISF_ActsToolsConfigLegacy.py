# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import CfgMgr

def getActsFatrasSimTool(name="ISF_ActsFatrasSimTool", **kwargs):
    kwargs.setdefault('MaxSteps'          , 2000           )
    return CfgMgr.ISF__ActsFatrasSimTool(name, **kwargs)
