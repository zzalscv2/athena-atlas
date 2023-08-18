#
#  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
#
doWriteRDOTrigger = False
doWriteBS = False
doEmptyMenu = True
include("TriggerJobOpts/runHLT_standalone.py")

from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

# ----------------------------------------------------------------
# Setup Views
# ----------------------------------------------------------------
from AthenaCommon.AlgSequence import AthSequencer
# viewSeq = AthSequencer("AthViewSeq", Sequential=True, ModeOR=False, StopOverride=False)
# topSequence += viewSeq

# from HLTSeeding.HLTSeedingConfig import mapThresholdToL1RoICollection
# roiCollectionName =  mapThresholdToL1RoICollection("EM")  
# View maker alg
# from AthenaCommon import CfgMgr

signatureName = 'Cosmic'

# TODO switch once done
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
from DecisionHandling.DecisionHandlingConf import ViewCreatorInitialROITool
from HLTSeeding.HLTSeedingConfig import mapThresholdToL1RoICollection, mapThresholdToL1DecisionCollection

inputMakerAlg = EventViewCreatorAlgorithm("IM_%s"%signatureName)
inputMakerAlg.ViewFallThrough = True
inputMakerAlg.RoIsLink = "initialRoI" # -||-
inputMakerAlg.InViewRoIs = "%sInputRoIs"%signatureName # contract with the consumer
inputMakerAlg.Views = "%sViewRoIs"%signatureName
inputMakerAlg.RoITool = ViewCreatorInitialROITool()
inputMakerAlg.InputMakerInputDecisions = [  mapThresholdToL1DecisionCollection("FSNOSEED") ] #After L1Dec there is a filter producing decision, this maps the relevant RoI to that decision 
# inputMakerAlg.InputMakerOutputDecisions = [ 'OUTDEC' ]
inputMakerAlg.InputMakerOutputDecisions =  'DUMMYOUTDEC' 


print(inputMakerAlg)

VDV = None
from TrigInDetConfig.InDetTrigFastTracking import makeInDetTrigFastTracking
viewAlgs, VDV = makeInDetTrigFastTracking(whichSignature=signatureName, rois=inputMakerAlg.InViewRoIs, doFTF= False )

# TODO add additional EFID tracking 
from AthenaCommon.CFElements import seqAND
# cosmicSequence = seqAND("%sSequence"%signatureName,viewAlgs)
# inputMakerAlg.ViewNodeName = cosmicSequence.name()

from InDetRecExample.ConfiguredNewTrackingCuts import ConfiguredNewTrackingCuts
trackingCosmicCuts  = ConfiguredNewTrackingCuts("Cosmics")

from TrigInDetConfig.EFIDTracking import makeInDetPatternRecognition
EFIDalgs = makeInDetPatternRecognition( signatureName,  NewTrackingCuts = trackingCosmicCuts )
viewAlgs.extend( EFIDalgs )
# EFIDseq = seqAND("%sEFIDSequence"%signatureName, EFIDalgs  )
#print len(EFIDalgs)

cosmicSequence = seqAND("%sSequence"%signatureName,viewAlgs)
inputMakerAlg.ViewNodeName = cosmicSequence.name()

# Cosmic seq in views
viewSequence = seqAND("%sViewSequence"%signatureName,  [ inputMakerAlg, cosmicSequence ] )

topSequence += viewSequence


