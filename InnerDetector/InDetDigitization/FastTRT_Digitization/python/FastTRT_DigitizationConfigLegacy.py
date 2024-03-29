# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# The earliest bunch crossing time for which interactions will be sent
# to the FastTRT Digitization code.
def FastTRT_FirstXing():
    return -50

# The latest bunch crossing time for which interactions will be sent
# to the FastTRT Digitization code.
def FastTRT_LastXing():
    return 50

def getFastTRTRange(name="FastTRTRange", **kwargs):
    #this is the time of the xing in ns
    kwargs.setdefault('FirstXing', FastTRT_FirstXing() )
    kwargs.setdefault('LastXing', FastTRT_LastXing() )
    kwargs.setdefault('CacheRefreshFrequency', 1.0 ) #default 0 no dataproxy reset
    kwargs.setdefault('ItemList', ["TRTUncompressedHitCollection#TRTUncompressedHits"] )
    from AthenaCommon import CfgMgr
    return CfgMgr.PileUpXingFolder(name, **kwargs)

def BasicTRTFastDigitizationTool(name, **kwargs):

    from IOVDbSvc.CondDB import conddb
    if not conddb.folderRequested('/TRT/Calib/errors'):
        conddb.addFolderSplitOnline("TRT","/TRT/Onl/Calib/errors","/TRT/Calib/errors")
    if not conddb.folderRequested( "/TRT/Calib/PID_vector" ):
        conddb.addFolderSplitOnline("TRT","/TRT/Onl/Calib/PID_vector", "/TRT/Calib/PID_vector")
    if not conddb.folderRequested( "/TRT/Calib/ToT/ToTVectors"):
        conddb.addFolder( "TRT_OFL", "/TRT/Calib/ToT/ToTVectors") 
    if not conddb.folderRequested( "/TRT/Calib/ToT/ToTValue"):
        conddb.addFolder( "TRT_OFL", "/TRT/Calib/ToT/ToTValue")

    kwargs.setdefault("RandomStreamName", "FastTRTDigitization")

    from Digitization.DigitizationFlags import digitizationFlags
    if digitizationFlags.doXingByXingPileUp():
        kwargs.setdefault("FirstXing", FastTRT_FirstXing())
        kwargs.setdefault("LastXing",  FastTRT_LastXing())

    from AthenaCommon import CfgMgr
    return CfgMgr.TRTFastDigitizationTool(name,**kwargs)

def TRTFastDigitizationTool(name="TRTFastDigitizationTool",**kwargs):
    kwargs.setdefault("trtDriftCircleContainer", "TRT_DriftCircles")
    kwargs.setdefault("trtPrdMultiTruthCollection", "PRD_MultiTruthTRT")
    kwargs.setdefault("HardScatterSplittingMode", 0)
    return BasicTRTFastDigitizationTool(name,**kwargs)

def TRTFastDigitizationToolHS(name="TRTFastDigitizationToolHS",**kwargs):
    kwargs.setdefault("trtDriftCircleContainer", "TRT_DriftCircles")
    kwargs.setdefault("trtPrdMultiTruthCollection", "PRD_MultiTruthTRT")
    kwargs.setdefault("HardScatterSplittingMode", 1)
    return BasicTRTFastDigitizationTool(name,**kwargs)

def TRTFastDigitizationToolPU(name="TRTFastDigitizationToolPU",**kwargs):
    kwargs.setdefault("trtDriftCircleContainer", "TRT_PU_DriftCircles")
    kwargs.setdefault("trtPrdMultiTruthCollection", "PRD_MultiTruthTRT_PU")
    kwargs.setdefault("HardScatterSplittingMode", 2)
    return BasicTRTFastDigitizationTool(name,**kwargs)

def TRTFastDigitizationToolSplitNoMergePU(name="TRTFastDigitizationToolSplitNoMergePU",**kwargs):
    kwargs.setdefault("trtHitCollectionName", "PileupTRTUncompressedHits")
    kwargs.setdefault("trtDriftCircleContainer", "TRT_PU_DriftCircles")
    kwargs.setdefault("trtPrdMultiTruthCollection", "PRD_MultiTruthTRT_PU")
    kwargs.setdefault("HardScatterSplittingMode", 0)
    return BasicTRTFastDigitizationTool(name,**kwargs)
