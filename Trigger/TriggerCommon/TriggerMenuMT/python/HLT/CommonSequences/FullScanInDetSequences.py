# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
from ..Config.MenuComponents import algorithmCAToGlobalWrapper
from .FullScanInDetConfig import commonInDetFullScanCfg

# ---------------------

def getCommonInDetFullScanSequence(flags):
    IDTrigConfig = getInDetTrigConfig( 'fullScan' )
    sequenceOut = IDTrigConfig.tracks_FTF()

    algs = algorithmCAToGlobalWrapper(commonInDetFullScanCfg,flags)

    return algs, sequenceOut
