# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags

def createBTaggingConfigFlags():
    btagcf = AthConfigFlags()

    btagcf.addFlag("BTagging.run2TaggersList", ['IP2D','IP3D','SV1','SoftMu','JetFitterNN','MV2c10'])
    btagcf.addFlag("BTagging.Run2TrigTaggers", ['IP2D','IP3D','SV1','JetFitterNN','MV2c10'])
    # Disable JetVertexCharge ATLASRECTS-4506
    btagcf.addFlag("BTagging.RunModus", "analysis") # reference mode used in FlavourTagPerformanceFramework (RetagFragment.py)
    btagcf.addFlag("BTagging.ReferenceType", "ALL") # reference type for IP and SV taggers (B, UDSG, ALL)
    btagcf.addFlag("BTagging.JetPtMinRef", 15e3) # in MeV for uncalibrated pt
    btagcf.addFlag("BTagging.PrimaryVertexCollectionName", "PrimaryVertices")
    btagcf.addFlag("BTagging.Grades", [ "0HitIn0HitNInExp2","0HitIn0HitNInExpIn","0HitIn0HitNInExpNIn","0HitIn0HitNIn",
                                        "0HitInExp", "0HitIn",
                                        "0HitNInExp", "0HitNIn",
                                        "InANDNInShared", "PixShared", "SctShared",
                                        "InANDNInSplit", "PixSplit",
                                        "Good"])
    # Taggers for validation
    btagcf.addFlag("BTagging.SaveSV1Probabilities",False)
    btagcf.addFlag("BTagging.RunJetFitterNN",False)
    #Do we really need this in AthConfigFlags?
    #Comments in BTaggingConfiguration.py
    btagcf.addFlag("BTagging.OutputFiles.Prefix", "BTagging_")
    btagcf.addFlag("BTagging.GeneralToolSuffix",'') #Not sure it will stay like that later on. Was '', 'Trig, or 'AODFix'
    # Run the flip taggers
    btagcf.addFlag("BTagging.RunFlipTaggers", False)

    return btagcf
