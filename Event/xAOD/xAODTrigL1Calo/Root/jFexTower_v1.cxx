/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "xAODTrigL1Calo/versions/jFexTower_v1.h"

// EDM includes(s):
#include "xAODCore/AuxStoreAccessorMacros.h"

namespace xAOD {

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexTower_v1, float    , eta         , setEta         )
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexTower_v1, float    , phi         , setPhi         )
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexTower_v1, uint8_t  , module      , setModule      )
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexTower_v1, uint8_t  , fpga        , setFpga        )
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexTower_v1, uint8_t  , channel     , setChannel     )
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexTower_v1, uint16_t , towerEt     , setTowerEt     )
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexTower_v1, uint8_t  , jFEXdataID  , setJFEXdataID  )
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexTower_v1, char     , isSaturated , setIsSaturated )
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexTower_v1, uint32_t , jFEXtowerID , setjFEXtowerID )
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexTower_v1, uint8_t  , Calosource  , setCalosource  )

/// initialize
void jFexTower_v1::initialize(const float Eta,const float Phi)
{
    setEta( Eta );
    setPhi( Phi );
}

void jFexTower_v1::initialize(const float Eta,
                              const float Phi,
                              const uint32_t IDSim,
                              const uint8_t source,
                              const uint16_t TowerEt,
                              const uint8_t Module,
                              const uint8_t Fpga,
                              const uint8_t Channel,
                              const uint8_t JFEXdataID,
                              const char IsSaturated)
{
    setEta( Eta );
    setPhi( Phi );
    setTowerEt( TowerEt );
    setModule( Module );
    setFpga( Fpga );
    setChannel( Channel );
    setJFEXdataID( JFEXdataID );
    setIsSaturated( IsSaturated );
    setjFEXtowerID( IDSim );
    setCalosource( source );
}

} // namespace xAOD
