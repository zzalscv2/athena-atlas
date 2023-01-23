#!/usr/bin/env python3
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

"""
test of Dev menu with CA migrated menu code, reproducing runHLT_standalone_newJO
"""
from AthenaCommon.Logging import logging
log = logging.getLogger('runHLT_standalone_newJO')

from AthenaConfiguration.AllConfigFlags import ConfigFlags 
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AccumulatorCache import AccumulatorDecorator
from AthenaCommon.Configurable import Configurable
Configurable.configurableRun3Behavior = 1


# select chains, as in runHLT_standalone
ConfigFlags.addFlag("Trigger.enabledSignatures",[])  
ConfigFlags.addFlag("Trigger.disabledSignatures",[]) 
ConfigFlags.addFlag("Trigger.selectChains",[])       
ConfigFlags.addFlag("Trigger.disableChains",[]) 
ConfigFlags.Trigger.enabledSignatures = ['Muon', 'Photon','Electron']

from AthenaConfiguration.AllConfigFlags import ConfigFlags
ConfigFlags.Trigger.generateMenuDiagnostics = True

from AthenaConfiguration.TestDefaults import defaultTestFiles
ConfigFlags.Input.Files = defaultTestFiles.RAW
ConfigFlags.Trigger.triggerMenuSetup="Dev_pp_run3_v1"

ConfigFlags.Trigger.EDMVersion=3
ConfigFlags.lock()
ConfigFlags.dump()


acc = ComponentAccumulator()
from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg
acc.merge(L1ConfigSvcCfg(ConfigFlags))

from TriggerMenuMT.HLT.Config.GenerateMenuMT_newJO import generateMenuMT 
menu = generateMenuMT( ConfigFlags)
acc.merge(menu)
acc.printConfig()
AccumulatorDecorator.printStats()


