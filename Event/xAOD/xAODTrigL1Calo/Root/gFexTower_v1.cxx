/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "xAODTrigL1Calo/versions/gFexTower_v1.h"

// EDM includes(s):
#include "xAODCore/AuxStoreAccessorMacros.h"

namespace xAOD{  

  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexTower_v1, uint8_t , iEta , setiEta )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexTower_v1, uint8_t , iPhi , setiPhi )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexTower_v1, float   , eta  , setEta )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexTower_v1, float   , phi  , setPhi )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexTower_v1, uint8_t , fpga , setFpga )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexTower_v1, int , towerEt , setTowerEt )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexTower_v1, char , isSaturated , setIsSaturated )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexTower_v1, uint32_t     , gFEXtowerID , setgFEXtowerID )
  
  /// initialize
  void gFexTower_v1::initialize(const uint8_t IEta,const uint8_t IPhi)
  {
    setiEta( IEta );
    setiPhi( IPhi );    
  }
  
  void gFexTower_v1::initialize(const uint8_t IEta,
                                const uint8_t IPhi,
                                const float Eta,
                                const float Phi,                                
                                const int TowerEt,
                                const uint8_t Fpga,
                                const char IsSaturated)
  {
    setiEta( IEta );
    setiPhi( IPhi );
    setEta( Eta );
    setPhi( Phi );
    setFpga( Fpga );
    setIsSaturated( IsSaturated );
    setTowerEt( TowerEt );

  }

} // namespace xAOD
