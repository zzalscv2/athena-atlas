
//  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration



// System include(s):
#include <stdexcept>

// xAOD include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"

// Local include(s):
#include "xAODTrigger/jFexFwdElRoI.h"
#include "getQuadrant.h"

namespace xAOD {

  //eta position in FCAL FPGAs
  const std::vector<int> jFexFwdElRoI_v1::s_FWD_EtaPosition =   {0,  8,   //Region 1 
                                                                 9, 11,   //Region 2
								 12,      //Region 3
								 13, 24};  //Region 4
  //eta position of FCAL EM layer as an integer                                                 
  const std::vector<int> jFexFwdElRoI_v1::s_FCAL_EtaPosition = {32,34,35,37,38,40,41,43,44,46,47,49};


   jFexFwdElRoI_v1::jFexFwdElRoI_v1()
     : SG::AuxElement() {
   }
  void jFexFwdElRoI_v1::initialize(uint8_t jFexNumber, uint8_t fpgaNumber, uint32_t tobWord, char istob, int resolution, float_t eta, float_t phi ) {
 
    setTobWord( tobWord );
    setjFexNumber( jFexNumber );
    setfpgaNumber(fpgaNumber);
    setTobLocalEta( unpackEtaIndex() );
    setTobLocalPhi( unpackPhiIndex() ); 
    setTobEt(unpackEtTOB());
    setTobEMIso(unpackEMIsoTOB());
    setTobEMf1(unpackEMf1TOB());
    setTobEMf2(unpackEMf2TOB());
    setTobSat(unpackSaturationIndex());
    setGlobalEta(getGlobalEta());
    setGlobalPhi(getGlobalPhi());
    setEta( eta );
    setPhi( phi ); 
    setResolution( resolution );
    setIsTOB(istob);
    setEtEMiso(getEtEMiso());
    setEtEM(getEtEM());
    setEtHad1(getEtHad1());
    setEtHad2(getEtHad2());
    
    return;
   }

    int jFexFwdElRoI_v1::menuEta() const {
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
        return index/4; // note: unlike SR and LR jets, apparently this can be signed?
    }

   //----------------
   /// Raw data words
   //----------------

   /// Used to differencite TOBs from xTOBs, once they are implemented for jFEX
    AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexFwdElRoI_v1, char, isTOB, setIsTOB )

   /// Only calculable externally
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexFwdElRoI_v1, uint32_t, tobWord    , setTobWord    )
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexFwdElRoI_v1, uint8_t , jFexNumber , setjFexNumber )
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexFwdElRoI_v1, uint8_t , fpgaNumber , setfpgaNumber )  
   
   /// Extracted from data words, stored for convenience
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexFwdElRoI_v1, uint8_t , tobLocalEta , setTobLocalEta )
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexFwdElRoI_v1, uint8_t , tobLocalPhi , setTobLocalPhi )
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexFwdElRoI_v1, uint16_t, tobEt       , setTobEt    )
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexFwdElRoI_v1, uint8_t,  tobEMIso    , setTobEMIso   )
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexFwdElRoI_v1, uint8_t,  tobEMf1     , setTobEMf1  )
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexFwdElRoI_v1, uint8_t,  tobEMf2     , setTobEMf2  )
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexFwdElRoI_v1, uint8_t , tobSat      , setTobSat   )
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexFwdElRoI_v1, int , globalEta, setGlobalEta )
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexFwdElRoI_v1, uint, globalPhi, setGlobalPhi )
   
  ///global coordinates, stored for furture use but not sent to L1Topo    
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexFwdElRoI_v1, float, eta, setEta)
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexFwdElRoI_v1, float, phi, setPhi)

   ///Setting the jFEX ET resolution       
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexFwdElRoI_v1, int  , tobEtScale, setResolution)

   ///Additional variabes from xTob (not available yet) 
    AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexFwdElRoI_v1, uint16_t, EtEMiso    , setEtEMiso    )
    AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexFwdElRoI_v1, uint16_t, EtEM       , setEtEM    )   
    AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexFwdElRoI_v1, uint16_t, EtHad1     , setEtHad1    )
    AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexFwdElRoI_v1, uint16_t, EtHad2     , setEtHad2    )

   //-----------------
   /// Methods- to decode data from the TOB/RoI and return to the user
   //-----------------

  

   //Hardware coordinate elements  

   //Raw ET on TOB scale (200 MeV/count)
    unsigned int jFexFwdElRoI_v1::unpackEtTOB() const{
     return (tobWord() >> s_etBit) & s_etMask;
    }
    
    //Return the isolation bit
    unsigned int jFexFwdElRoI_v1::unpackEMIsoTOB() const{
     return (tobWord() >> s_isoBit) & s_isoMask;
    } 

    //Return the emfraction1 bit
    unsigned int jFexFwdElRoI_v1::unpackEMf1TOB() const{
      return (tobWord() >> s_emf1Bit) & s_emf1Mask;
    }
  
    //Return the emfraction2 bit
    unsigned int jFexFwdElRoI_v1::unpackEMf2TOB() const{
      return (tobWord() >> s_emf2Bit) & s_emf2Mask;
    } 

   //Return an eta index
   unsigned int jFexFwdElRoI_v1::unpackEtaIndex() const {
     return (tobWord() >> s_etaBit) & s_etaMask;
   }
   //Return a phi index
   unsigned int jFexFwdElRoI_v1::unpackPhiIndex() const {
     return (tobWord() >> s_phiBit) & s_phiMask;
   }

   //Return sat flag
   unsigned int jFexFwdElRoI_v1::unpackSaturationIndex() const{
     return (tobWord() >> s_satBit) & s_satMask;
   }

   /// Methods that require combining results or applying scales

   /// ET on TOB scale
   unsigned int jFexFwdElRoI_v1::et() const {
     //return TOB Et in a 1 MeV scale
     return tobEt()*tobEtScale();
   }

  /// could add iso, emf1 and emf2 calculated from EtEMiso, EtEM, ETHad1 and EtHad2
   
  //global coords
  
  int jFexFwdElRoI_v1::getGlobalEta() const {

    int globalEta = 0;
    
    if(jFexNumber()==5 ) {
      
      if(tobLocalEta() <=s_FWD_EtaPosition[1]) { //Region 1                                                                   
	globalEta = (tobLocalEta() + (8*(jFexNumber() -3)) );
      }
      else if(tobLocalEta() <=s_FWD_EtaPosition[3]) { //Region 2                                                              
	globalEta = 25 +2*(tobLocalEta()-9);
      }
      else if(tobLocalEta() == s_FWD_EtaPosition[4] ) { //Region 3                                                            
	globalEta = 31;
      }
      else if(tobLocalEta() <= s_FWD_EtaPosition[6]) { //Region 4                                                             
	globalEta = s_FCAL_EtaPosition[tobLocalEta()-13]-1;
      }
      
    }
    else if(jFexNumber()==0) {
      
      if(tobLocalEta() <=s_FWD_EtaPosition[1]) { //Region 1                                                                   
	globalEta = (8-tobLocalEta() + (8*(jFexNumber() -3)) )-1;
      }
      else if(tobLocalEta() <=s_FWD_EtaPosition[3]) { //Region 2                                                              
	globalEta = -27 -2*(tobLocalEta()-9);
      }
      else if(tobLocalEta() == s_FWD_EtaPosition[4] ) { //Region 3                                                            
	globalEta = -32;
      }
      else if(tobLocalEta() <= s_FWD_EtaPosition[6]) { //Region 4                                                             
	globalEta = -s_FCAL_EtaPosition[tobLocalEta()-13];
      }
    }
    else { //Module 1-4                                                                                                         
      globalEta = (tobLocalEta() + (8*(jFexNumber() -3)) );
    }
    
    return globalEta;
  }


  uint jFexFwdElRoI_v1::getGlobalPhi() const {

    uint globalPhi = 0;
    const unsigned int quadrant = ::getQuadrant(fpgaNumber());
    
    //16 is the phi height of an FPGA                                                                                           
    if(jFexNumber() == 0 || jFexNumber() == 5) {
      
      if(tobLocalEta() <=s_FWD_EtaPosition[1]) { //Region 1                                                                   
	globalPhi = tobLocalPhi() + (quadrant * 16);
      }
      else if(tobLocalEta() <=s_FWD_EtaPosition[4]) {//Region 2 and Region 3 have the same granularity                        
	globalPhi = (16*quadrant) + 2*(tobLocalPhi());
      }
      else if(tobLocalEta() <=s_FWD_EtaPosition[6]) {//Region 4                                                               
	globalPhi = (16*quadrant) + 4*(tobLocalPhi())+1;
      }
    }
    else { //Modules 1-4                                                                                                        
      globalPhi = tobLocalPhi() + (quadrant * 16);
    }
    
    return globalPhi;
    
  }
  

  unsigned int jFexFwdElRoI_v1::getEtEMiso() const{
    // to be implemented
    return 0;
  }

  unsigned int jFexFwdElRoI_v1::getEtEM() const{
    // to be implemented  
    return 0;
  }

  unsigned int jFexFwdElRoI_v1::getEtHad1() const{
    // to be implemented  
    return 0;
  }

  unsigned int jFexFwdElRoI_v1::getEtHad2() const{
    // to be implemented  
    return 0;
  }






} // namespace xAOD

