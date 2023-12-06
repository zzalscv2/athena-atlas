# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import LHCPeriod, ProductionStep

Run1Grades = [ "Good", "BlaShared", "PixShared", "SctShared", "0HitBLayer" ]
Run2Grades = [ "0HitIn0HitNInExp2","0HitIn0HitNInExpIn","0HitIn0HitNInExpNIn","0HitIn0HitNIn",
               "0HitInExp", "0HitIn",
               "0HitNInExp", "0HitNIn",
               "InANDNInShared", "PixShared", "SctShared",
               "InANDNInSplit", "PixSplit",
               "Good" ]
Run4Grades = [ "A01","A02","A03","A04","A05","A06","A07","A08","A14_1","A14_2","A14_3","A14_4",
               "B01","B02","B03","B04","B05","B06","B07","B08","B14_1","B14_2","B14_3","B14_4",
               "C01","C02030405","C06","C07","C08","C14_1","C14_2","C14_3","C14_4" ]
calibrationChannelAliases = [
    "AntiKt4EMTopo->AntiKt4EMTopo,AntiKt4EMPFlow",
    "AntiKt4EMPFlow->AntiKt4EMPFlow,AntiKt4EMTopo",
    "AntiKt4HI->AntiKt4HI,AntiKt4EMPFlow,AntiKt4EMTopo,AntiKt4LCTopo",
    "AntiKtVR30Rmax4Rmin02PV0Track->AntiKtVR30Rmax4Rmin02PV0Track,AntiKt4EMPFlow,AntiKt4EMTopo",
    "AntiKt4PFlowCustomVtx->AntiKt4EMTopo",
    "AntiKtVR30Rmax4Rmin02Track->AntiKtVR30Rmax4Rmin02PV0Track,AntiKt4EMPFlow,AntiKt4EMTopo",

]

def getGrades(flags):
    if flags.GeoModel.Run is LHCPeriod.Run1:
        return Run1Grades
    elif flags.GeoModel.Run in [LHCPeriod.Run2, LHCPeriod.Run3]:
        return Run2Grades
    else:
        return Run4Grades


def getTaggerList(flags):
    base = ['IP2D','IP3D','SV1','JetFitterNN']
    if flags.Trigger.doHLT:
        base = ['SV1','JetFitterNN']
    if flags.GeoModel.Run >= LHCPeriod.Run4:
        base += ['MV2c10']
    flip = ['IP2DNeg', 'IP3DNeg','IP2DFlip', 'IP3DFlip','SV1Flip']
    if flags.BTagging.RunFlipTaggers:
        return base + flip
    return base


def minimumJetPtForTrackAssociation(flags):
    if flags.Trigger.doHLT:
        return 5e3
    return 4e3


def calibrationTag(flags):
    if flags.GeoModel.Run >= LHCPeriod.Run4:
        return "BTagCalibITk-23-00-03-v1"
    return ""


def saveSv1(prevFlags):
    return prevFlags.Common.ProductionStep is ProductionStep.Derivation or prevFlags.GeoModel.Run >= LHCPeriod.Run4


def runOldSecVrtSecIncl(prevFlags):
    return prevFlags.Common.ProductionStep is ProductionStep.Derivation


def isRun3Derivation(flags):
    derivation = flags.Common.ProductionStep is ProductionStep.Derivation
    before_the_future = flags.GeoModel.Run < LHCPeriod.Run4
    return derivation and before_the_future


def runFlipTag(flags):
    return isRun3Derivation(flags)

def getNNs(flags):
    # dummy for now
    caldir = 'BTagging/20231205/GN2v01/antikt4empflow'
    pf_nns = [f'{caldir}/network_fold{n}.onnx' for n in range(4)]
    if isRun3Derivation(flags):
        return {
            'AntiKt4EMPFlowJets': [
                {
                    'folds': pf_nns,
                },
            ]
        }
    return {}


def createBTaggingConfigFlags():
    btagcf = AthConfigFlags()

    btagcf.addFlag("BTagging.taggerList", getTaggerList)
    btagcf.addFlag("BTagging.databaseScheme", '')
    btagcf.addFlag("BTagging.calibrationChannelAliases",
                   calibrationChannelAliases)
    btagcf.addFlag("BTagging.forcedCalibrationChannel", '')
    btagcf.addFlag("BTagging.calibrationTag",
                   calibrationTag)

    # the track association minimum is set to 4 GeV because of track
    # jets in offline reconstruction.
    btagcf.addFlag("BTagging.minimumJetPtForTrackAssociation",
                   minimumJetPtForTrackAssociation)

    # these are only used for IPxD and SV1 likelihoods
    btagcf.addFlag("BTagging.RunModus", "analysis") # reference mode used in FlavourTagPerformanceFramework (RetagFragment.py)
    btagcf.addFlag("BTagging.ReferenceType", "ALL") # reference type for IP and SV taggers (B, UDSG, ALL)
    btagcf.addFlag("BTagging.JetPtMinRef", 15e3) # in MeV for uncalibrated pt
    btagcf.addFlag("BTagging.Grades", getGrades)


    # Taggers for validation
    btagcf.addFlag("BTagging.SaveSV1Probabilities", saveSv1)
    #Do we really need this in AthConfigFlags?
    #Comments in BTaggingConfiguration.py
    btagcf.addFlag("BTagging.OutputFiles.Prefix", "BTagging_")
    btagcf.addFlag("BTagging.GeneralToolSuffix",'') #Not sure it will stay like that later on. Was '', 'Trig, or 'AODFix'
    # Run the flip taggers
    btagcf.addFlag("BTagging.RunFlipTaggers", runFlipTag)

   # Trackless approach
    btagcf.addFlag("BTagging.Trackless", False)
    btagcf.addFlag("BTagging.Trackless_JetCollection", "AntiKt4EMPFlowJets")
    btagcf.addFlag("BTagging.Trackless_JetPtMin", 250) #in GeV
    btagcf.addFlag("BTagging.Trackless_dR", 0.4)

    # more aggressive trackless approach
    btagcf.addFlag("BTagging.savePixelHits", False)
    btagcf.addFlag("BTagging.saveSCTHits", False)

    # experimental flags
    btagcf.addFlag("BTagging.Pseudotrack", False)

    #NewVrtSecInclusiveAlg
    btagcf.addFlag("BTagging.RunNewVrtSecInclusive", runOldSecVrtSecIncl)

    # track classification tool flags
    btagcf.addFlag("BTagging.TrkClassFiveBinMode",False)

    # a flag to add V0finder
    btagcf.addFlag("BTagging.AddV0Finder", False)

    # (multifold) NN trainings, each jet collection maps to a list of
    # dicts. The dict has several keys:
    #  - folds: list of NNs to run
    #  - remapping (optional): any variable remapping
    btagcf.addFlag("BTagging.NNs", getNNs)

    return btagcf
