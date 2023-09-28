# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.Enums import LHCPeriod

def createIDPVMConfigFlags():
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    icf = AthConfigFlags()

    icf.addFlag("doValidateGSFTracks", False )
    icf.addFlag("doValidateLooseTracks", False )
    icf.addFlag("doValidateTightPrimaryTracks", False )
    icf.addFlag("doValidateTracksInJets", False )
    icf.addFlag("doValidateTracksInBJets", False )
    icf.addFlag("doValidateTruthToRecoNtuple", False )
    icf.addFlag("doValidateMuonMatchedTracks", False )
    icf.addFlag("doValidateElectronMatchedTracks", False )
    icf.addFlag("doValidateLargeD0Tracks", False )
    icf.addFlag("doValidateMergedLargeD0Tracks", False )
    icf.addFlag("doValidateLowPtRoITracks",False)
    icf.addFlag("doRecoOnly", False )
    icf.addFlag("doPhysValOutput", False )
    icf.addFlag("doExpertOutput", False )
    icf.addFlag("doTruthOriginPlots", False )
    icf.addFlag("doPerAuthorPlots", False )
    icf.addFlag("doHitLevelPlots", False )
    icf.addFlag("runDecoration", True )
    icf.addFlag("setTruthStrategy", "HardScatter" )
    icf.addFlag("jetsNameForHardScatter", 'AntiKt4EMTopoJets' ) # when building jets, what types of jets are built (used for hardScatterStrategy == 2)
    icf.addFlag("validateExtraTrackCollections", [] ) # List of extra track collection names to be validated in addition to Tracks.
    icf.addFlag("ancestorIDs", [] )
    icf.addFlag("selectedCharge", 0)
    icf.addFlag("requiredSiHits", 0)
    icf.addFlag("maxProdVertRadius", 300)
    icf.addFlag("hardScatterStrategy", 0 ) # The hard-scatter vertex selection strategy to use when running hard-scatter efficiency / performance plots in IDPVM. 0 corresponds to sumPt^2, 1 corresponds to sumPt
    icf.addFlag("truthMinPt", lambda pcf : 500 if pcf.GeoModel.Run <= LHCPeriod.Run3 else 1000) # Configurable pT cut for determining a "reconstructable" particle
    icf.addFlag("GRL", [])
    icf.addFlag("doIDTIDE", False ) # for IDTIDE derivation
    icf.addFlag("doTechnicalEfficiency", False) # for enabling the filling of technical efficiency

    return icf

