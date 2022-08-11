/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRIGL1CALO_VERSIONS_GFEXTOWER_V1_H
#define XAODTRIGL1CALO_VERSIONS_GFEXTOWER_V1_H

// EDM include(s):
#include "AthLinks/ElementLink.h"
#include "AthContainers/AuxElement.h"

// System include(s):
#include <stdint.h>

// ROOT include(s):
#include "Math/Vector4D.h"

namespace xAOD {

   /// Class describing input data of a LVL1 eFEX
   //  in the xAOD format.
 
    class gFexTower_v1 : public SG::AuxElement{
    public:

      /// Inherit all of the base class's constructors 
      using SG::AuxElement::AuxElement;

      /// @brief The pseudorapidity (\f$\eta\f$)
      uint8_t iEta() const; /// getter for integer eta index (0-39)
      void setEta(uint8_t); /// setter for the above

      /// @brief The azimuthal angle (\f$\phi\f$)     
      uint8_t iPhi() const; /// Getter for integer phi index [0-31] inclusive
      void setPhi(uint8_t); /// setter for the above

      /// get fpga number
      uint8_t fpga() const; /// getter for the fpga number [0-2] inclusive
      ///  set fpga number
      void setFpga(uint8_t); /// setter for the above
      
      /// get Energy Value
      uint16_t et() const; /// getter for the energy value
      /// set Energy Value
      void setEt(uint16_t); /// setter for the above

      /// Is gTower saturated?
      char isSaturated() const; /// getter for the saturation flag of gTower
      /// set saturation flag of gTower
      void setIsSaturated(char); ///setter for the above

      void initialize(const uint8_t Eta,const uint8_t Phi);
      void initialize(const uint8_t Eta,const uint8_t Phi,
                                   const uint16_t Et,
                                   const uint8_t Fpga,
				   const char IsSaturated);

    
    private:
        
  }; // class gFexTower_v1
} // namespace xAOD

// Declare the inheritance of the type:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::gFexTower_v1, SG::AuxElement );
#endif // XAODTRIGL1CALO_VERSIONS_GFEXTOWER_V1_H
