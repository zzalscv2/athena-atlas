# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

def createPhysValConfigFlags():
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    icf = AthConfigFlags()

    icf.addFlag("PhysVal.OutputFileName", "")

    icf.addFlag("PhysVal.doExample", False)
    icf.addFlag("PhysVal.doInDet", False)
    icf.addFlag("PhysVal.doInDetLargeD0", False)
    icf.addFlag("PhysVal.doBtag", False)
    icf.addFlag("PhysVal.doMET", False)
    icf.addFlag("PhysVal.doEgamma", False)
    icf.addFlag("PhysVal.doTau", False)
    icf.addFlag("PhysVal.doJet", False)
    icf.addFlag("PhysVal.doTopoCluster", False)
    icf.addFlag("PhysVal.doZee", False)
    icf.addFlag("PhysVal.doPFlow", False)
    icf.addFlag("PhysVal.doMuon", False)
    icf.addFlag("PhysVal.doLRTMuon", False)
    icf.addFlag("PhysVal.doActs", False)
    icf.addFlag("PhysVal.doLLPSecVtx", False)
    icf.addFlag("PhysVal.doLLPSecVtxLeptons", False)

    from InDetPhysValMonitoring.InDetPhysValFlags import createIDPVMConfigFlags
    icf.addFlagsCategory("PhysVal.IDPVM", createIDPVMConfigFlags, prefix=True)

    return icf
