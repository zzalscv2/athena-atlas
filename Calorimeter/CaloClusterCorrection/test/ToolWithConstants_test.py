#!/usr/bin/env python
#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: CaloRec/python/ToolWithConstants_test.py
# Author: scott snyder
# Date: Apr, 2020
# Brief: Test for ToolWithConstants.
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaPython.PyAthenaComps import Alg, StatusCode
from AthenaConfiguration.ComponentFactory import CompFactory
import sys
import ROOT


class TestAlg (Alg):
    def initialize (self):
        ROOT.errorcheck.ReportMessage.hideFunctionNames(True)
        toolh = ROOT.ToolHandle(ROOT.CaloUtils.ToolWithConstantsTestTool)
        self.tool1 = toolh ('CaloUtils::ToolWithConstantsTestTool/tool1')
        if not self.tool1.retrieve():
            return StatusCode.Failure
        self.tool2 = toolh ('CaloUtils::ToolWithConstantsTestTool/tool2')
        if not self.tool2.retrieve():
            return StatusCode.Failure
        return StatusCode.Success
    def execute (self):
        ctx = self.getContext()
        print (ctx.eventID().run_number(), ctx.eventID().lumi_block(),
               ctx.eventID().time_stamp())
        sys.stdout.flush()
        self.tool1.execute (ctx).ignore()
        self.tool2.execute (ctx).ignore()

        if ctx.evt() == 0:
            self.tool2.testWriteConstants (ctx)
            self.testMerge (ctx)

        return StatusCode.Success


    def testMerge (self, ctx):
        tc = ROOT.CaloRec.ToolConstants()
        assert self.tool1.mergeConstants (tc, ctx).isSuccess()
        print ('testMerge: ', tc.clsname(), tc.version(), tc.toString (''))
        sys.stdout.flush()

        tc = ROOT.CaloRec.ToolConstants()
        tc.clsname ('foofoo')
        assert self.tool1.mergeConstants (tc, ctx).isFailure()

        tc = ROOT.CaloRec.ToolConstants()
        tc.clsname ('CaloUtils::ToolWithConstantsTestTool')
        tc.version (10)
        assert self.tool1.mergeConstants (tc, ctx).isFailure()
        return


def testCfg (flags):
    result = ComponentAccumulator()

    tool1 = CompFactory.CaloUtils.ToolWithConstantsTestTool \
            ('tool1',
             RunNumber = 310000,
             prefix = 'test.',
             cf1 = 2.5,
             ci1 = 10,
             cb1 = False,
             ca1 = "[3, 4.5, 6]",
             ca2 = "[[4, 5], [6, 7], [9, 8]]")
    result.addPublicTool (tool1)

    tool2 = CompFactory.CaloUtils.ToolWithConstantsTestTool \
            ('tool2',
             RunNumber = 310000,
             CondKey = 'test2Cond',
             DBHandleKey = 'test2Cond',
             prefix = 'test.',
             cf1 = 2.5,
             ci1 = 10,
             ca1 = "[3, 4.5, 6]")
    result.addPublicTool (tool2)

    result.addEventAlgo (TestAlg ('TestAlg'))
    return result


ROOT.errorcheck.ReportMessage.hideFunctionNames (True)

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.TestDefaults import defaultTestFiles
flags = initConfigFlags()
flags.Input.Files = defaultTestFiles.RDO_RUN2
flags.Input.TimeStamps = [1000]

flags.lock()
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg (flags)

from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
acc.merge (McEventSelectorCfg (flags, EventsPerLB = 2))

acc.merge (testCfg (flags))
acc.run(8)
