# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

"""
Configuration database for ISF
Elmar Ritsch, 10/11/2014
"""

from AthenaCommon.CfgGetter import addTool, addService

addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getInDetTrackingGeometryBuilder",            "ISF_InDetTrackingGeometryBuilder")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasCaloTrackingGeometryBuilder",       "ISF_FatrasCaloTrackingGeometryBuilder")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasMuonTrackingGeometryBuilder",       "ISF_FatrasMuonTrackingGeometryBuilder")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasGeometryBuilder",                   "ISF_FatrasGeometryBuilder")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasNavigator",                         "ISF_FatrasNavigator")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasChargedPropagator",                 "ISF_FatrasChargedPropagator")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasSTEP_Propagator",                   "ISF_FatrasSTEP_Propagator")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasG4HadIntProcessor",                 "ISF_FatrasG4HadIntProcessor")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasParametricHadIntProcessor",         "ISF_FatrasParametricHadIntProcessor")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasBetheHeitlerEnergyLossUpdator",     "ISF_FatrasBetheHeitlerEnergyLossUpdator")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasEnergyLossUpdator",                 "ISF_FatrasEnergyLossUpdator")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasEnergyLossSamplerBetheHeitler",     "ISF_FatrasEnergyLossSamplerBetheHeitler")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasEnergyLossSamplerBetheBloch",       "ISF_FatrasEnergyLossSamplerBetheBloch")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasMultipleScatteringUpdator",         "ISF_FatrasMultipleScatteringUpdator")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasMultipleScatteringSamplerHighland", "ISF_FatrasMultipleScatteringSamplerHighland")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasMultipleScatteringSamplerGaussianMixture", "ISF_FatrasMultipleScatteringSamplerGaussianMixture")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasMultipleScatteringSamplerGeneralMixture", "ISF_FatrasMultipleScatteringSamplerGeneralMixture")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasPhysicsValidationTool",             "ISF_FatrasPhysicsValidationTool")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasProcessSamplingTool",               "ISF_FatrasProcessSamplingTool")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasMaterialUpdator" ,                  "ISF_FatrasMaterialUpdator")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasExtrapolator",                      "ISF_FatrasExtrapolator")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasConversionCreator",                 "ISF_FatrasConversionCreator")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasHitCreatorPixel",                   "ISF_FatrasHitCreatorPixel")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasHitCreatorSCT",                     "ISF_FatrasHitCreatorSCT")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasHitCreatorTRT",                     "ISF_FatrasHitCreatorTRT")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasSimHitCreatorMS",                   "ISF_FatrasSimHitCreatorMS")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasSimHitCreatorID",                   "ISF_FatrasSimHitCreatorID")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasPdgG4Particle",                     "ISF_FatrasPdgG4Particle")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasParticleDecayHelper",               "ISF_FatrasParticleDecayHelper")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasKinematicFilter",                   "ISF_FatrasKinematicFilter")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasSimTool",                           "ISF_FatrasSimTool")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasPileupSimTool",                     "ISF_FatrasPileupSimTool")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getG4RunManagerHelper",                      "ISF_G4RunManagerHelper")

addService("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasTrackingGeometrySvc",            "ISF_FatrasTrackingGeometrySvc")
addService("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasSimServiceID",                   "ISF_FatrasSimSvc")
addService("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasPileupSimServiceID",             "ISF_FatrasPileupSimSvc")

addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasPileupHitCreatorPixel",             "ISF_FatrasPileupHitCreatorPixel")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasPileupHitCreatorSCT",               "ISF_FatrasPileupHitCreatorSCT")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasPileupHitCreatorTRT",               "ISF_FatrasPileupHitCreatorTRT")
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasPileupSimHitCreatorID",             "ISF_FatrasPileupSimHitCreatorID")

addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasSimulatorToolST"                  , "ISF_FatrasSimulatorToolST"                    )
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasSimulatorTool"                    , "ISF_FatrasSimulatorTool"                    )
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasPileupSimulatorToolST"            , "ISF_FatrasPileupSimulatorToolST"                )
addTool("ISF_FatrasServices.ISF_FatrasServicesConfigLegacy.getFatrasPileupSimulatorTool"              , "ISF_FatrasPileupSimulatorTool"                )
