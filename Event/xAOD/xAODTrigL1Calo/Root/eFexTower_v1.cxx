/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// EDM includes(s):
#include "xAODCore/AuxStoreAccessorMacros.h"

// Local include(s):
#include "xAODTrigL1Calo/versions/eFexTower_v1.h"

namespace xAOD{  

  AUXSTORE_OBJECT_SETTER_AND_GETTER( eFexTower_v1 , std::vector<uint16_t> , et_count , setEt_count )
  AUXSTORE_OBJECT_MOVE( eFexTower_v1 , std::vector<uint16_t> , et_count , setEt_count )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( eFexTower_v1, float , eta , setEta )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( eFexTower_v1, float , phi , setPhi )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( eFexTower_v1, uint8_t , module , setModule )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( eFexTower_v1, uint8_t , fpga , setFpga )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( eFexTower_v1, uint32_t , em_status , setEm_status )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( eFexTower_v1, uint32_t , had_status , setHad_status ) 
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( eFexTower_v1, uint32_t , eFEXtowerID , seteFEXtowerID ) 
  
  
  /// initialize
  void eFexTower_v1::initialize(const float Eta,const float Phi)
  {
    setEta( Eta );
    setPhi( Phi );    
  }
  
  void eFexTower_v1::initialize(const float Eta,const float Phi,
                                   const std::vector<uint16_t>& Et_count,
                                   const uint8_t Module,
                                   const uint8_t Fpga,
                                   const uint32_t Em_status,
				   const uint32_t Had_status)
  {
    setEta( Eta );
    setPhi( Phi );
    setEt_count( Et_count );
    setModule( Module );
    setFpga( Fpga );
    setEm_status( Em_status );
    setHad_status( Had_status );
  }

  int32_t eFexTower_v1::id() const {
      int etaIndex = int( (eta()+0.025)*10 ) + (((eta()+0.025)<0) ? -1 : 1); // runs from -25 to 25 (excluding 0)
      int phiIndex = int( (phi()+0.025)*32./ROOT::Math::Pi() ) + ((phi()+0.025)<0 ? 63 : 0); // runs from 0 to 63
      int modIndex = ( module()>23 ) ? 99 : module(); // module runs from 0 to 23 or otherwise takes value 99
      int fpgaIndex = ( fpga() > 3 ) ? 9 : fpga(); //from runs from 0 to 3 or otherwise takes value 9
      return (std::abs(etaIndex)*100000 + phiIndex*1000 + modIndex*10 +fpgaIndex)*(etaIndex<0 ? -1 : 1);
  }

} // namespace xAOD
