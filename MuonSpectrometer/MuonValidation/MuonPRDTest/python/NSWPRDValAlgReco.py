# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# jobOptions to activate the dump of the NSWPRDValAlg nTuple
# This file can be used with Reco_tf by specifying --postInclude MuonPRDTest.NSWPRDValAlgReco.NSWPRDValAlgRecoCfg
# It dumps Truth, MuEntry and Hits, Digits, SDOs and RDOs for MM and sTGC

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def NSWPRDValAlgRecoCfg(flags, name = "NSWPRDValAlg", **kwargs):
    result = ComponentAccumulator()

    histSvc = CompFactory.THistSvc(Output=["NSWPRDValAlg DATAFILE='NSWPRDValAlg.reco.ntuple.root' OPT='RECREATE'"])
    result.addService(histSvc) 

    kwargs.setdefault("doTruth", True)
    kwargs.setdefault("doMuEntry", True)

    kwargs.setdefault("doMMHit", False) # not present in RDO files
    kwargs.setdefault("doMMDigit", False) # not present in RDO files
    kwargs.setdefault("doMMRDO", flags.Detector.EnableMM)
    kwargs.setdefault("doMMPRD", flags.Detector.EnableMM)
    kwargs.setdefault("doMMFastDigit", False)

    kwargs.setdefault("doSTGCHit", False)
    kwargs.setdefault("doSTGCDigit", False) 
    kwargs.setdefault("doSTGCRDO", flags.Detector.EnablesTGC)
    kwargs.setdefault("doSTGCPRD",flags.Detector.EnablesTGC )
    kwargs.setdefault("doSTGCFastDigit", False)

    kwargs.setdefault("doRPCHit", False) # no RPC_Hits present in RDO files
    kwargs.setdefault("doRPCSDO", True)
    kwargs.setdefault("doRPCDigit", False) # no RPC_DIGITS present in RDO files

    kwargs.setdefault("doMDTHit", False) # no MDT_Hits present in RDO files
    kwargs.setdefault("doMDTSDO", True)
    kwargs.setdefault("doMDTDigit", False) # no MDT_DIGITS present in RDO files

    kwargs.setdefault("doTGCHit", False) # no TGC_Hits present in RDO files
    kwargs.setdefault("doTGCSDO", True)
    kwargs.setdefault("doTGCDigit", False) # no TGC_DIGITS present in RDO files
    kwargs.setdefault("doTGCRDO", False)
    kwargs.setdefault("doTGCPRD", False)


    #Turn off by default but keep the option to turn on for validation of the NSW 
    kwargs.setdefault("doMMSDO", False) 
    kwargs.setdefault("doSTGCSDO", False) 

    if not  flags.Detector.EnableCSC:
        kwargs.setdefault("CscRDODecoder","") # Remove the tool to prevent initializing CSC calibration tool
    kwargs.setdefault("doCSCHit", False) # no CSC_Hits present in RDO files
    kwargs.setdefault("doCSCSDO", flags.Detector.EnableCSC)
    kwargs.setdefault("doCSCDigit", False) # no CSC_DIGITS present in RDO files
    kwargs.setdefault("doCSCRDO", False)
    kwargs.setdefault("doCSCPRD", flags.Detector.EnableCSC)

    NSWPRDValAlg = CompFactory.NSWPRDValAlg(name, **kwargs)
    result.addEventAlgo(NSWPRDValAlg)

    return result
