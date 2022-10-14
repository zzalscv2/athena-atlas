/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


// xAOD include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"

// Local include(s):
#include "xAODForward/versions/ZdcModule_v1.h"

namespace xAOD {

   ZdcModule_v1::ZdcModule_v1()
      : SG::AuxElement() {

   }

  // simple types
  //AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(ZdcModule_v1, uint32_t, id,setId) // remove oct 29, 2021
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(ZdcModule_v1,uint32_t, zdcId,setZdcId)
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(ZdcModule_v1,int,zdcSide,setZdcSide)
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(ZdcModule_v1,int,zdcModule,setZdcModule)
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(ZdcModule_v1,int,zdcChannel,setZdcChannel)
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(ZdcModule_v1,int,zdcType,setZdcType)

    
  const std::vector<uint16_t>& ZdcModule_v1::getWaveform(const std::string s) const 
  {
    const Accessor< std::vector<uint16_t> > acc(s);
    return acc( *this );
  }

  void ZdcModule_v1::setWaveform(const std::string s, const std::vector<uint16_t>& waveform)
  {
    const Accessor< std::vector<uint16_t> > acc(s);
    acc( *this ) = waveform;
  }
  

} // namespace xAOD
