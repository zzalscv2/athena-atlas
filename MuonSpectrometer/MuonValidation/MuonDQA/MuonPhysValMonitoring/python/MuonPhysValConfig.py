#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

'''@file MuonPhysValConfig.py
@author N. Pettersson
@date 2022-07-13
@brief Main CA-based python configuration for MuonPhysValMonitoring
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PhysValMuonCfg(flags, container='', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("IsData", not flags.Input.isMC)
    kwargs.setdefault("SlowMuonContainerName", "")

    kwargs.setdefault("SelectMuonWorkingPoints", [0, 1, 2])
    kwargs.setdefault("SelectMuonAuthors", [1, 2 , 4, 5, 6, 8, 10])
    selectMuonCat = [0, 1]
    if not flags.Input.isMC:
        selectMuonCat = [0, 1, 4]
    kwargs.setdefault("SelectMuonCategories", selectMuonCat)
    from IsolationSelection.IsolationSelectionConfig import MuonPhysValIsolationSelCfg
    kwargs.setdefault("IsoTool", acc.popToolsAndMerge(MuonPhysValIsolationSelCfg(flags)))
    from InDetConfig.InDetTrackSelectorToolConfig import MuonCombinedInDetDetailedTrackSelectorToolCfg
    kwargs.setdefault("TrackSelector",acc.popToolsAndMerge(MuonCombinedInDetDetailedTrackSelectorToolCfg(flags)))
    from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg
    kwargs.setdefault("MuonSelector", acc.popToolsAndMerge(MuonSelectionToolCfg(flags)))
    from MuonConfig.MuonRecToolsConfig import MuonEDMPrinterToolCfg
    edmprinter = acc.popToolsAndMerge(MuonEDMPrinterToolCfg(flags))
    kwargs.setdefault("MuonPrinter", CompFactory.Rec.MuonPrintingTool(MuonStationPrinter=edmprinter))
    kwargs.setdefault("EnableLumi", False)
    from AthenaCommon.Constants import WARNING
    kwargs.setdefault("OutputLevel", WARNING)
    kwargs.setdefault("DetailLevel", 10)

    acc.setPrivateTools(CompFactory.MuonPhysValMonitoring.MuonPhysValMonitoringTool("muphysval", **kwargs))
    return acc


def PhysValMuonTriggerCfg(flags, container='', **kwargs):
    acc = ComponentAccumulator()

    selectHLTMuonItems = [
        ["HLT_mu20","L1_MU20"],
        ["HLT_mu20_iloose_L1MU15","L1_MU15"],
        ["HLT_mu24","L1_MU20"],
        ["HLT_mu24_iloose_L1MU15","L1_MU15"],
        ["HLT_mu24_imedium","L1_MU20"],
        ["HLT_mu26","L1_MU20"],
        ["HLT_mu26_imedium","L1_MU20"],
        ["HLT_mu50","L1_MU20"]
    ]

    selectL1MuonItems = [
        "L1_MU4",
        "L1_MU6",
        "L1_MU10",
        "L1_MU11",
        "L1_MU15",
        "L1_MU20",
    ]

    kwargs.setdefault("SelectHLTMuonItems", selectHLTMuonItems)
    kwargs.setdefault("SelectL1MuonItems", selectL1MuonItems)
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    kwargs.setdefault("TrigDecTool", acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags)))

    acc.setPrivateTools(acc.popToolsAndMerge(
        PhysValMuonCfg(flags, container, **kwargs)))
    return acc
