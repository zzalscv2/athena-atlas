# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
import AthenaCommon.SystemOfUnits as Units
from InDetConfig.TrackingPassFlags import createITkTrackingPassFlags


def __flagsFromConfigSettings(settings):
    flags = createITkTrackingPassFlags()
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
    flags.input_name = flags.name
    flags.minPT = [flags.pTmin] # hack to sync pT threshold used in offline and trigger
    return flags


def createTrigTrackingPassFlagsITk():
    
    #hide instantiation of flags in a function that can be consumed by addFlagsCategory
    def flagsFactory(settings):
        def hidearg():
            return __flagsFromConfigSettings(settings)
        return hidearg

    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    flags = AthConfigFlags()
    from TrigInDetConfig.ConfigSettings import ConfigSettingsInstances,getInDetTrigConfig

    for i in ConfigSettingsInstances.keys():
      category = f'Trigger.ITkTracking.{i}'
      factory = flagsFactory(getInDetTrigConfig(i))     
      flags.addFlagsCategory(category,factory,prefix=True)

    return flags


import unittest

class FlagsCopiedTest(unittest.TestCase):
    def setUp(self):
        from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
        flags.Trigger.doID
        flags.Trigger.ITkTracking.Muon
        flags.Trigger.ITkTracking.Electron.minPT = 2.0 * Units.GeV
        self.newflags = flags.cloneAndReplace('ITk.Tracking.ActivePass', 'Trigger.ITkTracking.Electron')

        self.newflags.dump(".*ITk")

    def runTest(self):
        self.assertEqual(self.newflags.ITk.Tracking.ActivePass.minPT, 2.0 * Units.GeV, msg="Flags are not copied")



class UnsetFlagsTest(FlagsCopiedTest):
    def runTest(self):
        self.assertEqual(self.newflags.ITk.Tracking.ActivePass.vertex_jet, None)


if __name__ == "__main__":
    unittest.main()