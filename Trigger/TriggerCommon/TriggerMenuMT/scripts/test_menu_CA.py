#!/usr/bin/env python3
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

"""
test of Dev menu with CA migrated menu code, reproducing runHLT_standalone_newJO
"""
from AthenaCommon.Logging import logging
log = logging.getLogger('test_menu_CA')

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AccumulatorCache import AccumulatorDecorator

# Make sure nobody uses deprecated global ConfigFlags
import AthenaConfiguration.AllConfigFlags
del AthenaConfiguration.AllConfigFlags.ConfigFlags

flags = initConfigFlags()

# select chains, as in runHLT_standalone
flags.addFlag("Trigger.enabledSignatures",[])
flags.addFlag("Trigger.disabledSignatures",[])
flags.addFlag("Trigger.selectChains",[])
flags.addFlag("Trigger.disableChains",[])
flags.Trigger.enabledSignatures = ['Muon', 'Photon','Electron', 'MinBias', 'HeavyIon']

flags.Trigger.generateMenuDiagnostics = True

from AthenaConfiguration.TestDefaults import defaultTestFiles
flags.Input.Files = defaultTestFiles.RAW_RUN2
flags.Trigger.triggerMenuSetup="Dev_pp_run3_v1"

flags.Trigger.EDMVersion=3
flags.fillFromArgs()
flags.lock()
flags.dump()


acc = ComponentAccumulator()
from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg
acc.merge(L1ConfigSvcCfg(flags))

from TriggerMenuMT.HLT.Config.GenerateMenuMT_newJO import generateMenuMT 
menu = generateMenuMT(flags)
acc.merge(menu)

acc.printConfig()
AccumulatorDecorator.printStats()


