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
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexTower_v1, uint16_t , towerEt , setTowerEt )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexTower_v1, char , isSaturated , setIsSaturated )
  
  /// initialize
  void gFexTower_v1::initialize(const uint8_t Eta,const uint8_t Phi)
  {
    setEta( Eta );
    setPhi( Phi );    
  }
  
  void gFexTower_v1::initialize(const uint8_t Eta,const uint8_t Phi,
                                   const uint16_t TowerEt,
                                   const uint8_t Fpga,
				   const char IsSaturated)
  {
    setEta( Eta );
    setPhi( Phi );
    setTowerEt( TowerEt );
    setFpga( Fpga );
    setIsSaturated( IsSaturated );
  }

} // namespace xAOD
