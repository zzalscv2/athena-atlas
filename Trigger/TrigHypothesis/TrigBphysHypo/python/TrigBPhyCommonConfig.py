# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def TrigBPHY_TrkVKalVrtFitterCfg(flags, suffix, **kwargs):
    kwargs.setdefault("MakeExtendedVertex", False) #different from offline
    from TrkConfig.TrkVKalVrtFitterConfig import BPHY_TrkVKalVrtFitterCfg
    return BPHY_TrkVKalVrtFitterCfg(flags, name='TrigBphysFitter_'+suffix, **kwargs)

