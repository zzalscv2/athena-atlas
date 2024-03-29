# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import CfgMgr

# The earliest bunch crossing time for which interactions will be sent
# to the ZDC Digitization code.
def ZDC_FirstXing():
    return 0 #Assume ZDC is only sensitive to the current bunch crossing, for the moment

######################################################################################

# The latest bunch crossing time for which interactions will be sent
# to the ZDC Digitization code.
def ZDC_LastXing():
    return 0 #Assume ZDC is only sensitive to the current bunch crossing, for the moment

######################################################################################

def getZdcRange(name="ZdcRange" , **kwargs):
    #this is the time of the xing in ns                                                                                
    kwargs.setdefault('FirstXing', ZDC_FirstXing() )
    kwargs.setdefault('LastXing',  ZDC_LastXing() )
    kwargs.setdefault('CacheRefreshFrequency', 1.0 ) #default 0 no dataproxy reset                                              
    kwargs.setdefault('ItemList', ["ZDC_SimFiberHit_Collection#ZDC_SimFiberHit_Collection"] )
    return CfgMgr.PileUpXingFolder(name, **kwargs)

######################################################################################

def getZDC_PileUpTool(name="ZDC_PileUpTool",**kwargs):
    from Digitization.DigitizationFlags import digitizationFlags
    # Configure bunch-crossing envelope
    if digitizationFlags.doXingByXingPileUp():
        kwargs.setdefault("FirstXing", ZDC_FirstXing() )
        kwargs.setdefault("LastXing", ZDC_LastXing() )

    return CfgMgr.ZDC_PileUpTool(name, **kwargs)
