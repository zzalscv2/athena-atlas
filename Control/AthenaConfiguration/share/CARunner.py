#!/usr/bin/env python 
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import sys,os
import pickle

if __name__=="__main__":
    import argparse
    parser= argparse.ArgumentParser(prog="CARunner.py",description="Executes a pickled ComponentAccumulator",
                                    usage="CARunner.py [options]  <picklefile>")
    parser.add_argument("-d","--debug", default=None, choices=["init", "exec", "fini"], help="attach debugger (gdb) before run, <stage>")
    parser.add_argument("-i","--interactive", action='store_true', help="Drop into interactive mode")
    parser.add_argument("--evtMax", type=int, default=None, help="Max number of events to process")
    parser.add_argument("--skipEvents", type=int, default=None, help="Number of events to skip")
    parser.add_argument("-l", "--loglevel", default=None, choices=["ALL","VERBOSE","DEBUG","INFO","WARNING","ERROR","FATAL"], help="logging level")
    

    (args,leftover)=parser.parse_known_args(sys.argv[1:])

    if len(leftover)==0:
        print ("ERROR: No pickle file given")
        sys.exit(-1)

    if len(leftover)>1:
        print ("Expect exactly one pickle file, got %s" % " ".join(leftover))
        sys.exit(-1)

    inputName=leftover[0]
    if not os.access(inputName,os.R_OK):
        print("ERROR, can't read file",inputName)
        sys.exit(-1)
    
    inFile=open(inputName, 'rb')
    
    acc=pickle.load(inFile)

    nEvt=None
    if args.evtMax:
        nEvt=args.evtMax

    if acc._isMergable: #Not a top-level accumulator
        from AthenaConfiguration.MainServicesConfig import MainServicesCfg
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()
        acc1 = MainServicesCfg(flags)
        acc1.merge(acc)
    else:
        acc1 = acc

    if args.debug:
        from AthenaCommon.Debugging import DbgStage
        if args.debug not in DbgStage.allowed_values:
            print ("ERROR, Unknown debug-stage, allowed value are init, exec, fini")
            sys.exit(-1)
        acc1.setDebugStage(args.debug)


    if args.loglevel:
        from AthenaCommon import Constants
        if hasattr(Constants,args.loglevel):
            acc1.getService("MessageSvc").OutputLevel=getattr(Constants,args.loglevel)
        else:
            print ("ERROR: Unknown log-level, allowed values are ALL, VERBOSE, DEBUG,INFO, WARNING, ERROR, FATAL")
            sys.exit(-1)

    if args.interactive:
        acc1.interactive="init"

    if args.skipEvents:
        acc1.getService("EventSelector").SkipEvents=int(args.skipEvents)

    acc1.run(nEvt)
