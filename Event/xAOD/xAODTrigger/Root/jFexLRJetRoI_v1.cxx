/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


// System include(s):
#include <stdexcept>

// xAOD include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"

// Local include(s):
#include "xAODTrigger/jFexLRJetRoI.h"
#include "getQuadrant.h"

namespace xAOD {

  // globalEta/Phi calculation in the FCAL varies depending on position in eta space due to TT granularity change.
  //| Region          |      eta region     | TT (eta x phi)
  //---------------------------------------------------------
  // Region 1  EMB    |      |eta| <  25    | (1 x 1)
  // Region 2  EMIE   | 25 < |eta| < 31     | (2 x 2)
  // Region 3  TRANS  | 31 < |eta| < 32     | (1 x 2)
  // Region 4  FCAL   |      |eta| > 32     | (2 x 4)                 

  //eta position in FCAL FPGAs
  const std::vector<int> jFexLRJetRoI_v1::s_FWD_EtaPosition  =   {0,  8,   //Region 1 
                                                                  9, 11,   //Region 2
                                                                     12,   //Region 3
                                                                 13, 23};  //Region 4

    //eta position of FCAL EM layer as an integer
    //Needs to be modified with firmware values
    const std::vector<int> jFexLRJetRoI_v1::s_FCAL_EtaPosition = {32,34,35,37,38,40,41,43,44,46,47,49};
    
 
   jFexLRJetRoI_v1::jFexLRJetRoI_v1()
     : SG::AuxElement() {
   }
   void jFexLRJetRoI_v1::initialize(uint8_t jFexNumber, uint8_t fpgaNumber, uint32_t tobWord, char istob, int resolution, float_t eta, float_t phi ) {
 
     setTobWord( tobWord );
     setjFexNumber( jFexNumber );
     setfpgaNumber(fpgaNumber);
     setTobEt(unpackEtTOB());
     setTobLocalEta( unpackEtaIndex() );
     setTobLocalPhi( unpackPhiIndex() );
     setTobSat(unpackSaturationIndex());
     setGlobalEta(unpackGlobalEta());
     setGlobalPhi(unpackGlobalPhi());
     setEta( eta );
     setPhi( phi );
     setResolution( resolution ); 
     setIsTOB(istob);

     return;
   }

    int jFexLRJetRoI_v1::menuEta() const {
        // adapted from TSU::toTopoInteger
        static const unsigned int RESOLUTION = 40;
        float tmp = eta()*RESOLUTION;
        int index;
        if ( (abs(tmp)-0.5)/2. == std::round((abs(tmp)-0.5)/2.) ) {
            if ( tmp>0 ) { index = std::floor(tmp); }
            else { index = std::ceil(tmp); }
        } else {
            index = std::round(tmp);
        }
        return index/4;
    }

   //----------------
   /// Raw data words
   //----------------

   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexLRJetRoI_v1, uint32_t, tobWord     , setTobWord    )
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexLRJetRoI_v1, uint8_t , jFexNumber  , setjFexNumber )
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexLRJetRoI_v1, uint8_t , fpgaNumber  , setfpgaNumber )   
   
   /// Used to differencite TOBs from xTOBs
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexLRJetRoI_v1, char, isTOB, setIsTOB )   

   /// Extracted from data words
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexLRJetRoI_v1, uint16_t, tobEt       , setTobEt       )
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexLRJetRoI_v1, uint8_t , tobLocalEta , setTobLocalEta )
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexLRJetRoI_v1, uint8_t , tobLocalPhi , setTobLocalPhi )
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexLRJetRoI_v1, uint8_t , tobSat      , setTobSat      )
 
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexLRJetRoI_v1, int , globalEta, setGlobalEta )
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexLRJetRoI_v1, uint, globalPhi, setGlobalPhi )
   
  ///global coordinates, stored for furture use but not sent to L1Topo    
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexLRJetRoI_v1, float, eta, setEta)
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexLRJetRoI_v1, float, phi, setPhi)

   ///Setting the jFEX ET resolution
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexLRJetRoI_v1, int  , tobEtScale, setResolution)
   
   //-----------------
   //
   //Methods to decode data from the TOB/RoI and return to the user
   //-----------------

  //include in future when xTOB in jFEX has been implemented.
   
   /// TOB or xTOB?
   //jFexLRJetRoI_v1::ObjectType jFexLRJetRoI_v1::type() const {
   //if (Word1() == 0) return TOB;
   //else              return xTOB;
   //}

   //Hardware coordinate elements  

   //Raw ET on TOB scale (200 MeV/count)
    unsigned int jFexLRJetRoI_v1::unpackEtTOB() const{
     //Data content = TOB
     return (tobWord() >> s_etBit) & s_etMask;

    } 

   //Return an eta index
   unsigned int jFexLRJetRoI::unpackEtaIndex() const {
     return (tobWord() >> s_etaBit) & s_etaMask;
   }
   //Return a phi index
   unsigned int jFexLRJetRoI::unpackPhiIndex() const {
     return (tobWord() >> s_phiBit) & s_phiMask;
   }

   //Return sat flag
   unsigned int jFexLRJetRoI::unpackSaturationIndex() const{
     return (tobWord() >> s_satBit) & s_satMask;
   }

   /// Methods that require combining results or applying scales

   /// ET on TOB scale
   unsigned int jFexLRJetRoI_v1::et() const {
    //Return the TOB Et in a 1 MeV scale
     return tobEt()*tobEtScale();
   }

  // Global coords
  // It has been decided that jFEX Large-R jets are going up to 2.5 so like jFEX Taus
    int jFexLRJetRoI_v1::unpackGlobalEta() const {
        
        int globalEta = 0;
        if(jFexNumber()<3){
            globalEta= 8*(jFexNumber()-2) - (tobLocalEta()+1);
        }
        else if(jFexNumber()<6){
            globalEta= 8*(jFexNumber()-3) + (tobLocalEta());
        }
        return globalEta;
    }

    // In the region of |eta|<2.5 there is not different granularity
    uint jFexLRJetRoI_v1::unpackGlobalPhi() const {
        const unsigned int quadrant = ::getQuadrant(fpgaNumber());
        
        uint globalPhi = tobLocalPhi() + (quadrant * 16);
        return globalPhi;
    }


} // namespace xAOD


