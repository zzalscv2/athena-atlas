/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "xAODTrigL1Calo/versions/gFexTower_v1.h"

// EDM includes(s):
#include "xAODCore/AuxStoreAccessorMacros.h"

namespace xAOD{  

  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexTower_v1, uint8_t , iEta , setEta )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexTower_v1, uint8_t , iPhi , setPhi )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexTower_v1, uint8_t , fpga , setFpga )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexTower_v1, int16_t , towerEt , setTowerEt )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexTower_v1, char , isSaturated , setIsSaturated )
  
  /// initialize
  void gFexTower_v1::initialize(const uint8_t Eta,const uint8_t Phi)
  {
    setEta( Eta );
    setPhi( Phi );    
  }
  
  void gFexTower_v1::initialize(const uint8_t Eta,
                                const uint8_t Phi,
                                const int16_t TowerEt,
                                const uint8_t Fpga,
                                const char IsSaturated)
  {
    setEta( Eta );
    setPhi( Phi );
    setFpga( Fpga );
    setIsSaturated( IsSaturated );
    setTowerEt( signedEt(TowerEt) );

  }

  // Extract TowerEt with sign 
  int16_t gFexTower_v1::signedEt(int16_t TowerEt) const {
    int16_t signedTowerEt = TowerEt; 
    int SIGNMASK = 0x0800;
    int EXTENDS =  0xF000;
    if((SIGNMASK & signedTowerEt)) {
      signedTowerEt = (EXTENDS | signedTowerEt); 
    }
    return signedTowerEt; 
  }


} // namespace xAOD
