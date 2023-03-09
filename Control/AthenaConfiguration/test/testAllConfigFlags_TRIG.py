#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.TestDefaults import defaultTestFiles
from TriggerJobOpts.TriggerConfigFlags import trigGeoTag, trigGlobalTag

import unittest

class Test(unittest.TestCase):

   def setUp(self):
      self.flags = initConfigFlags()
      self.flags.Trigger.doHLT = True

   def tearDown(self):
      self.flags.dump('.*AtlasVersion|.*GlobalTag')

   def test_data(self):
      """For data, trigger always uses dedicated tags."""
      self.flags.Input.Files = defaultTestFiles.RAW_RUN2

      self.assertEqual(self.flags.GeoModel.AtlasVersion, trigGeoTag(self.flags))
      self.assertEqual(self.flags.IOVDb.GlobalTag, trigGlobalTag(self.flags))

   def test_mc(self):
      """For MC, the tags should be the same as without trigger (configured from file)."""
      self.flags.Input.Files = defaultTestFiles.RDO_RUN2
      flags_notrig = initConfigFlags()
      flags_notrig.Trigger.doHLT = False
      flags_notrig.Input.Files = self.flags.Input.Files

      self.assertEqual(self.flags.GeoModel.AtlasVersion, flags_notrig.GeoModel.AtlasVersion)
      self.assertEqual(self.flags.IOVDb.GlobalTag, flags_notrig.IOVDb.GlobalTag)


if __name__ == "__main__":
   unittest.main()
