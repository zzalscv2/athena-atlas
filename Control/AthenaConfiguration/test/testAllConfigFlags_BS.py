#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
import unittest

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.Enums import LHCPeriod
from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultGeometryTags


class Test(unittest.TestCase):
    def setUp(self):
        self.flags = initConfigFlags()

    def tearDown(self):
        self.flags.dump()

    def test_run2_lite(self):
        self.flags.Input.Files = defaultTestFiles.RAW_RUN2
        self.flags.initAll()
        self.flags.lock()

        self.assertEqual(self.flags.GeoModel.Run, LHCPeriod.Run2)

    def test_run2(self):
        self.flags.Input.Files = defaultTestFiles.RAW_RUN2
        self.flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
        self.flags._loadDynaFlags("GeoModel")
        self.flags.initAll()
        self.flags.lock()

        self.assertEqual(self.flags.GeoModel.Run, LHCPeriod.Run2)

    def test_run3_lite(self):
        self.flags.Input.Files = defaultTestFiles.RAW_RUN3
        self.flags.initAll()
        self.flags.lock()

        self.assertEqual(self.flags.GeoModel.Run, LHCPeriod.Run3)

    def test_run3(self):
        self.flags.Input.Files = defaultTestFiles.RAW_RUN3
        self.flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
        self.flags._loadDynaFlags("GeoModel")
        self.flags.initAll()
        self.flags.lock()

        self.assertEqual(self.flags.GeoModel.Run, LHCPeriod.Run3)


if __name__ == "__main__":
   unittest.main()
