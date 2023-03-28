# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addTool
addTool("Sleptons.SleptonsConfig.getSleptonsPhysicsTool"          , "SleptonsPhysicsTool" )
addTool("Sleptons.SleptonsConfig.getAllSleptonsPhysicsTool"         , "AllSleptonsPhysicsTool" )
#####################################################
#Gravitino Section.
#####################################################
addTool("Sleptons.SleptonsConfig.getSElectronRPlusToElectronGravitino"         , "SElectronRPlusToElectronGravitino" )
addTool("Sleptons.SleptonsConfig.getSElectronRMinusToElectronGravitino"        , "SElectronRMinusToElectronGravitino" )
addTool("Sleptons.SleptonsConfig.getSMuonRPlusToMuonGravitino"       , "SMuonRPlusToMuonGravitino" )
addTool("Sleptons.SleptonsConfig.getSMuonRMinusToMuonGravitino"      , "SMuonRMinusToMuonGravitino" )
addTool("Sleptons.SleptonsConfig.getSTauLPlusToTauGravitino"        , "STauLPlusToTauGravitino" )
addTool("Sleptons.SleptonsConfig.getSTauLMinusToTauGravitino"        , "STauLMinusToTauGravitino" )
addTool("Sleptons.SleptonsConfig.getSElectronLPlusToElectronGravitino"       , "SElectronLPlusToElectronGravitino" )
addTool("Sleptons.SleptonsConfig.getSElectronLMinusToElectronGravitino"      , "SElectronLMinusToElectronGravitino" )
addTool("Sleptons.SleptonsConfig.getSMuonLPlusToMuonGravitino"        , "SMuonLPlusToMuonGravitino" )
addTool("Sleptons.SleptonsConfig.getSMuonLMinusToMuonGravitino"       , "SMuonLMinusToMuonGravitino" )
addTool("Sleptons.SleptonsConfig.getSTauRPlusToTauGravitino"       , "STauRPlusToTauGravitino" )
addTool("Sleptons.SleptonsConfig.getSTauRMinusToTauGravitino"      , "STauRMinusToTauGravitino" )
#####################################################
#On shell Tau+Neutralino
#Needed for connanihlation model.
#####################################################
addTool("Sleptons.SleptonsConfig.getSElectronRPlusToElectronNeutralino"         , "SElectronRPlusToElectronNeutralino" )
addTool("Sleptons.SleptonsConfig.getSElectronRMinusToElectronNeutralino"        , "SElectronRMinusToElectronNeutralino" )
addTool("Sleptons.SleptonsConfig.getSMuonRPlusToMuonNeutralino"       , "SMuonRPlusToMuonNeutralino" )
addTool("Sleptons.SleptonsConfig.getSMuonRMinusToMuonNeutralino"      , "SMuonRMinusToMuonNeutralino" )
addTool("Sleptons.SleptonsConfig.getSTauLPlusToTauNeutralino"        , "STauLPlusToTauNeutralino" )
addTool("Sleptons.SleptonsConfig.getSTauLMinusToTauNeutralino"        , "STauLMinusToTauNeutralino" )
addTool("Sleptons.SleptonsConfig.getSElectronLPlusToElectronNeutralino"       , "SElectronLPlusToElectronNeutralino" )
addTool("Sleptons.SleptonsConfig.getSElectronLMinusToElectronNeutralino"      , "SElectronLMinusToElectronNeutralino" )
addTool("Sleptons.SleptonsConfig.getSMuonLPlusToMuonNeutralino"        , "SMuonLPlusToMuonNeutralino" )
addTool("Sleptons.SleptonsConfig.getSMuonLMinusToMuonNeutralino"       , "SMuonLMinusToMuonNeutralino" )
addTool("Sleptons.SleptonsConfig.getSTauRPlusToTauNeutralino"       , "STauRPlusToTauNeutralino" )
addTool("Sleptons.SleptonsConfig.getSTauRMinusToTauNeutralino"      , "STauRMinusToTauNeutralino" )

#####################################################
#Off shell Tau+Neutralino
#
#####################################################
addTool("Sleptons.SleptonsConfig.getSTauRMinusToPionMinusNeutralino"        , "STauRMinusToPionMinusNeutralino" )
addTool("Sleptons.SleptonsConfig.getSTauRPlusToPionPlusNeutralino"          , "STauRPlusToPionPlusNeutralino" )
addTool("Sleptons.SleptonsConfig.getSTauLMinusToPionMinusNeutralino"        , "STauLMinusToPionMinusNeutralino" )
addTool("Sleptons.SleptonsConfig.getSTauLPlusToPionPlusNeutralino"          , "STauLPlusToPionPlusNeutralino" )

addTool("Sleptons.SleptonsConfig.getSTauRMinusToRhoMinusNeutralino"        , "STauRMinusToRhoMinusNeutralino" )
addTool("Sleptons.SleptonsConfig.getSTauRPlusToRhoPlusNeutralino"          , "STauRPlusToRhoPlusNeutralino" )
addTool("Sleptons.SleptonsConfig.getSTauLMinusToRhoMinusNeutralino"        , "STauLMinusToRhoMinusNeutralino" )
addTool("Sleptons.SleptonsConfig.getSTauLPlusToRhoPlusNeutralino"          , "STauLPlusToRhoPlusNeutralino" )

addTool("Sleptons.SleptonsConfig.getSTauRMinusToEMinusNeutralino"        , "STauRMinusToEMinusNeutralino" )
addTool("Sleptons.SleptonsConfig.getSTauRPlusToEPlusNeutralino"          , "STauRPlusToEPlusNeutralino" )
addTool("Sleptons.SleptonsConfig.getSTauLMinusToEMinusNeutralino"        , "STauLMinusToEMinusNeutralino" )
addTool("Sleptons.SleptonsConfig.getSTauLPlusToEPlusNeutralino"          , "STauLPlusToEPlusNeutralino" )

addTool("Sleptons.SleptonsConfig.getSTauRMinusToMuMinusNeutralino"        , "STauRMinusToMuMinusNeutralino" )
addTool("Sleptons.SleptonsConfig.getSTauRPlusToMuPlusNeutralino"          , "STauRPlusToMuPlusNeutralino" )
addTool("Sleptons.SleptonsConfig.getSTauLMinusToMuMinusNeutralino"        , "STauLMinusToMuMinusNeutralino" )
addTool("Sleptons.SleptonsConfig.getSTauLPlusToMuPlusNeutralino"          , "STauLPlusToMuPlusNeutralino" )

addTool("Sleptons.SleptonsConfig.getSTauRMinusToa1MinusNeutralino"        , "STauRMinusToa1MinusNeutralino" )
addTool("Sleptons.SleptonsConfig.getSTauRPlusToa1PlusNeutralino"          , "STauRPlusToa1PlusNeutralino" )
addTool("Sleptons.SleptonsConfig.getSTauLMinusToa1MinusNeutralino"        , "STauLMinusToa1MinusNeutralino" )
addTool("Sleptons.SleptonsConfig.getSTauLPlusToa1PlusNeutralino"          , "STauLPlusToa1PlusNeutralino" )
