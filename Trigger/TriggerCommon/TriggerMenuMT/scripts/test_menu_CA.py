#!/usr/bin/env python3
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

"""Standalone menu generation in CA mode"""

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AccumulatorCache import AccumulatorDecorator
from TriggerJobOpts import runHLT

# Make sure nobody uses deprecated global ConfigFlags
import AthenaConfiguration.AllConfigFlags
del AthenaConfiguration.AllConfigFlags.ConfigFlags

# Set flags
flags = initConfigFlags()
runHLT.set_flags(flags)
flags.Trigger.generateMenuDiagnostics = True
flags.Common.isOnline = True    # online environment
flags.Input.Files = []          # menu cannot depend on input files

flags.fillFromArgs()
flags.lock()

# Set the Python OutputLevel on the root logger (usually done in MainServicesCfg)
from AthenaCommon.Logging import log
log.setLevel(flags.Exec.OutputLevel)

from TriggerMenuMT.HLT.Config.GenerateMenuMT_newJO import generateMenuMT
acc = ComponentAccumulator()
menu = generateMenuMT(flags)
acc.merge(menu)

with open(flags.Trigger.triggerMenuSetup+".pkl", "wb") as f:
    acc.store(f)
AccumulatorDecorator.printStats()


