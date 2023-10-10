# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def SetupSensitiveDetectorsCfg(flags):
    result = ComponentAccumulator()
    tools = []

    from AthenaConfiguration.Enums import BeamType
    if flags.Beam.Type is BeamType.Cosmics:
        if flags.Detector.EnableMDT:
            from MuonG4SD.MuonG4SDToolConfig import MDTSensitiveDetectorCosmicsCfg
            tools += [ result.popToolsAndMerge(MDTSensitiveDetectorCosmicsCfg(flags)) ]
        if flags.Detector.EnableRPC:
            from MuonG4SD.MuonG4SDToolConfig import RPCSensitiveDetectorCosmicsCfg
            tools += [ result.popToolsAndMerge(RPCSensitiveDetectorCosmicsCfg(flags)) ]
        if flags.Detector.EnableTGC:
            from MuonG4SD.MuonG4SDToolConfig import TGCSensitiveDetectorCosmicsCfg
            tools += [ result.popToolsAndMerge(TGCSensitiveDetectorCosmicsCfg(flags)) ]
        if flags.Detector.EnableCSC:
            from MuonG4SD.MuonG4SDToolConfig import CSCSensitiveDetectorCosmicsCfg
            tools += [ result.popToolsAndMerge(CSCSensitiveDetectorCosmicsCfg(flags)) ]
    else:
        if flags.Detector.EnableMDT:
            from MuonG4SD.MuonG4SDToolConfig import MDTSensitiveDetectorCfg
            tools += [ result.popToolsAndMerge(MDTSensitiveDetectorCfg(flags)) ]
        if flags.Detector.EnableRPC:
            from MuonG4SD.MuonG4SDToolConfig import RPCSensitiveDetectorCfg
            tools += [ result.popToolsAndMerge(RPCSensitiveDetectorCfg(flags)) ]
        if flags.Detector.EnableTGC:
            from MuonG4SD.MuonG4SDToolConfig import TGCSensitiveDetectorCfg
            tools += [ result.popToolsAndMerge(TGCSensitiveDetectorCfg(flags)) ]
        if flags.Detector.EnableCSC:
            from MuonG4SD.MuonG4SDToolConfig import CSCSensitiveDetectorCfg
            tools += [ result.popToolsAndMerge(CSCSensitiveDetectorCfg(flags)) ]

    if flags.Detector.EnablesTGC :
        from MuonG4SD.MuonG4SDToolConfig import sTGCSensitiveDetectorCfg
        tools += [ result.popToolsAndMerge(sTGCSensitiveDetectorCfg(flags)) ]
    if flags.Detector.EnableMM :
        from MuonG4SD.MuonG4SDToolConfig import MicromegasSensitiveDetectorCfg
        tools += [ result.popToolsAndMerge(MicromegasSensitiveDetectorCfg(flags)) ]

    result.setPrivateTools(tools)
    return result


def SimHitContainerListCfg(flags):
    simHitContainers = []
    if flags.Detector.EnableMDT:
        simHitContainers+=[("MDTSimHitCollection", "MDT_Hits")]
    if flags.Detector.EnableRPC:
        simHitContainers+=[("RPCSimHitCollection", "RPC_Hits")]
    if flags.Detector.EnableTGC:
        simHitContainers+=[("TGCSimHitCollection", "TGC_Hits")]
    if flags.Detector.EnableMM:
        simHitContainers+=[("MMSimHitCollection", "MM_Hits")]
    if flags.Detector.EnablesTGC:
        simHitContainers+=[("sTGCSimHitCollection", "sTGC_Hits")]
    if flags.Detector.EnableCSC:
        simHitContainers+=[("CSCSimHitCollection", "CSC_Hits")]
    return simHitContainers

