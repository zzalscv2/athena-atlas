
/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/


// System include(s):
#include <stdexcept>

// xAOD include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"

// Local include(s):
#include "xAODTrigger/versions/gFexGlobalRoI_v1.h"

namespace xAOD {

  /// Constants used in converting to ATLAS units
  const float gFexGlobalRoI_v1::s_tobEtScale = 200.; 
 

   gFexGlobalRoI_v1::gFexGlobalRoI_v1()
      : SG::AuxElement() {

   }

   void gFexGlobalRoI_v1::initialize( uint32_t word ) {

      setWord( word );
      setGlobalType(unpackType());
      setQuantityOne( unpackQuantityOneIndex() );
      setQuantityTwo( unpackQuantityTwoIndex() );
      setStatusOne( unpackStatusOne());
      setStatusTwo( unpackStatusTwo());
      setSaturated(unpackSaturated());

   }


   /// Raw data words
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexGlobalRoI_v1, uint32_t, word,
                                         setWord )
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexGlobalRoI_v1, uint8_t, statusOne,
                                         setStatusOne )
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexGlobalRoI_v1, uint8_t, statusTwo,
                                         setStatusTwo )
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexGlobalRoI_v1, uint8_t, saturated,
                                         setSaturated )
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexGlobalRoI_v1, int, globalType,
                                         setGlobalType )
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexGlobalRoI_v1, uint16_t, quantityOne,
                                         setQuantityOne )
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexGlobalRoI_v1, uint16_t, quantityTwo,
                                         setQuantityTwo )


   /// Methods to decode data from the TOB/RoI and return to the user


   /// Object disambiguation ()
   int gFexGlobalRoI_v1::unpackType() const {
     auto tobID = (word() >> s_tobIDBit) & s_tobIDMask;
     if (tobID == 0 )  return gNull;
     else if (tobID == 1 )  return gScalar; /// scalar values (MET, SumET)
     else if (tobID == 2 )  return gMET; /// MET components (METx, METy)
     else if (tobID == 3 )  return gMHT; /// MET hard term components (MHTx, MHTy)
     else if (tobID == 4 )  return gMST; /// MET soft term components (MSTx, MSTy)
     else return -999;
   }

   bool gFexGlobalRoI_v1::isgScalar() const {
     return globalType() == gScalar;
   }

   bool gFexGlobalRoI_v1::isgMET() const {
     return globalType() == gMET;
   }

   bool gFexGlobalRoI_v1::isgMHT() const {
     return globalType() == gMHT;
   }

   bool gFexGlobalRoI_v1::isgMST() const {
     return globalType() == gMST;
   }



   unsigned int gFexGlobalRoI_v1::unpackStatusOne() const{
    return (word() >> s_statusOneBit) & s_statusOneMask;
   }

   unsigned int gFexGlobalRoI_v1::unpackStatusTwo() const{
    return (word() >> s_statusTwoBit) & s_statusTwoMask;
   }

   unsigned int gFexGlobalRoI_v1::unpackSaturated() const{
    return (word() >> s_saturBit) & s_saturMask;
   }
    

   /// Raw quantity on TOB scale (3200 MeV/count)
   unsigned int gFexGlobalRoI_v1::unpackQuantityOneIndex() const {
    // Data content = TOB
    return (word() >> s_quantityOneBit) & s_quantityOneMask;
   
   }

   unsigned int gFexGlobalRoI_v1::unpackQuantityTwoIndex() const {
    // Data content = TOB
    return (word() >> s_quantityTwoBit) & s_quantityTwoMask;
   
   }


   /// Methods that require combining results or applying scales

   /// MET/SumEt on TOB scale
   float gFexGlobalRoI_v1::METquantityOne() const {
    if (globalType() != gNull){
        return quantityOne()*s_tobEtScale;
    }
    return -999;
   }

   float gFexGlobalRoI_v1::METquantityTwo() const {
    if (globalType() != gNull){
        return quantityTwo()*s_tobEtScale;
    }
    return -999;
   }

   float gFexGlobalRoI_v1::SumEt() const {
    if (globalType() == gScalar ){
        return quantityTwo()*s_tobEtScale;
    }
    return -999;
   }

   


   



} // namespace xAOD
