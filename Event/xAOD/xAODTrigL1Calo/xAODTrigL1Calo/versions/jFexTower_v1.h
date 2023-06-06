/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRIGL1CALO_VERSIONS_JFEXTOWER_V1_H
#define XAODTRIGL1CALO_VERSIONS_JFEXTOWER_V1_H

// EDM include(s):
#include "AthLinks/ElementLink.h"
#include "AthContainers/AuxElement.h"

// System include(s):
#include <stdint.h>

// ROOT include(s):
#include "Math/Vector4D.h"

namespace xAOD {

   /// Class describing input data of a LVL1 jFEX
   //  in the xAOD format.
 
    class jFexTower_v1 : public SG::AuxElement{
    public:

      /// Inherit all of the base class's constructors 
      using SG::AuxElement::AuxElement;

      /// @brief The pseudorapidity (\f$\eta\f$)
      float eta() const; /// getter for the global eta value (float)
      void setEta(float); /// setter for the above
      
      int globalEta() const; /// getter for the global eta value (int)
      void setglobalEta(int); /// setter for the above

      /// @brief The azimuthal angle (\f$\phi\f$)     
      float phi() const; /// getter for the global phi value (float)
      void setPhi(float); /// setter for the above
      
      unsigned int globalPhi() const; /// getter for the global phi value (int)
      void setglobalPhi(unsigned int); /// setter for the above

      /// get module number
      uint8_t module() const; /// getter for the module number [0-5] inclusive
      /// set module number
      void setModule(uint8_t); /// setter for the above

      /// get fpga number
      uint8_t fpga() const; /// getter for the fpga number [0-3] inclusive
      ///  set fpga number
      void setFpga(uint8_t); /// setter for the above

      /// get channel number
      uint8_t channel() const; /// getter for the channel number [0-59]
      ///  set fpga number
      void setChannel(uint8_t); /// setter for the above
      
      /// get Energy Counts
      const std::vector<uint16_t>& et_count() const; /// getter for the 11 energy counts
      /// set Energy Counts
      void setEt_count(const std::vector<uint16_t>&); /// setter for the above
      void setEt_count(std::vector<uint16_t>&&); /// setter for the above      
      

      /// get the location where Et is placed in the data stream
      uint8_t jFEXdataID() const; /// getter for the location in data stream where Et is placed [0-15]
      /// set the location where Et is placed in the data stream
      void setJFEXdataID(uint8_t); /// setter for the above

      /// Is the Tower saturated?
      const std::vector<char>& isjTowerSat() const; /// getter for the saturation flag of jTower
      /// set saturation flag of the Tower
      void setIsjTowerSat(const std::vector<char>&); ///setter for the above
      void setIsjTowerSat(std::vector<char>&&); ///setter for the above

      /// get the Simulation ID
      uint32_t jFEXtowerID() const; /// getter for the jtower simulation ID
      /// set the simulation ID
      void setjFEXtowerID(uint32_t); ///setter for the above

      /// Information about the calorimeter source
      /// Calosource values:  0: barrel, 1: tile, 2: emec, 3: hec, 4: fcal1, 5: fcal2, 6: fcal3
      uint8_t Calosource() const; /// getter for the calorimeter source
      /// set calorimeter source
      void setCalosource(uint8_t); ///setter for the above
      
      //Returns the Et of the Tower
      uint16_t jTowerEt() const;
      
      //Returns true if the Tower belongs to the FPGA core area
      bool isCore() const;

      // Decorated variables
      
      /// Information about SCell Ets
      const std::vector<float>& SCellEt() const; /// getter for Aux SCell Et
      void setSCellEt(const std::vector<float>&); /// setter for the above
      void setSCellEt(std::vector<float>&&); /// setter for the above  
      
      /// Information about SCell IDs
      const std::vector<int>& SCellID() const; /// getter for Aux SCell IDs
      void setSCellID(const std::vector<int>&); /// setter for the above
      void setSCellID(std::vector<int>&&); /// setter for the above  
      
      /// Information about Tile Et
      int TileEt() const; /// getter for Aux Tile Et
      void setTileEt(int); /// setter for the above
      



      void initialize(  const float Eta,const float Phi);
      void initialize(  const float Eta,const float Phi,const int globaleta,const unsigned int globalphi,
                        const uint32_t IDsim,
                        const uint8_t source,
                        const std::vector<uint16_t>& Et_count,
                        const uint8_t Module,
                        const uint8_t Fpga,
                        const uint8_t Channel,
                        const uint8_t JFEXdataID,
                        const std::vector<char>& IsjTowerSat);

    
    private:
        
  }; // class jFexTower_v1
} // namespace xAOD

// Declare the inheritance of the type:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::jFexTower_v1, SG::AuxElement );
#endif // XAODTRIGL1CALO_VERSIONS_JFEXTOWER_V1_H
