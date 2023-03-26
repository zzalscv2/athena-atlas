# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addTool
addTool("Sleptons.SleptonsConfigLegacy.getSleptonsPhysicsTool"          , "SleptonsPhysicsTool" )
addTool("Sleptons.SleptonsConfigLegacy.getAllSleptonsPhysicsTool"         , "AllSleptonsPhysicsTool" )
#####################################################
#Gravitino Section.
#####################################################
addTool("Sleptons.SleptonsConfigLegacy.getSElectronRPlusToElectronGravitino"         , "SElectronRPlusToElectronGravitino" )
addTool("Sleptons.SleptonsConfigLegacy.getSElectronRMinusToElectronGravitino"        , "SElectronRMinusToElectronGravitino" )
addTool("Sleptons.SleptonsConfigLegacy.getSMuonRPlusToMuonGravitino"       , "SMuonRPlusToMuonGravitino" )
addTool("Sleptons.SleptonsConfigLegacy.getSMuonRMinusToMuonGravitino"      , "SMuonRMinusToMuonGravitino" )
addTool("Sleptons.SleptonsConfigLegacy.getSTauLPlusToTauGravitino"        , "STauLPlusToTauGravitino" )
addTool("Sleptons.SleptonsConfigLegacy.getSTauLMinusToTauGravitino"        , "STauLMinusToTauGravitino" )
addTool("Sleptons.SleptonsConfigLegacy.getSElectronLPlusToElectronGravitino"       , "SElectronLPlusToElectronGravitino" )
addTool("Sleptons.SleptonsConfigLegacy.getSElectronLMinusToElectronGravitino"      , "SElectronLMinusToElectronGravitino" )
addTool("Sleptons.SleptonsConfigLegacy.getSMuonLPlusToMuonGravitino"        , "SMuonLPlusToMuonGravitino" )
addTool("Sleptons.SleptonsConfigLegacy.getSMuonLMinusToMuonGravitino"       , "SMuonLMinusToMuonGravitino" )
addTool("Sleptons.SleptonsConfigLegacy.getSTauRPlusToTauGravitino"       , "STauRPlusToTauGravitino" )
addTool("Sleptons.SleptonsConfigLegacy.getSTauRMinusToTauGravitino"      , "STauRMinusToTauGravitino" )
#####################################################
#On shell Tau+Neutralino
#Needed for connanihlation model.
#####################################################
addTool("Sleptons.SleptonsConfigLegacy.getSElectronRPlusToElectronNeutralino"         , "SElectronRPlusToElectronNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSElectronRMinusToElectronNeutralino"        , "SElectronRMinusToElectronNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSMuonRPlusToMuonNeutralino"       , "SMuonRPlusToMuonNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSMuonRMinusToMuonNeutralino"      , "SMuonRMinusToMuonNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSTauLPlusToTauNeutralino"        , "STauLPlusToTauNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSTauLMinusToTauNeutralino"        , "STauLMinusToTauNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSElectronLPlusToElectronNeutralino"       , "SElectronLPlusToElectronNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSElectronLMinusToElectronNeutralino"      , "SElectronLMinusToElectronNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSMuonLPlusToMuonNeutralino"        , "SMuonLPlusToMuonNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSMuonLMinusToMuonNeutralino"       , "SMuonLMinusToMuonNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSTauRPlusToTauNeutralino"       , "STauRPlusToTauNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSTauRMinusToTauNeutralino"      , "STauRMinusToTauNeutralino" )

#####################################################
#Off shell Tau+Neutralino
#
#####################################################
addTool("Sleptons.SleptonsConfigLegacy.getSTauRMinusToPionMinusNeutralino"        , "STauRMinusToPionMinusNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSTauRPlusToPionPlusNeutralino"          , "STauRPlusToPionPlusNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSTauLMinusToPionMinusNeutralino"        , "STauLMinusToPionMinusNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSTauLPlusToPionPlusNeutralino"          , "STauLPlusToPionPlusNeutralino" )

addTool("Sleptons.SleptonsConfigLegacy.getSTauRMinusToRhoMinusNeutralino"        , "STauRMinusToRhoMinusNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSTauRPlusToRhoPlusNeutralino"          , "STauRPlusToRhoPlusNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSTauLMinusToRhoMinusNeutralino"        , "STauLMinusToRhoMinusNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSTauLPlusToRhoPlusNeutralino"          , "STauLPlusToRhoPlusNeutralino" )

addTool("Sleptons.SleptonsConfigLegacy.getSTauRMinusToEMinusNeutralino"        , "STauRMinusToEMinusNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSTauRPlusToEPlusNeutralino"          , "STauRPlusToEPlusNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSTauLMinusToEMinusNeutralino"        , "STauLMinusToEMinusNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSTauLPlusToEPlusNeutralino"          , "STauLPlusToEPlusNeutralino" )

addTool("Sleptons.SleptonsConfigLegacy.getSTauRMinusToMuMinusNeutralino"        , "STauRMinusToMuMinusNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSTauRPlusToMuPlusNeutralino"          , "STauRPlusToMuPlusNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSTauLMinusToMuMinusNeutralino"        , "STauLMinusToMuMinusNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSTauLPlusToMuPlusNeutralino"          , "STauLPlusToMuPlusNeutralino" )

addTool("Sleptons.SleptonsConfigLegacy.getSTauRMinusToa1MinusNeutralino"        , "STauRMinusToa1MinusNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSTauRPlusToa1PlusNeutralino"          , "STauRPlusToa1PlusNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSTauLMinusToa1MinusNeutralino"        , "STauLMinusToa1MinusNeutralino" )
addTool("Sleptons.SleptonsConfigLegacy.getSTauLPlusToa1PlusNeutralino"          , "STauLPlusToa1PlusNeutralino" )
