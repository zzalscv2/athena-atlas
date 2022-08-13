# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addTool, addService, addAlgorithm
addService("Digitization.PileUpConfigLegacy.getStepArrayBM"                , "StepArrayBM")
addService("Digitization.PileUpConfigLegacy.getFixedArrayBM"               , "FixedArrayBM")
addService("Digitization.PileUpConfigLegacy.getArrayBM"                    , "ArrayBM")

addService("Digitization.PileUpConfigLegacy.getLowPtMinBiasEventSelector"  , "LowPtMinBiasEventSelector")
addService("Digitization.PileUpConfigLegacy.getHighPtMinBiasEventSelector" , "HighPtMinBiasEventSelector")
addService("Digitization.PileUpConfigLegacy.getcavernEventSelector"        , "cavernEventSelector")
addService("Digitization.PileUpConfigLegacy.getBeamGasEventSelector"       , "BeamGasEventSelector")
addService("Digitization.PileUpConfigLegacy.getBeamHaloEventSelector"      , "BeamHaloEventSelector")

addTool("Digitization.PileUpConfigLegacy.getMinBiasCache"                  , "MinBiasCache")
addTool("Digitization.PileUpConfigLegacy.getLowPtMinBiasCache"             , "LowPtMinBiasCache")
addTool("Digitization.PileUpConfigLegacy.getHighPtMinBiasCache"            , "HighPtMinBiasCache")
addTool("Digitization.PileUpConfigLegacy.getCavernCache"                   , "CavernCache")
addTool("Digitization.PileUpConfigLegacy.getBeamGasCache"                  , "BeamGasCache")
addTool("Digitization.PileUpConfigLegacy.getBeamHaloCache"                 , "BeamHaloCache")

addTool("Digitization.DigiAlgConfigLegacy.getTestPileUpTool"               , "TestPileUpTool")
addTool("Digitization.DigiAlgConfigLegacy.getTestFilterPileUpTool"         , "TestFilterPileUpTool")

addService("Digitization.PileUpConfigLegacy.getPileUpEventLoopMgr"         , "PileUpEventLoopMgr")

addAlgorithm("Digitization.DigiAlgConfigLegacy.getStandardPileUpToolsAlg"  , "StandardPileUpToolsAlg")
addAlgorithm("Digitization.DigiAlgConfigLegacy.getFastPileUpToolsAlg"      , "FastPileUpToolsAlg")
addAlgorithm("Digitization.DigiAlgConfigLegacy.getSplitPileUpToolsAlg"     , "SplitPileUpToolsAlg")
addAlgorithm("Digitization.DigiAlgConfigLegacy.getSplitSFPileUpToolsAlg"  , "SplitSFPileUpToolsAlg")
addAlgorithm("Digitization.DigiAlgConfigLegacy.getStandardSignalOnlyTruthPileUpToolsAlg"  , "StandardSignalOnlyTruthPileUpToolsAlg")
addAlgorithm("Digitization.DigiAlgConfigLegacy.getStandardInTimeOnlyTruthPileUpToolsAlg"  , "StandardInTimeOnlyTruthPileUpToolsAlg")
addAlgorithm("Digitization.DigiAlgConfigLegacy.getStandardInTimeOnlyGeantinoTruthPileUpToolsAlg"  , "StandardInTimeOnlyGeantinoTruthPileUpToolsAlg")
addAlgorithm("Digitization.DigiAlgConfigLegacy.getSplitNoMergePileUpToolsAlg"  , "SplitNoMergePileUpToolsAlg")
addAlgorithm("Digitization.DigiAlgConfigLegacy.getSplitNoMergeSFPileUpToolsAlg"  , "SplitNoMergeSFPileUpToolsAlg")
addAlgorithm("Digitization.DigiAlgConfigLegacy.getSplitNoMergeFSPileUpToolsAlg"  , "SplitNoMergeFSPileUpToolsAlg")
addAlgorithm("Digitization.DigiAlgConfigLegacy.getSplitNoMergeFFPileUpToolsAlg"  , "SplitNoMergeFFPileUpToolsAlg")
addAlgorithm("Digitization.DigiAlgConfigLegacy.getTestPileUpToolsAlg"      , "TestPileUpToolsAlg")
addAlgorithm("Digitization.DigiAlgConfigLegacy.getTestFilterPileUpToolsAlg"      , "TestFilterPileUpToolsAlg")
addAlgorithm("Digitization.DigiAlgConfigLegacy.getTestTruthJetFilterPileUpToolsAlg" , "TestTruthJetFilterPileUpToolsAlg")

addService("Digitization.PileUpMergeSvcConfigLegacy.getPileUpMergeSvc"     , "PileUpMergeSvc")
