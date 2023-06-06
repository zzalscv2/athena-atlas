/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "xAODTrigL1Calo/versions/jFexTower_v1.h"

// EDM includes(s):
#include "xAODCore/AuxStoreAccessorMacros.h"

namespace xAOD {


// SETTER and GETTERs in the AuxContainer
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexTower_v1, float        , eta         , setEta         )
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexTower_v1, float        , phi         , setPhi         )
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexTower_v1, int          , globalEta   , setglobalEta   )
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexTower_v1, unsigned int , globalPhi   , setglobalPhi   )
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexTower_v1, uint8_t      , module      , setModule      )
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexTower_v1, uint8_t      , fpga        , setFpga        )
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexTower_v1, uint8_t      , channel     , setChannel     )
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexTower_v1, uint8_t      , jFEXdataID  , setJFEXdataID  )
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexTower_v1, uint32_t     , jFEXtowerID , setjFEXtowerID )
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexTower_v1, uint8_t      , Calosource  , setCalosource  )

AUXSTORE_OBJECT_SETTER_AND_GETTER   ( jFexTower_v1, std::vector<uint16_t> , et_count , setEt_count )
AUXSTORE_OBJECT_MOVE                ( jFexTower_v1, std::vector<uint16_t> , et_count , setEt_count )

AUXSTORE_OBJECT_SETTER_AND_GETTER   ( jFexTower_v1, std::vector<char> , isjTowerSat , setIsjTowerSat )
AUXSTORE_OBJECT_MOVE                ( jFexTower_v1, std::vector<char> , isjTowerSat , setIsjTowerSat )

// SETTER and GETTERs for the decorations
AUXSTORE_OBJECT_SETTER_AND_GETTER   ( jFexTower_v1, std::vector<float> , SCellEt , setSCellEt )
AUXSTORE_OBJECT_MOVE                ( jFexTower_v1, std::vector<float> , SCellEt , setSCellEt )
AUXSTORE_OBJECT_SETTER_AND_GETTER   ( jFexTower_v1, std::vector<int>   , SCellID , setSCellID )
AUXSTORE_OBJECT_MOVE                ( jFexTower_v1, std::vector<int>   , SCellID , setSCellID )
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( jFexTower_v1, int                , TileEt  , setTileEt  )


/// initialize
void jFexTower_v1::initialize(const float Eta,const float Phi)
{
    setEta( Eta );
    setPhi( Phi );
}

void jFexTower_v1::initialize(const float Eta,
                              const float Phi,
                              const int globaleta,
                              const unsigned int globalphi,
                              const uint32_t IDSim,
                              const uint8_t source,
                              const std::vector<uint16_t>& Et_count,
                              const uint8_t Module,
                              const uint8_t Fpga,
                              const uint8_t Channel,
                              const uint8_t JFEXdataID,
                              const std::vector<char>& IsjTowerSat)
{
    setEta( Eta );
    setPhi( Phi );
    setglobalEta( globaleta );
    setglobalPhi( globalphi );
    setModule( Module );
    setFpga( Fpga );
    setChannel( Channel );
    setJFEXdataID( JFEXdataID );
    setIsjTowerSat( IsjTowerSat );
    setjFEXtowerID( IDSim );
    setCalosource( source );
    setEt_count( Et_count );
}

/// Returns the Et of the Tower (in counts! it needs conversion)
uint16_t jFexTower_v1::jTowerEt() const {
    
    if(et_count().size() == 1){
        return et_count().at(0);
    }
    return 0;
}

bool jFexTower_v1::isCore() const{
    // FPGA eta bounderies 
    const float eta_edge[6] = { -1.6, -0.8, 0, 0.8, 1.6, 5};
    const float phi_edge[4] = { 0.5*M_PI, M_PI, 1.5*M_PI, 2*M_PI};
    
    int cal_jfex = -1; 
    int cal_fpga = -1; 
    
    
    // finding the jFEX module
    for(unsigned int leta=0; leta<6; leta++){
        if(eta() < eta_edge[leta] ){
            cal_jfex = leta;
            break;
        }
    }
    
    // converts phi to [0,2pi]
    float mphi = phi() < 0 ? 2*M_PI+phi() : phi();
    
    // finding FPGA number
    for(unsigned int lphi=0; lphi<4; lphi++){
        if(mphi < phi_edge[lphi] ){
            cal_fpga = lphi;
            break;
        }
    }
    
    // correcting the FPGA number to match the firmware scheme FPGA (U4) 2 -> 3 and FPGA (U3) 3 -> 2
    cal_fpga = cal_fpga == 2 ? 3 : cal_fpga == 3 ? 2 : cal_fpga;
    
    return (module() == cal_jfex and fpga() == cal_fpga);
}

} // namespace xAOD
