# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
import AthenaCommon.SystemOfUnits as Units
from InDetConfig.TrackingPassFlags import createTrackingPassFlags


def __flagsFromConfigSettings(settings):
    flags = createTrackingPassFlags()
    for setting, value in settings.__dict__.items():
        setting = setting.lstrip("_")
        if setting in flags._flagdict:
            if value is not None: 
                #flags._flagdict[setting] = value
                flags._flagdict[setting].set(value)
        else:
            if value is None: 
                flags.addFlag(setting, lambda pf: None)
            else:
                flags.addFlag(setting, value)

    flags.addFlag("trkTracks_FTF", f'HLT_IDTrkTrack_{flags.suffix}_FTF')
    flags.addFlag("tracks_FTF", f'HLT_IDTrack_{flags.suffix}_FTF')
    flags.addFlag("trkTracks_IDTrig", f'HLT_IDTrkTrack_{flags.suffix}_IDTrig')
    flags.addFlag("tracks_IDTrig", f"HLT_IDTrack_{flags.suffix}_IDTrig")
    flags.addFlag("refitROT", False) # should likely be moved to ConfigSettingsBase
    flags.addFlag("trtExtensionType", "xf") # should likely be moved to ConfigSettingsBase
    flags.minPT = flags.pTmin # hack to sync pT threshold used in offline and trigger

    #temporary - to be reworked
    flags.nHolesMax           = 2
    flags.nHolesGapMax        = 2
    flags.Xi2max              = 9.
    flags.Xi2maxNoAdd         = 25.
    flags.seedFilterLevel     = 0.
    flags.nWeightedClustersMin= 6
    
    if flags.isLRT:
      flags.nHolesGapMax        = 1
      flags.nWeightedClustersMin= 8
    if flags.input_name=="cosmics":
      flags.nClustersMin        = 4
      flags.nHolesMax           = 3
      flags.Xi2max              = 60.
      flags.Xi2maxNoAdd         = 100.
      flags.nWeightedClustersMin= 8
    

    return flags


def createTrigTrackingPassFlags():
    
    #hide instantiation of flags in a function that can be consumed by addFlagsCategory
    def flagsFactory(settings):
        def hidearg():
            return __flagsFromConfigSettings(settings)
        return hidearg

    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    flags = AthConfigFlags()
    from TrigInDetConfig.ConfigSettings import ConfigSettingsInstances,getInDetTrigConfig

    for i in ConfigSettingsInstances.keys():
      category = f'Trigger.InDetTracking.{i}'
      factory = flagsFactory(getInDetTrigConfig(i))     
      flags.addFlagsCategory(category,factory,prefix=True)

    flags.addFlag('Trigger.InDetTracking.RoiZedWidthDefault', 180.0)

    return flags


import unittest

class FlagsCopiedTest(unittest.TestCase):
    def setUp(self):
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()
        flags.Trigger.doID
        flags.Trigger.InDetTracking.Muon
        flags.Trigger.InDetTracking.Electron.minPT = 2.0 * Units.GeV
        self.newflags = flags.cloneAndReplace('InDet.Tracking.ActiveConfig', 'Trigger.InDetTracking.Electron')

        self.newflags.dump(".*InDet")

    def runTest(self):
        self.assertEqual(self.newflags.InDet.Tracking.ActiveConfig.minPT, 2.0 * Units.GeV, msg="Flags are not copied")



class UnsetFlagsTest(FlagsCopiedTest):
    def runTest(self):
        self.assertEqual(self.newflags.InDet.Tracking.ActiveConfig.vertex_jet, None)


if __name__ == "__main__":
    unittest.main()
