# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import CfgMgr

def getLArHitFilter(name="LArHitFilter" , **kwargs):
    return CfgMgr.LArHitFilter(name, **kwargs)
