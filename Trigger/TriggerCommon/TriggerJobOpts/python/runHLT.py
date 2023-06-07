# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
CA module to configure the (standalone) HLT for athena and athenaHLT.
There is a separate entry point for each application to tailor some
flags and services. All common code should go to runLTCfg(flags).

Usage:
  athena --CA [options] TriggerJobOpts/runHLT.py [flags]
  athenaHLT [options] TriggerJobOpts.runHLT [flags]

  python -m TriggerJobOpts.runHLT  # not recommended (due to missing LD_PRELOADs)
"""


def lock_and_restrict(flags):
   """Deny access to a few flags and lock"""

   def bomb(x):
      raise RuntimeError("Concurrency flags cannot be used in the HLT to ensure "
                         "that the configuration is portable across different CPUs")

   flags.Concurrency.NumProcs = bomb
   flags.Concurrency.NumThreads = bomb
   flags.Concurrency.NumConcurrentEvents = bomb
   flags.lock()


def set_flags(flags):
   """Set default flags for running HLT"""

   from AthenaConfiguration.Enums import BeamType

   flags.Trigger.doHLT = True    # needs to be set early as other flags depend on it
   flags.Trigger.EDMVersion = 3  # Run-3 EDM
   flags.Beam.Type = BeamType.Collisions
   flags.InDet.useDCS = False    # DCS is in general not available online
   flags.Muon.MuonTrigger = True # Setup muon reconstruction for trigger

   # Disable some forward detetors
   flags.Detector.GeometryALFA = False
   flags.Detector.GeometryFwdRegion = False
   flags.Detector.GeometryLucid = False

   # Increase scheduler checks and verbosity
   flags.Scheduler.CheckDependencies = True
   flags.Scheduler.ShowControlFlow = True
   flags.Scheduler.ShowDataDeps = True
   flags.Scheduler.EnableVerboseViews = True
   flags.Input.FailOnUnknownCollections = True
   flags.Scheduler.AutoLoadUnmetDependencies = False

   # Additional flags to filter chains
   flags.addFlag("Trigger.enabledSignatures",[])
   flags.addFlag("Trigger.disabledSignatures",[])
   flags.addFlag("Trigger.selectChains",[])
   flags.addFlag("Trigger.disableChains",[])

   ### TEMPORARY settings:
   # FIXME: disable coherent prescales during development phase to allow menu with single-chain CPS groups
   flags.Trigger.disableCPS = True

   flags.Trigger.enabledSignatures = ['Muon', 'Tau','MinBias','Bphysics','Egamma', 'Electron', 'Photon', 'MET', 'Jet','Bjet','Calib']
   #--------------#
   #Leave commented lines for tests, since this is under development
   #flags.Trigger.triggerMenuModifier=[ 'emptyMenu','HLT_mu8_L1MU5VF']
   #flags.Trigger.selectChains =  ['HLT_mu4_L1MU3V','HLT_mu8_L1MU5VF','HLT_2mu6_L12MU5VF', 'HLT_mu24_mu6_L1MU14FCH','HLT_mu24_mu6_probe_L1MU14FCH'] #, 'HLT_mu4_mu6_L12MU3V']
   #--------------#


def runHLTCfg(flags):
   """Main function to configure the HLT in athena and athenaHLT"""
   from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
   from AthenaCommon.Logging import logging

   log = logging.getLogger('runHLT')
   acc = ComponentAccumulator()

   # Load these objects from StoreGate
   loadFromSG = [('xAOD::EventInfo', 'StoreGateSvc+EventInfo'),
                 ('TrigConf::L1Menu','DetectorStore+L1TriggerMenu'),
                 ('TrigConf::HLTMenu','DetectorStore+HLTTriggerMenu')]

   from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
   acc.merge(SGInputLoaderCfg(flags, loadFromSG))

   from TriggerJobOpts.TriggerHistSvcConfig import TriggerHistSvcConfig
   acc.merge(TriggerHistSvcConfig(flags))

   # Menu
   from TriggerMenuMT.HLT.Config.GenerateMenuMT_newJO import generateMenuMT
   from TriggerJobOpts.TriggerConfig import triggerRunCfg
   menu = triggerRunCfg(flags, menu=generateMenuMT)
   acc.merge(menu)

   from LumiBlockComps.LumiBlockMuWriterConfig import LumiBlockMuWriterCfg
   acc.merge(LumiBlockMuWriterCfg(flags), sequenceName="HLTBeginSeq")

   if flags.Trigger.doTransientByteStream and flags.Trigger.doCalo:
       from TriggerJobOpts.TriggerTransBSConfig import triggerTransBSCfg_Calo
       acc.merge(triggerTransBSCfg_Calo(flags), sequenceName="HLTBeginSeq")

   # L1 simulation
   if flags.Trigger.doLVL1:
       from TriggerJobOpts.Lvl1SimulationConfig import Lvl1SimulationCfg
       acc.merge(Lvl1SimulationCfg(flags), sequenceName="HLTBeginSeq")

   # Track overlay needs this to ensure that the collections are copied correctly
   # (due to the hardcoding of the name in the converters)
   if flags.Overlay.doTrackOverlay:
       from TrkEventCnvTools.TrkEventCnvToolsConfigCA import TrkEventCnvSuperToolCfg
       acc.merge(TrkEventCnvSuperToolCfg(flags))

   if flags.Common.isOnline:
     from TrigOnlineMonitor.TrigOnlineMonitorConfig import trigOpMonitorCfg
     acc.merge( trigOpMonitorCfg(flags) )

   # Print config and statistics
   if log.getEffectiveLevel() <= logging.DEBUG:
       acc.printConfig(withDetails=False, summariseProps=True, printDefaults=True)

   from AthenaConfiguration.AccumulatorCache import AccumulatorDecorator
   AccumulatorDecorator.printStats()

   return acc


def athenaHLTCfg(flags):
   """Top-level cfg function when running in athenaHLT"""
   # Set default flags for running HLT
   set_flags(flags)

   # Lock flags
   lock_and_restrict(flags)

   # Configure HLT
   acc = runHLTCfg(flags)
   return acc


def athenaCfg():
   """Top-level cfg function when running in athena"""
   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   from AthenaConfiguration.Enums import Format

   # Set default flags for running HLT
   flags = initConfigFlags()
   set_flags(flags)

   # To allow running from MC
   flags.Common.isOnline = lambda f: not f.Input.isMC

   # Output configuration - currently testing offline workflow
   flags.Trigger.writeBS = False
   flags.Output.doWriteRDO = True
   flags.Output.RDOFileName = 'RDO_TRIG.pool.root'

   # TODO: These should be set via command line:
   flags.Exec.MaxEvents = 50
   flags.Concurrency.NumThreads = 1
   flags.Trigger.doLVL1 = True # run L1 sim also on data

   # Fill flags from command line
   parser = flags.getArgumentParser()
   flags.fillFromArgs(parser=parser)

   # Configure main services
   _allflags = flags.clone()   # copy including Concurrency flags
   _allflags.lock()
   from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   acc = MainServicesCfg(_allflags)
   del _allflags

   # Lock flags
   lock_and_restrict(flags)

   if flags.Input.Format is Format.BS:
       from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
       acc.merge(ByteStreamReadCfg(flags))
   else:
       from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
       acc.merge(PoolReadCfg(flags))

   # Configure HLT
   acc.merge(runHLTCfg(flags))
   return acc


def main(flags=None):
   """This method is called by athena (no flags) and athenaHLT (with pre-populated flags)"""

   # Make sure nobody uses deprecated global ConfigFlags
   import AthenaConfiguration.AllConfigFlags
   del AthenaConfiguration.AllConfigFlags.ConfigFlags

   return athenaCfg() if flags is None else athenaHLTCfg(flags)


# This entry point is only used when running in athena
if __name__ == "__main__":
   import sys
   sys.exit(main().run().isFailure())
