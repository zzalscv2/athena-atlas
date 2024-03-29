# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
Configuration database for ISF_SimulationSelectors
Elmar Ritsch, 10/11/2014
"""

from AthenaCommon.CfgGetter import addTool

addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getElectronGeant4Selector"               , "ISF_ElectronGeant4Selector"              )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getMuonGeant4Selector"                   , "ISF_MuonGeant4Selector"                  )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getProtonAFIIGeant4Selector"             , "ISF_ProtonAFIIGeant4Selector"            )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getPionAFIIGeant4Selector"               , "ISF_PionAFIIGeant4Selector"              )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getPionATLFAST3Geant4Selector"         , "ISF_PionATLFAST3Geant4Selector"        )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getPionATLFAST3_QS_Geant4Selector"       , "ISF_PionATLFAST3_QS_Geant4Selector"      )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getProtonATLFAST3Geant4Selector"       , "ISF_ProtonATLFAST3Geant4Selector"      )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getProtonATLFAST3_QS_Geant4Selector"     , "ISF_ProtonATLFAST3_QS_Geant4Selector"    )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getNeutronATLFAST3Geant4Selector"      , "ISF_NeutronATLFAST3Geant4Selector"     )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getNeutronATLFAST3_QS_Geant4Selector"    , "ISF_NeutronATLFAST3_QS_Geant4Selector"   )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getChargedKaonATLFAST3Geant4Selector"  , "ISF_ChargedKaonATLFAST3Geant4Selector" )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getChargedKaonATLFAST3_QS_Geant4Selector", "ISF_ChargedKaonATLFAST3_QS_Geant4Selector" )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getKLongATLFAST3Geant4Selector"        , "ISF_KLongATLFAST3Geant4Selector"       )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getKLongATLFAST3_QS_Geant4Selector"      , "ISF_KLongATLFAST3_QS_Geant4Selector"     )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getChargedKaonAFIIGeant4Selector"        , "ISF_ChargedKaonAFIIGeant4Selector"       )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getKLongAFIIGeant4Selector"              , "ISF_KLongAFIIGeant4Selector"             )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getMuonAFIIGeant4Selector"               , "ISF_MuonAFIIGeant4Selector"              )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getMuonAFII_QS_Geant4Selector"           , "ISF_MuonAFII_QS_Geant4Selector"          )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getMuonFatrasSelector"                   , "ISF_MuonFatrasSelector"                  )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getEtaGreater5ParticleKillerSimSelector" , "ISF_EtaGreater5ParticleKillerSimSelector")
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getDefaultFastCaloSimV2Selector"         , "ISF_DefaultFastCaloSimV2Selector"        )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getDefaultDNNCaloSimSelector"         , "ISF_DefaultDNNCaloSimSelector"        )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getDefaultFatrasSelector"                , "ISF_DefaultFatrasSelector"               )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getDefaultActsSelector"                  , "ISF_DefaultActsSelector"                 )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getDefaultParticleKillerSelector"        , "ISF_DefaultParticleKillerSelector"       )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getDefaultGeant4Selector"                , "ISF_DefaultGeant4Selector"               )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getDefaultAFIIGeant4Selector"            , "ISF_DefaultAFIIGeant4Selector"           )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getDefaultLongLivedGeant4Selector"       , "ISF_DefaultLongLivedGeant4Selector"      )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getDefaultAFII_QS_Geant4Selector"        , "ISF_DefaultAFII_QS_Geant4Selector"       )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getFullGeant4Selector"                   , "ISF_FullGeant4Selector"                  )
addTool("ISF_SimulationSelectors.ISF_SimulationSelectorsConfigLegacy.getPassBackGeant4Selector"               , "ISF_PassBackGeant4Selector"              )
