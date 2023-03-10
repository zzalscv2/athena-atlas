/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
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


uint32_t jFEXFormTOBs::formTauTOB(int jFEX, int iPhi, int iEta, int EtClus, int IsoRing, int Resolution, int ptMinToTopo )
{
    uint32_t tobWord = 0;
    
    int eta = iEta-8; // needed to substract 8 to be in the FPGA core area
    int phi = iPhi-8; // needed to substract 8 to be in the FPGA core area
    int sat = 1; //1 bit for saturation flag, not coded yet
    
    // correcting C-side. mirror symmetry
    if(jFEX == 1 || jFEX == 2){
        eta = 15 - iEta;
    }
    else if(jFEX == 0){
        eta = 16 - iEta ;
    }

    unsigned int et = EtClus/Resolution;
    if (et > 0x7ff) { //0x7ff is 11 bits
        ATH_MSG_DEBUG("Et saturated: " << et );
        et = 0x7ff;
    }

    unsigned int iso = IsoRing/Resolution;
    if (iso > 0x7ff) iso = 0x7ff;  //0x7ff is 11 bits

    //create basic tobword with 32 bits
    tobWord = tobWord + (iso << FEXAlgoSpaceDefs::jTau_isoBit) + (et << FEXAlgoSpaceDefs::jTau_etBit) + (eta << FEXAlgoSpaceDefs::jTau_etaBit) + (phi << FEXAlgoSpaceDefs::jTau_phiBit) + sat ;

    ATH_MSG_DEBUG("tobword tau with iso, et, eta and phi: " << std::bitset<32>(tobWord) );


    unsigned int minEtThreshold = ptMinToTopo/Resolution;

    if (et <= minEtThreshold) return 0;
    else return tobWord;

}    

int jFEXFormTOBs::Get_calibrated_SRj_ET(int Energy, int jfex){
    
    int Et_edge[8] = {20,30,40,50,65,80,110,150};
    int et_range = -1;
    
    //checking upper threshold for SRjet energy
    for(int i=0;i<8; i++){
        if(Energy < Et_edge[i] * 1e3){
            et_range = i;
            break;
        }
    }
    
    //the last threshold is inf therefore, if non of the other thresholds is satisfied, the calibration parameter is set to the maximum
    if(et_range<0){
        et_range = 8;
    }
    
    int et = (Energy * FEXAlgoSpaceDefs::SRJ_Calib_params[jfex][et_range]) >> 7;
    return et;
}



uint32_t jFEXFormTOBs::formSRJetTOB(int jFEX, int iPhi, int iEta, int EtClus, int Resolution, int ptMinToTopo ) {
    uint32_t tobWord = 0;
    unsigned int eta = 0;
    unsigned int phi = 0;
    unsigned int jFEXSmallRJetTOBEt = 0;
    int Res = 0; // 11 bits reserved
    int Sat = 1; //  1 bit for saturation. Set to 1 when jet energy is saturated

    if(jFEX == 1 || jFEX == 2) {

        eta = 15 - iEta;
        phi = iPhi - 8;
    }
    if(jFEX == 3 || jFEX == 4) {

        eta = iEta - 8;
        phi = iPhi - 8;
    }
    else if(jFEX == 5) {

        eta = iEta - 8;
        if(iEta < FEXAlgoSpaceDefs::jFEX_algoSpace_A_EMIE_eta) { // ieta lower than EMIE stats -> belong to EMB
            phi = iPhi - 8;
        }
        else if(iEta < FEXAlgoSpaceDefs::jFEX_algoSpace_A_FCAL_start_eta) { // ieta lower than FCAL stats -> belong to EMIE
            phi = iPhi - 4;
        }
        else { // rest ieta belongs to FCAL
            phi = iPhi - 2;
        }
    }
    else if(jFEX == 0) {
        
        eta = 36 - iEta;
        if(iEta < FEXAlgoSpaceDefs::jFEX_algoSpace_C_FCAL_end_eta) { // ieta lower than FCal ends -> FCAL
            phi = iPhi -2 ;
        }
        else if(iEta < FEXAlgoSpaceDefs::jFEX_algoSpace_C_EMIE_end_eta) {// ieta lower than EMIE ends -> EMIE
            phi = iPhi -4 ;
        }
        else {// rest of ieta -> EMB
            phi = iPhi -8 ;
        } 
    }
    
    // COMENTED FOR NOW, Appliying jet calibration
    //jFEXSmallRJetTOBEt = Get_calibrated_SRj_ET(EtClus,jFEX)/Resolution;
    
    //In the firmware the calibration is not applied yet.
    jFEXSmallRJetTOBEt = EtClus/Resolution;
    
    if(jFEXSmallRJetTOBEt > 0x7ff) {
        jFEXSmallRJetTOBEt = 0x7ff;
    }
    //create basic tobword with 32 bits
    tobWord = tobWord + (Res << FEXAlgoSpaceDefs::jJ_resBit) + (jFEXSmallRJetTOBEt << FEXAlgoSpaceDefs::jJ_etBit) + (eta << FEXAlgoSpaceDefs::jJ_etaBit) + (phi << FEXAlgoSpaceDefs::jJ_phiBit)  + (Sat);
    ATH_MSG_DEBUG("tobword smallRJet with res, et, eta and phi: " << std::bitset<32>(tobWord) );
    
    // retrieving the threshold for the TOB Et
    unsigned int minEtThreshold = ptMinToTopo/Resolution;
    
    if (jFEXSmallRJetTOBEt <= minEtThreshold) return 0;
    else return tobWord;
}



uint32_t jFEXFormTOBs::formLRJetTOB(int jFEX, int iPhi, int iEta, int EtClus, int Resolution, int ptMinToTopo ) {
    
    uint32_t tobWord = 0;
    unsigned int eta = 0;
    unsigned int phi = 0;
    unsigned int jFEXLargeRJetTOBEt = 0;
    int Res = 0; // 9 bits reserved
    int Sat = 1; //  1 bit for saturation. Set to 1 when jet energy is saturated

    if(jFEX == 1 || jFEX == 2) {

        eta = 15 - iEta;
        phi = iPhi - 8;
    }
    if(jFEX == 3 || jFEX == 4) {

        eta = iEta - 8;
        phi = iPhi - 8;
    }
    else if(jFEX == 5) {
        eta = iEta -8;

        if(iEta < FEXAlgoSpaceDefs::jFEX_algoSpace_A_EMIE_eta) { // iEta lower than EMIE stats -> belong to EMB
            phi = iPhi-8;
        }
        else if(iEta < FEXAlgoSpaceDefs::jFEX_algoSpace_A_FCAL_start_eta) { // iEta lower than FCAL stats -> belong to EMIE
            phi = iPhi -4;
        }
        else { // rest iEta belongs to FCAL
            phi = iPhi -2;
        }
    }
    else if(jFEX == 0) {
        eta = 36 - iEta;

        if(iEta < FEXAlgoSpaceDefs::jFEX_algoSpace_C_FCAL_end_eta) { // iEta lower than FCal ends -> FCAL
            phi = iPhi -2 ;
        }
        else if(iEta < FEXAlgoSpaceDefs::jFEX_algoSpace_C_EMIE_end_eta) {// iEta lower than EMIE ends -> EMIE
            phi = iPhi -4 ;
        }
        else {// rest of iEta -> EMB
            phi = iPhi -8 ;
        }
    }
    
    jFEXLargeRJetTOBEt = EtClus/Resolution;
    if (jFEXLargeRJetTOBEt > 0x1fff) {
        jFEXLargeRJetTOBEt = 0x1fff;  //0x1fff is 13 bits
    }
    //create basic tobword with 32 bits
    tobWord = tobWord + (Res << FEXAlgoSpaceDefs::jLJ_resBit) + (jFEXLargeRJetTOBEt << FEXAlgoSpaceDefs::jLJ_etBit) + (eta << FEXAlgoSpaceDefs::jLJ_etaBit) + (phi << FEXAlgoSpaceDefs::jLJ_phiBit) + (Sat);
    ATH_MSG_DEBUG("tobword largeRJet with res, et, eta, phi: " << std::bitset<32>(tobWord) );

    
    unsigned int minEtThreshold = ptMinToTopo/Resolution;

    if (jFEXLargeRJetTOBEt <= minEtThreshold) return 0;
    else return tobWord;
}


uint32_t jFEXFormTOBs::formSumETTOB(int ETlow, int EThigh, int Resolution )
{
    uint32_t tobWord = 0;

    int satlow = 0;
    int sathigh = 0;

    unsigned int etlow = ETlow/Resolution;
    if (etlow > 0x7fff) { //0x7fff is 15 bits
        ATH_MSG_DEBUG("sumEtlow saturated: " << etlow );
        etlow = 0x7fff;
        satlow=1;
    }

    unsigned int ethigh = EThigh/Resolution;
    if (ethigh > 0x7fff) { //0x7fff is 15 bits
        ATH_MSG_DEBUG("sumEthigh saturated: " << ethigh );
        ethigh = 0x7fff;
        sathigh=1;
    }

    //create basic tobword with 32 bits
    tobWord = tobWord + (sathigh << FEXAlgoSpaceDefs::jTE_Sat_upperBit) + (ethigh << FEXAlgoSpaceDefs::jTE_Et_upperBit) + (etlow << FEXAlgoSpaceDefs::jTE_Et_lowerBit) + (satlow << FEXAlgoSpaceDefs::jTE_Sat_lowerBit) ;
    ATH_MSG_DEBUG("tobword SumET with Sathigh, EThigh, ETlow and Satlow  : " << std::bitset<32>(tobWord) );

    return tobWord;

}
    
    
uint32_t jFEXFormTOBs::formMetTOB(int METX, int METY, int Resolution ) {
    uint32_t tobWord = 0;

    int sat = 0;
    int res = 0;

    int metX = METX/Resolution;
    int metY = METY/Resolution;

    //0x7fff is 15 bits (decimal value 32767), however as MET is a signed value (can be negative) only 14 bits are allowed (16383) the MSB is the sign
    if (std::abs(metX) > 0x3fff) {
        ATH_MSG_DEBUG("sumEtlow saturated: " << metX );
        metX = 0x7fff;
        sat=1;
    }

    
    if (std::abs(metY) > 0x3fff) { //0x7fff is 15 bits (decimal value 32767), however as MET is a signed value (can be negative) only 14 bits are allowed (16383)
        ATH_MSG_DEBUG("sumEthigh saturated: " << metY );
        metY = 0x7fff;
        sat=1;
    }

    //create basic tobword with 32 bits
    tobWord = tobWord + (res << FEXAlgoSpaceDefs::jXE_ResBit) + ((metY & 0x7fff) << FEXAlgoSpaceDefs::jXE_Ey_Bit) + ((metX & 0x7fff) << FEXAlgoSpaceDefs::jXE_Ex_Bit) + (sat << FEXAlgoSpaceDefs::jXE_SatBit)  ;
    ATH_MSG_DEBUG("tobword MET with Res, MET_Y, MET_X, Sat: " << std::bitset<32>(tobWord) );

    return tobWord;

}



} // end of namespace bracket
