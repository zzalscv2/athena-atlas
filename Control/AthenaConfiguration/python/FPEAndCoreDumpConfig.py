# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator



#Steered by flag 

def FPEAndCoreDumpCfg(flags):
    cfg=ComponentAccumulator()
    cds=CompFactory.CoreDumpSvc(FastStackTrace=True)
    
    #AthGeneration & AthAnalysis don't contain the FPEAuditor
    if flags.Exec.FPE != -2 and hasattr(CompFactory,"FPEAuditor"):
        #If we run with FPEAuditor, the CoreDumpSvc should not catch SIGFPE 
        #(but otherwise use the default list of signals)     
        signalsToCatch=[int(i) for i in cds.Signals] # Get the default

        if flags.Exec.FPE<0:
            cfg.addService(CompFactory.FPEControlSvc(),create=True)
        else:
            cfg.addAuditor(CompFactory.FPEAuditor(NStacktracesOnFPE=flags.Exec.FPE))
            from signal import SIGFPE
            signalsToCatch.remove(SIGFPE)
            cds.Signals=signalsToCatch
        pass

    cfg.addService(cds,create=True)

    return cfg
    
