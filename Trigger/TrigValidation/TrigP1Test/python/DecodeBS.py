# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# Script to unpack (Run-3) HLT bytestream and write ESD file. Only to be used
# for validation purposes. For reconstruction use the full TriggerRecoConfig.
#

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
from TriggerJobOpts.TriggerRecoConfig import Run3TriggerBSUnpackingCfg, TriggerEDMCfg

# Set and parse flags
flags = initConfigFlags()
flags.parser().add_argument('--moduleID', type=int, default=0, help='HLT module ID to decode')
args = flags.fillFromArgs()

flags.Output.ESDFileName = 'ESD.pool.root' if args.moduleID==0 else f'ESD.Module{args.moduleID}.pool.root'
flags.lock()

cfg = MainServicesCfg(flags)
cfg.merge( ByteStreamReadCfg(flags) )

acc_bs = Run3TriggerBSUnpackingCfg(flags)
acc_bs.getEventAlgo('TrigDeserialiser').ModuleID = args.moduleID
cfg.merge(acc_bs)

cfg.merge( TriggerEDMCfg(flags) )

import sys
sys.exit(cfg.run().isFailure())
