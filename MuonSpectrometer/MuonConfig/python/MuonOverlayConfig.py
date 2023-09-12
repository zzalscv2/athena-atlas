# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
def MuonOverlayCfg(configFlags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()
    from MuonConfig.CSC_OverlayConfig import CSC_OverlayCfg
    from MuonConfig.MDT_OverlayConfig import MDT_OverlayCfg
    from MuonConfig.MM_OverlayConfig import MM_OverlayCfg
    from MuonConfig.RPC_OverlayConfig import RPC_OverlayCfg
    from MuonConfig.sTGC_OverlayConfig import sTGC_OverlayCfg
    from MuonConfig.TGC_OverlayConfig import TGC_OverlayCfg

    if configFlags.Detector.EnableCSC:
        acc.merge(CSC_OverlayCfg(configFlags))
    if configFlags.Detector.EnableMDT:
        acc.merge(MDT_OverlayCfg(configFlags))
    if configFlags.Detector.EnableRPC:
        acc.merge(RPC_OverlayCfg(configFlags))
    if configFlags.Detector.EnableTGC:
        acc.merge(TGC_OverlayCfg(configFlags))
    if configFlags.Detector.EnablesTGC:
        acc.merge(sTGC_OverlayCfg(configFlags))
    if configFlags.Detector.EnableMM:
        acc.merge(MM_OverlayCfg(configFlags))
    return acc

