#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def jFexDBConfig(flags, name="jFEXCondAlgo"):


    acc=ComponentAccumulator()
    DBCond = CompFactory.LVL1.jFEXCondAlgo(name)
    
    if not flags.Input.isMC:
        
        ModSettings_folder  = "/TRIGGER/L1Calo/V1/Calibration/JfexModuleSettings"
        NoiseCut_folder     = "/TRIGGER/L1Calo/V1/Calibration/JfexNoiseCuts"
        SysSettingst_folder = "/TRIGGER/L1Calo/V1/Calibration/JfexSystemSettings"
        
        from IOVDbSvc.IOVDbSvcConfig import addFolders
        acc.merge(addFolders(flags, ModSettings_folder , "TRIGGER_ONL", className="CondAttrListCollection"))
        acc.merge(addFolders(flags, NoiseCut_folder    , "TRIGGER_ONL", className="CondAttrListCollection"))
        acc.merge(addFolders(flags, SysSettingst_folder, "TRIGGER_ONL", className="CondAttrListCollection"))
        
        DBCond.JfexModuleSettings = ModSettings_folder
        DBCond.JfexNoiseCuts      = NoiseCut_folder
        DBCond.JfexSystemSettings = SysSettingst_folder
    
    
    acc.addEventAlgo(DBCond)

    return acc
