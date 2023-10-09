# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import CfgMgr
# The earliest bunch crossing time for which interactions will be sent
# to the TRT Digitization code.
def TRT_FirstXing():
    return -50

# The latest bunch crossing time for which interactions will be sent
# to the TRT Digitization code.
def TRT_LastXing():
    return 50

def getTRTRange(name="TRTRange", **kwargs):
    #this is the time of the xing in ns
    kwargs.setdefault('FirstXing', TRT_FirstXing() )
    kwargs.setdefault('LastXing', TRT_LastXing() )
    kwargs.setdefault('CacheRefreshFrequency', 1.0 ) #default 0 no dataproxy reset
    kwargs.setdefault('ItemList', ["TRTUncompressedHitCollection#TRTUncompressedHits"] )
    from AthenaCommon import CfgMgr
    return CfgMgr.PileUpXingFolder(name, **kwargs)

def BasicTRTDigitizationTool(name, **kwargs):

    from AthenaCommon.Resilience import protectedInclude
    protectedInclude ("TRT_Digitization/TRT_Digitization_CommonOptions.py")
    kwargs.setdefault("PAI_Tool_Xe", "TRT_PAI_Process_Xe")
    kwargs.setdefault("PAI_Tool_Ar", "TRT_PAI_Process_Ar")
    kwargs.setdefault("PAI_Tool_Kr", "TRT_PAI_Process_Kr")

    from Digitization.DigitizationFlags import digitizationFlags
    #flag from Simulation/Digitization
    if not digitizationFlags.doInDetNoise.get_Value():
      kwargs.setdefault("Override_noiseInSimhits", 0)
      kwargs.setdefault("Override_noiseInUnhitStraws", 0)
    #kwargs.setdefault("Override_useMagneticFieldMap=0)
    #TRT xenon range cut
    trtRangeCut=0.05
    if hasattr(digitizationFlags, 'TRTRangeCut'):
      trtRangeCut = digitizationFlags.TRTRangeCut.get_Value()
    kwargs.setdefault("Override_TrtRangeCutProperty", trtRangeCut)

    # Import Beam job properties
    from AthenaCommon.BeamFlags import jobproperties
    if jobproperties.Beam.beamType == "cosmics" :
       kwargs.setdefault("PrintDigSettings", True)
       kwargs.setdefault("Override_cosmicFlag", 0)
       kwargs.setdefault("Override_doCosmicTimingPit", 1)
       kwargs.setdefault("Override_jitterTimeOffset", 0.)
       kwargs.setdefault("Override_timeCorrection", 0)

    if digitizationFlags.doXingByXingPileUp():
        kwargs.setdefault("FirstXing", TRT_FirstXing())
        kwargs.setdefault("LastXing",  TRT_LastXing())
    from AthenaCommon.DetFlags import DetFlags
    if not DetFlags.pileup.any_on():
        kwargs.setdefault("MergeSvc", '')
        kwargs.setdefault("OnlyUseContainerName", False)

    ##else:
    ##    from AthenaCommon.DetFlags import DetFlags
    ##    from AthenaCommon.AppMgr import ServiceMgr
    ##    from PileUpComps.PileUpCompsConf import PileUpXingFolder
    ##    if DetFlags.pileup.TRT_on():
    ##        TRTRange = PileUpXingFolder( "TRTRange" )
    ##        TRTRange.ItemList += ["TRTUncompressedHitCollection#TRTUncompressedHits"]
    ##        #this is the time of the xing in ns
    ##        TRTRange.FirstXing = TRT_FirstXing()
    ##        TRTRange.LastXing  = TRT_LastXing()
    ##        TRTRange.CacheRefreshFrequency = 1.0; #default 0 no dataproxy reset
    ##        # add TRTRange to known pileuo intervals
    ##        ServiceMgr.PileUpMergeSvc.Intervals += [TRTRange]

    kwargs.setdefault("RandomSeedOffset", digitizationFlags.rndmSeedOffset1.get_Value()+digitizationFlags.rndmSeedOffset2.get_Value())
    return CfgMgr.TRTDigitizationTool(name,**kwargs)

def TRTDigitizationTool(name="TRTDigitizationTool",**kwargs):
    from Digitization.DigitizationFlags import digitizationFlags
    if digitizationFlags.PileUpPresampling and 'LegacyOverlay' not in digitizationFlags.experimentalDigi():
        from OverlayCommonAlgs.OverlayFlags import overlayFlags
        kwargs.setdefault("OutputObjectName", overlayFlags.bkgPrefix() + "TRT_RDOs")
        kwargs.setdefault("OutputSDOName", overlayFlags.bkgPrefix() + "TRT_SDO_Map")
    else:
        kwargs.setdefault("OutputObjectName", "TRT_RDOs")
        kwargs.setdefault("OutputSDOName", "TRT_SDO_Map")
    kwargs.setdefault("HardScatterSplittingMode", 0)
    return BasicTRTDigitizationTool(name,**kwargs)

def TRTGeantinoTruthDigitizationTool(name="TRTGeantinoTruthDigitizationTool",**kwargs):
    kwargs.setdefault("VetoPileUpTruthLinks", False)
    return TRTDigitizationTool(name,**kwargs)

def TRTDigitizationToolHS(name="TRTDigitizationToolHS",**kwargs):
    kwargs.setdefault("OutputObjectName", "TRT_RDOs")
    kwargs.setdefault("OutputSDOName", "TRT_SDO_Map")
    kwargs.setdefault("HardScatterSplittingMode", 1)
    return BasicTRTDigitizationTool(name,**kwargs)

def TRTDigitizationToolPU(name="TRTDigitizationToolPU",**kwargs):
    kwargs.setdefault("OutputObjectName", "TRT_PU_RDOs")
    kwargs.setdefault("OutputSDOName", "TRT_PU_SDO_Map")
    kwargs.setdefault("HardScatterSplittingMode", 2)
    return BasicTRTDigitizationTool(name,**kwargs)

def TRTDigitizationToolSplitNoMergePU(name="TRTDigitizationToolSplitNoMergePU",**kwargs):
    kwargs.setdefault("HardScatterSplittingMode", 0)
    kwargs.setdefault("DataObjectName", "PileupTRTUncompressedHits")
    kwargs.setdefault("OutputObjectName", "TRT_PU_RDOs")
    kwargs.setdefault("OutputSDOName", "TRT_PU_SDO_Map")
    kwargs.setdefault("Override_noiseInSimhits", 0)
    kwargs.setdefault("Override_noiseInUnhitStraws", 0)
    return BasicTRTDigitizationTool(name,**kwargs)

def TRTDigitizationHS(name="TRTDigitizationHS",**kwargs):
    kwargs.setdefault("DigitizationTool", "TRTDigitizationToolHS")
    return CfgMgr.TRTDigitization(name,**kwargs)

def TRTDigitizationPU(name="TRTDigitizationPU",**kwargs):
    kwargs.setdefault("DigitizationTool", "TRTDigitizationToolPU")
    return CfgMgr.TRTDigitization(name,**kwargs)

def TRT_OverlayDigitizationTool(name="TRT_OverlayDigitizationTool",**kwargs):
    from OverlayCommonAlgs.OverlayFlags import overlayFlags
    if overlayFlags.isOverlayMT():
        kwargs.setdefault("OnlyUseContainerName", False)
        kwargs.setdefault("OutputObjectName", overlayFlags.sigPrefix() + "TRT_RDOs")
        kwargs.setdefault("OutputSDOName", overlayFlags.sigPrefix() + "TRT_SDO_Map")
    else:
        kwargs.setdefault("OutputObjectName", overlayFlags.evtStore()+"+TRT_RDOs")
        kwargs.setdefault("OutputSDOName", overlayFlags.evtStore()+ "+TRT_SDO_Map")
    kwargs.setdefault("HardScatterSplittingMode", 0)
    kwargs.setdefault("Override_getT0FromData", 0)
    kwargs.setdefault("Override_noiseInSimhits", 0)
    kwargs.setdefault("Override_noiseInUnhitStraws", 0)
    kwargs.setdefault("Override_isOverlay", 1)
    if overlayFlags.isDataOverlay():
        kwargs.setdefault("DigVersContainerKey", "/TRT/Cond/DigVers")
    return BasicTRTDigitizationTool(name,**kwargs)

def TRT_OverlayDigitization(name="TRT_OverlayDigitization",**kwargs):
    kwargs.setdefault("DigitizationTool", "TRT_OverlayDigitizationTool")
    # Multi-threading settinggs
    from AthenaCommon.ConcurrencyFlags import jobproperties as concurrencyProps
    is_hive = (concurrencyProps.ConcurrencyFlags.NumThreads() > 0)
    if is_hive:
        kwargs.setdefault('Cardinality', concurrencyProps.ConcurrencyFlags.NumThreads())
        # Set common overlay extra inputs
        kwargs.setdefault("ExtraInputs", [("McEventCollection", "TruthEvent")])

    return CfgMgr.TRTDigitization(name,**kwargs)
