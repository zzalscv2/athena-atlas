#!/usr/bin/env python
#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

'''@file dumpTruth.py
@author RD Schaffer
@date 2023-08-04
@brief Script to print out truth events for HepMC (HITS, RDO) or TruthEvent (AOD)
'''

if __name__=='__main__':
    # from AthenaCommon.Constants import INFO
    from argparse import RawTextHelpFormatter
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    parser = flags.getArgumentParser(description='Run ASCII dumper of Truth on files with either HepMC or TruthEvent format.\
    User MUST provide file(s) to read (--filesInput).\
    If file name has EVNT, HITS, RDO or AOD, then working with the different formats is automatic. Otherwise, use --doHepMC to select HepMC.\
    Also, use --doPtEtaPhi for pt/eta/phi printout, and --skipEvents, --evtMax to select events.')
    parser.add_argument('--doHepMC', action='store_true',
                        help='Run HepMCReader, otherwise run xAODTruthReader (default)')
    parser.add_argument('--doPUEventPrintout', action='store_true',
                        help='Print out PU event for xAODTruthReader')
    parser.add_argument('--doPtEtaPhi', action='store_true',
                        help='Print out particle 4-mom as pt,eta,phi. Default is px,py,pz.')
    parser.add_argument('--HepMCContainerKey', default="",
                        help='HepMC container key. If not given, set to GEN_EVENT for EVNT file. Should be TruthEvent for HITS or RDO file, which is set automatically if HITS or RDO is in file name.')
    parser.set_defaults(threads=1)
    args, _ = parser.parse_known_args()

    # Setup logs
    from AthenaCommon.Logging import log

    # Default file type is for AOD with xAOD::TruthEventContainer, 
    #   check for HITS or RDO in file name and if found switch on HepMC flag
    if len(args.filesInput):
        if ("EVNT"  in args.filesInput[0] or "RDO" in args.filesInput[0] or "HITS" in args.filesInput[0]) and "AOD" not in args.filesInput[0]:
            args.doHepMC = True
            log.info('Found EVNT, HITS or RDO in file: ' + args.filesInput[0] + '. So turning on HepMC dumping')

    # set up defaults for either HepMCReader or xAODTruthReader
    if args.doHepMC:
        if len(args.HepMCContainerKey):
            flags.addFlag("HepMCContainerKey", args.HepMCContainerKey)
        else:
            # test for RDO or HITS file, and set HepMC container key to TruthEvent
            if len(args.filesInput):
                if "RDO" in args.filesInput[0] or "HITS" in args.filesInput[0]:
                    flags.addFlag("HepMCContainerKey", "TruthEvent")  
                elif "EVNT" in args.filesInput[0]:
                    flags.addFlag("HepMCContainerKey", "GEN_EVENT")  
        # default file name
        flags.Input.Files = ['evnt.pool.root']
    else:
        flags.addFlag("xAODTruthEventContainerKey", "TruthEvents")
        flags.addFlag("xAODTruthPileupEventContainerKey", "TruthPileupEvents")
        if args.doPUEventPrintout:
            flags.addFlag("DoPUEventPrintout", True)
        else:
            flags.addFlag("DoPUEventPrintout", False)

        # default file name
        flags.Input.Files = ['truth.pool.root']
    if args.doPtEtaPhi:
        flags.addFlag("Do4momPtEtaPhi", True)
    else:
        flags.addFlag("Do4momPtEtaPhi", False)

    flags.fillFromArgs(parser=parser)
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc=MainServicesCfg(flags)
    acc.merge(PoolReadCfg(flags)) 
    if args.doHepMC:
        from xAODTruthCnv.xAODTruthCnvConfig import HepMCTruthReaderCfg
        acc.merge(HepMCTruthReaderCfg(flags))
    else:
        from xAODTruthCnv.xAODTruthCnvConfig import xAODTruthReaderCfg
        acc.merge(xAODTruthReaderCfg(flags))

    acc.store(open("HepMCTruthReader.pkl","wb"))

    from AthenaConfiguration.Utils import setupLoggingLevels
    setupLoggingLevels(flags, acc)

    statusCode = acc.run(maxEvents = 1)
    assert statusCode.isSuccess() is True, "Application execution did not succeed"
