# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# jobOptions to activate the dump of the NSWPRDValAlg nTuple
# This file can be used with Sim_tf by specifying --postInclude MuonPRDTest.NSWPRDValAlgSim.NSWPRDValAlgSimCfg
# It dumps Truth, MuEntry and Hits, Digits, SDOs and RDOs for MM and sTGC

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import INFO

def NSWPRDValAlgSimCfg(flags, name = "NSWPRDValAlg", **kwargs):
    result = ComponentAccumulator()

    histSvc = CompFactory.THistSvc(Output=["NSWPRDValAlg DATAFILE='NSWPRDValAlg.digi.ntuple.root' OPT='RECREATE'"])
    result.addService(histSvc) 

    kwargs.setdefault("OutputLevel", INFO)
    kwargs.setdefault("doTruth", True)
    kwargs.setdefault("doMuEntry", True)

    kwargs.setdefault("doMDTHit", True)
    kwargs.setdefault("doRPCHit", True)
    kwargs.setdefault("doTGCHit", True)

    kwargs.setdefault("doMMHit", flags.Detector.EnableMM)
    kwargs.setdefault("doSTGCHit", flags.Detector.EnablesTGC)
    kwargs.setdefault("doCSCHit", flags.Detector.EnableCSC)

    if not  flags.Detector.EnableCSC:
        kwargs.setdefault("CscRDODecoder","") # Remove the tool to prevent initializing CSC calibration tool
    
    NSWPRDValAlg = CompFactory.NSWPRDValAlg(name, **kwargs)
    result.addEventAlgo(NSWPRDValAlg)

    return result
