/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                                  jFEXFormTOBs.cxx
//                              -------------------
//     begin                : 11 08 2022
//     email                : sergi.rodriguez@cern.ch
//  ***************************************************************************/

#include "L1CaloFEXSim/jFEXFormTOBs.h"
#include "L1CaloFEXSim/FEXAlgoSpaceDefs.h"

namespace LVL1 {

// default constructor for persistency

jFEXFormTOBs::jFEXFormTOBs(const std::string& type, const std::string& name, const IInterface* parent):
    AthAlgTool(type, name, parent)
{
    declareInterface<IjFEXFormTOBs>(this);
}

/** Desctructor */
jFEXFormTOBs::~jFEXFormTOBs() {}

StatusCode jFEXFormTOBs::initialize()
{
    return StatusCode::SUCCESS;
}


uint32_t jFEXFormTOBs::formTauTOB(int iPhi, int iEta, int EtClus, int IsoRing, int Resolution, int ptMinToTopo )
{
    uint32_t tobWord = 0;
    
    int eta = iEta-8; // needed to substract 8 to be in the FPGA core area
    int phi = iPhi-8; // needed to substract 8 to be in the FPGA core area
    int sat = 0; //1 bit for saturation flag, not coded yet

    unsigned int et = EtClus/Resolution;
    if (et > 0x7ff) { //0x7ff is 11 bits
        ATH_MSG_DEBUG("Et saturated: " << et );
        et = 0x7ff;
        sat=1;
    }

    unsigned int iso = IsoRing/Resolution;
    if (iso > 0x7ff) iso = 0x7ff;  //0x7ff is 11 bits

    //create basic tobword with 32 bits
    tobWord = tobWord + (iso << FEXAlgoSpaceDefs::jTau_isoBit) + (et << FEXAlgoSpaceDefs::jTau_etBit) + (eta << FEXAlgoSpaceDefs::jTau_etaBit) + (phi << FEXAlgoSpaceDefs::jTau_phiBit) + sat ;

    ATH_MSG_DEBUG("tobword tau with iso, et, eta and phi: " << std::bitset<32>(tobWord) );


    unsigned int minEtThreshold = ptMinToTopo/Resolution;

    if (et < minEtThreshold) return 0;
    else return tobWord;

}    
    
    


} // end of namespace bracket
