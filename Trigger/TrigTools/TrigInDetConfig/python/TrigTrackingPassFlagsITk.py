# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
import AthenaCommon.SystemOfUnits as Units
from TrigInDetConfig.TrigTrackingPassFlags import createTrigTrackingPassFlags

def createTrigTrackingPassFlagsITk():
    
  flags = createTrigTrackingPassFlags(mode="ITk")
  return flags


import unittest

class FlagsCopiedTest(unittest.TestCase):
    def setUp(self):
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()
        flags.Trigger.doID
        flags.Trigger.ITkTracking.Muon
        flags.Trigger.ITkTracking.Electron.minPT = 2.0 * Units.GeV
        self.newflags = flags.cloneAndReplace('ITk.Tracking.ActiveConfig', 'Trigger.ITkTracking.Electron')

        self.newflags.dump(".*ITk")

    def runTest(self):
        self.assertEqual(self.newflags.ITk.Tracking.ActiveConfig.minPT, 2.0 * Units.GeV, msg="Flags are not copied")



class UnsetFlagsTest(FlagsCopiedTest):
    def runTest(self):
        self.assertEqual(self.newflags.ITk.Tracking.ActiveConfig.vertex_jet, None)


if __name__ == "__main__":
    unittest.main()
