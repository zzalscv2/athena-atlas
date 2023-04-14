/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           gFexInputByteStreamTool  -  This tool decodes Run3 gFEX input data!
//                              -------------------
//     begin                : 10 08 2022
//     email                : cecilia.tosciri@cern.ch
//  ***************************************************************************/

#include "gFexInputByteStreamTool.h"
#include "gFexPos.h"
#include "CxxUtils/span.h"
#include "eformat/SourceIdentifier.h"
#include "eformat/Status.h"

using ROBF = OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;
using WROBF = OFFLINE_FRAGMENTS_NAMESPACE_WRITE::ROBFragment;

namespace gPos = LVL1::gFEXPos;


gFexInputByteStreamTool::gFexInputByteStreamTool(const std::string& type,
        const std::string& name,
        const IInterface* parent)
    : base_class(type, name, parent) {}

StatusCode gFexInputByteStreamTool::initialize() {

    ATH_MSG_DEBUG(" ROB IDs: " << MSG::hex << m_robIds.value() << MSG::dec);

    // Conversion mode for gTowers
    ConversionMode gTowersmode = getConversionMode(m_gTowersReadKey, m_gTowersWriteKey, msg());
    ATH_CHECK(gTowersmode!=ConversionMode::Undefined);
    ATH_CHECK(m_gTowersWriteKey.initialize(gTowersmode==ConversionMode::Decoding));
    ATH_CHECK(m_gTowersReadKey.initialize(gTowersmode==ConversionMode::Encoding));
    ATH_MSG_DEBUG((gTowersmode==ConversionMode::Encoding ? "Encoding" : "Decoding") << " gTowers ");
    
    // Initialize monitoring tool if not empty
    if (!m_monTool.empty()) {
        ATH_CHECK(m_monTool.retrieve());
        ATH_MSG_INFO("Logging errors to " << m_monTool.name() << " monitoring tool");
        m_UseMonitoring = true;
    }    
    

    return StatusCode::SUCCESS;
}

// BS->xAOD conversion
StatusCode gFexInputByteStreamTool::convertFromBS(const std::vector<const ROBF*>& vrobf, const EventContext& ctx) const {
    
    //WriteHandle for gFEX EDMs
    
    //---gTower EDM
    SG::WriteHandle<xAOD::gFexTowerContainer> gTowersContainer(m_gTowersWriteKey, ctx);
    ATH_CHECK(gTowersContainer.record(std::make_unique<xAOD::gFexTowerContainer>(), std::make_unique<xAOD::gFexTowerAuxContainer>()));
    ATH_MSG_DEBUG("Recorded gFexTowerContainer with key " << gTowersContainer.key());
        
    // Iterate over ROBFragments to decode
    for (const ROBF* rob : vrobf) {
        // Iterate over ROD words and decode
        
        
        ATH_MSG_DEBUG("Starting to decode " << rob->rod_ndata() << " ROD words from ROB 0x" << std::hex << rob->rob_source_id());        
        //There is no data to decode.. not even the ROD trailers
        if(rob->rod_ndata() <= 0){
            continue;
        }
        
        const auto dataArray = CxxUtils::span{rob->rod_data(), rob->rod_ndata()};
    
        // Starting to loop over the gFEX words

        unsigned int n_words = rob->rod_ndata();

        int fpga = -99; //FPGA number 0,1,2 -> A,B,C
        int fpgaPosition = -99; //Position of the FPGA header in the dataframe

        // Preparing input fiber arrays to be used as input to the routine 
        // for converting fiber information into towers
        gfiber Afiber = {{{0}}};
        gfiber Bfiber = {{{0}}};
        gfiber Cfiber = {{{0}}};

        gfiber AMapped = {{{0}}};
        gfiber BMapped = {{{0}}};
        gfiber CMapped = {{{0}}};

        int rows = Afiber.size();
        int cols = Afiber[0].size();

        // Loop over the 32-bit words to extract words relative to each FPGA
        // Put 100x7 words available for each FPGA in corresponding arrays
        for(unsigned int iWord=0; iWord<n_words; iWord++) {
            if (dataArray[iWord] == gPos::FPGA_A_INPUT_HEADER){
                fpga = 0;
                fpgaPosition = iWord;                
            }
            else if (dataArray[iWord] == gPos::FPGA_B_INPUT_HEADER) {
                fpga = 1;
                fpgaPosition = iWord;  
            }
            else if (dataArray[iWord] == gPos::FPGA_C_INPUT_HEADER) {
                fpga = 2;
                fpgaPosition = iWord;  
            }
            else continue;


            if (fpga == 0){
                for (int irow = 0; irow < rows; irow++){
                    for (int icol = 0; icol < cols; icol++){
                       Afiber[irow][icol] = dataArray[fpgaPosition + (7*irow) + 1 + icol];
                    }
                }
            }
        
            if (fpga == 1){
                for (int irow = 0; irow < rows; irow++){
                    for (int icol = 0; icol < cols; icol++){
                       Bfiber[irow][icol] = dataArray[fpgaPosition + (7*irow) + 1 + icol];
                    }
                }
            }

            if (fpga == 2){
                for (int irow = 0; irow < rows; irow++){
                    for (int icol = 0; icol < cols; icol++){
                       Cfiber[irow][icol] = dataArray[fpgaPosition + (7*irow) + 1 + icol];
                    }
                }
            }
        }

        // For each FPGA we resemble the fibers->gTowers used in hardware
        // Atwr, Btwr, Ctwr will contain towers for each FPGA
        gtFPGA Atwr  = {{{0}}};
        gtFPGA Btwr  = {{{0}}};
        gtFPGA Ctwr  = {{{0}}};

        gtFPGA AtwrS  = {{{0}}};
        gtFPGA BtwrS  = {{{0}}};
        gtFPGA CtwrS  = {{{0}}};

        a_gtrx_map(Afiber, AMapped);

        int fpgaA = 0;
        // int puCorrA = 0;
        int fBcidA = -1; 
        int do_lconv = 1; 
   
        gtReconstructABC(fpgaA,
                         AMapped,               // input fibers AB_FIBER = 80 > C fibers
                         gPos::AB_FIBERS,
                         Atwr, &fBcidA,
                         do_lconv,              // flag to indicate multilinear conversion
                         gPos::AMPD_NFI, 
                         gPos::ACALO_TYPE, 
                         gPos::AMPD_GTRN_ARR, 
                         gPos::AMPD_DSTRT_ARR, 
                         gPos::AMPD_DTYP_ARR, 
                         gPos::AMSK  );

        gtRescale(Atwr, AtwrS, 4);

        b_gtrx_map(Bfiber, BMapped);

        int fpgaB = 1; 
        int fBcidB = -1;
    
        gtReconstructABC( fpgaB,
                          BMapped, gPos::AB_FIBERS, 
                          Btwr, &fBcidB,
                          do_lconv, 
                          gPos::BMPD_NFI, 
                          gPos::BCALO_TYPE, 
                          gPos::BMPD_GTRN_ARR, 
                          gPos::BMPD_DSTRT_ARR, 
                          gPos::BMPD_DTYP_ARR, 
                          gPos::BMSK  );

        gtRescale(Btwr, BtwrS, 4);

        c_gtrx_map(Cfiber, CMapped);

        int fpgaC = 2; 
        int fBcidC = -1;
    
        gtReconstructABC( fpgaC,
                          CMapped, gPos::C_FIBERS, 
                          Ctwr, &fBcidC,
                          do_lconv, 
                          gPos::CMPD_NFI, 
                          gPos::CCALO_TYPE, 
                          gPos::CMPD_GTRN_ARR, 
                          gPos::CMPD_DSTRT_ARR, 
                          gPos::CMPD_DTYP_ARR, 
                          gPos::CMSK  );

        gtRescale(Ctwr, CtwrS, 4);

        // Fill the gTower EDM with the corresponding towers
        int iEta = 0;
        int iPhi = 0;
        float Eta = 0;
        float Phi = 0;
        int Et  = 0;
        int Fpga = 0;
        char IsSaturated = 0;
        int towerID = 0;

        // Assign ID based on FPGA (FPGA-A 0->0; FPGA-B 1->10000, FPGA-C 2->20000) and gTower number assigned as per firmware convention


        int twr_rows = AtwrS.size();
        int twr_cols = AtwrS[0].size();
        
        Fpga = 0;

        // Save towers from FPGA A in gTower EDM
        for (int irow = 0; irow < twr_rows; irow++){
            for (int icol = 0; icol < twr_cols; icol++){
                iEta = icol + 8;
                iPhi = irow;
                Et = AtwrS[irow][icol];
                getEtaPhi(Eta, Phi, iEta, iPhi, towerID);
                gTowersContainer->push_back( std::make_unique<xAOD::gFexTower>() );
                gTowersContainer->back()->initialize(iEta, iPhi, Eta, Phi, Et, Fpga, IsSaturated, towerID);  
                towerID += 1;

  
            }
        }

        // Save towers from FPGA B in gTower EDM
        Fpga = 1;
        towerID = 10000;
        // Save towers from FPGA B in gTower EDM             
        for (int irow = 0; irow < twr_rows; irow++){
            for (int icol = 0; icol < twr_cols; icol++){
                iEta = icol + 20;
                iPhi = irow;
                Et = BtwrS[irow][icol];
                getEtaPhi(Eta, Phi, iEta, iPhi, towerID);
                gTowersContainer->push_back( std::make_unique<xAOD::gFexTower>() );
                gTowersContainer->back()->initialize(iEta, iPhi, Eta, Phi, Et, Fpga, IsSaturated, towerID);  
                towerID += 1;

            }
        }

        // Save towers from FPGA C in gTower EDM
        Fpga = 2;
        towerID = 20000;
        for (int irow = 0; irow < twr_rows; irow++){
            for (int icol = 0; icol < twr_cols/2; icol++){                
                iEta = icol + 2;
                iPhi = irow;
                Et = CtwrS[irow][icol];
                getEtaPhi(Eta, Phi, iEta, iPhi, towerID);
                gTowersContainer->push_back( std::make_unique<xAOD::gFexTower>() );
                gTowersContainer->back()->initialize(iEta, iPhi, Eta, Phi, Et, Fpga, IsSaturated, towerID);  
                towerID += 1;   
            }
            for (int icol = twr_cols/2; icol < twr_cols; icol++){                
                iEta = icol + 26;
                iPhi = irow;
                Et = CtwrS[irow][icol];
                getEtaPhi(Eta, Phi, iEta, iPhi, towerID);
                gTowersContainer->push_back( std::make_unique<xAOD::gFexTower>() );
                gTowersContainer->back()->initialize(iEta, iPhi, Eta, Phi, Et, Fpga, IsSaturated, towerID);  
                towerID += 1;

            }
        }  
        

    }
        
    return StatusCode::SUCCESS;
}




void gFexInputByteStreamTool::a_gtrx_map( const gfiber &inputData, gfiber &jf_lar_rx_data) const{
   
    int rows = inputData.size();
    int cols = inputData[0].size();
  
    for(int i = 0; i < rows; i++){
        for (int j=0; j< cols; j++){
            if(i < 80) {
                jf_lar_rx_data[i][j] = inputData[gPos::GTRX_MAP_A_IND[i]][j];
            } 
            else if (j<cols-1){
                jf_lar_rx_data[i][j] = 0;
            } 
            else {
                jf_lar_rx_data[i][j] = (inputData[0][0] & 0x03F0000);
            }
        }
    }
}


void gFexInputByteStreamTool::b_gtrx_map( const gfiber &inputData, gfiber &jf_lar_rx_data) const{

    int rows = inputData.size();
    int cols = inputData[0].size();

    for(int i =0; i< rows; i++){
        for (int j=0; j< cols; j++){
            if( i< 80) {
                jf_lar_rx_data[i][j] = inputData[gPos::GTRX_MAP_B_IND[i]][j];
            } 
            else if (j<cols-1){
                jf_lar_rx_data[i][j] = 0;
            } 
            else {
                jf_lar_rx_data[i][j] = (inputData[0][0] & 0x03F0000);
            }
        }
    }
}


void gFexInputByteStreamTool::c_gtrx_map( const gfiber &inputData, gfiber &outputData) const{

    int rows = inputData.size();
    int cols = inputData[0].size();
 
    for(int i =0; i< rows; i++){
        for (int j=0; j< cols; j++){
            if( i< 50) {
                outputData[i][j] = inputData[gPos::GTRX_MAP_C_IND[i]][j];
            } 
            else if( j< cols-1 ) {
                outputData[i][j] = 0;
            } 
            else {
                outputData[i][j] = ( inputData[0][0] & 0x03F0000);
            }
        }
    }
}


void gFexInputByteStreamTool::gtReconstructABC(int XFPGA, 
                                               gfiber Xfiber,  int Xin, 
                                               gtFPGA &Xgt, int *BCIDptr,
                                               int do_lconv, 
                                               std::array<int, gPos::MAX_FIBERS> XMPD_NFI,
                                               std::array<int, gPos::MAX_FIBERS>  XCALO_TYPE,
                                               gCaloTwr XMPD_GTRN_ARR,
                                               gType XMPD_DSTRT_ARR,  
                                               gTypeChar XMPD_DTYP_ARR,
                                               std::array<int, gPos::MAX_FIBERS> XMSK) const{
 
// Output is uncalibrated gTowers with 50MeV LSB
//       Xfiber -- 80 fibers, each with seven words, 32 bits per word
//       Xin    -- usually 80 -- number of fibers actually used, is 50 for EMEC/HEC FPGAC 
//       Xgt    --  12*32 = 384 towers -- given as integers, 
//              limited to 12 signed bits in hardware ( -2048 to 2047, lsb 200 MeV)  
//              Currently no calibration is implemented

//       XMPD_NFI         -- gives the fiber type 0, 1, 2, 3 (A & B only use types 0,1,2) 
//       XMPD_DTYP_ARR    -- gives the detector type for the 20 fields on a fiber
//       XMPD_D_STRT      -- gives the starting bit out of 224 for 20 fields that can be on a fiber
//       XMPD_GTRN_ARR    -- maps the first 16 of 20 fields onto one of the 384 towers 

//       In the firmware Xfiber is an array of 32 bit words and on each of seven clocks new data for a BC comes in.


    //loop over fibers -- 
    for(int irow=0; irow<gPos::ABC_ROWS; irow++){
        for(int icolumn=0; icolumn<gPos::AB_COLUMNS; icolumn++){
            Xgt[irow][icolumn] = 0;
        }
    }

    // detector (data field type) type :
    // -- "0000" - EMB, EMB/EMEC -> EM contribution  0
    // -- "0001" - TREX,HEC - Had contribution       1
    // -- "0010" - extended region ( EMEC)           2
    // -- "0011" - extended region ( HEC)            3
    // -- "0100" - position info (for EMB, EMB/EMEC)
    // -- "0101" - CRC
    // -- "0110" - overlaping HEC  - gTower will be sum of EMEC + TREX + overlaping HEC input)  6
    // -- "1000" - saturation flags for inputs 7-0 
    // -- "1001" - saturation flags for inputs 15-8
    // -- "1010" - BCID_LOW
    // -- "1111" - unused field

    // eventually need to read this from record 
    *BCIDptr = 0;
    
  
    std::array<int, gPos::AB_TOWERS> etowerData;
    std::array<int, gPos::AB_TOWERS> htowerData;
    std::array<int, gPos::ABC_ROWS>  xetowerData;
    std::array<int, gPos::ABC_ROWS>  xhtowerData;
    std::array<int, gPos::ABC_ROWS>  ohtowerData;
  
    etowerData.fill(0);
    htowerData.fill(0);
    xetowerData.fill(0);
    xhtowerData.fill(0);
    ohtowerData.fill(0);


    for(int iFiber = 0; iFiber < Xin; iFiber++) { 
        // first do CRC check
        std::array<int, 6> tmp; 
        for(int i = 0; i < 6; i++){ tmp[i] = Xfiber[iFiber][i];  }; 
        int CRC  = crc9d32(tmp, 6, 1);
        int withoutComma = Xfiber[iFiber][6] & 0xFFFFFF00 ; 
        CRC = crc9d23(withoutComma, CRC,  1 );
        int StoredCRC = ( (Xfiber[iFiber][6]>>23) & 0x000001FF);
        if( (CRC != StoredCRC) && (StoredCRC != 0 ) )   {
            
            std::stringstream sdetail;
            sdetail  << "[gFexInputByteStreamTool::gtReconstructABC]: Fiber " << iFiber << "of Xin " << Xin << ":  BAD New CRC: " << CRC << "Stored CRC "<< StoredCRC ;
            std::stringstream slocation;
            slocation  << "Fiber " << iFiber << "of Xin " << Xin;
            std::stringstream stitle;
            stitle  << "Bad CRC" ;
            printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str());                 
            
        }
        
        // now check if CRC lower bits are correct in the trailer
        int fiber_type  = XMPD_NFI[iFiber];
        // each fiber has 20 pieces of information -- called iDatum here 
        for(int iDatum = 0; iDatum < 16; iDatum++) {
            // tells where the data is coming from -- see data field type above 
            int dataType = XMPD_DTYP_ARR[fiber_type][iDatum];
            // tower number 0 - 383 
            int ntower = XMPD_GTRN_ARR[iFiber][iDatum];
            if( ntower == -1 ){
                ATH_MSG_DEBUG("[gFexInputByteStreamTool::gtReconstructABC: unused location  iFiber "<< iFiber << ", calo type "<< XCALO_TYPE[iFiber]<<", data type "<< dataType <<", iDatum " << iDatum << "tower " << ntower);
            } 
            else if( (ntower < 0) || (ntower >383) ){
                
                std::stringstream sdetail;
                sdetail  << "[gFexInputByteStreamTool::gtReconstructABC: bad value of ntower: iFiber "<< iFiber<< ", calo type"<< XCALO_TYPE[iFiber]<< ", data type "<< dataType<< ", iDatum "<< iDatum<< "tower "<< ntower ;
                std::stringstream slocation;
                slocation  << "iFiber "<< iFiber<< ", calo type"<< XCALO_TYPE[iFiber];
                std::stringstream stitle;
                stitle  << "Bad value of ntower" ;
                printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str());                     
                
            }

            // bit number in the 32*7 = 234 bits transmitted in one BC
            // word refers to the 7 32 bits words transmitted in each BC 
            int ihigh     = XMPD_DSTRT_ARR[fiber_type][iDatum];
            int ihword    = ihigh/32;
            int ihbit     = ihigh%32;
  
            int ilow = 0;
            int ilword = 0;
            int ilbit = 0;

            int hTREXval = 0;
            int lTREXval = 0;
            int hHECval = 0;
            int lHECval = 0;
      
            // need to be sure to skip positon data!
            int mask = 0;
            int lmask = 0;
            int hmask = 0;
      
            if( XMSK[iFiber] != 1 ) {
                dataType = 99;
            }

            if( XCALO_TYPE[iFiber] < 3) {
                switch(dataType){
                    case 0:
                    ilow    = ihigh - 11;
                    ilword  = ilow/32; 
                    ilbit   = ilow%32;
                    if(ilword == ihword){
                        mask = 0x00000FFF;
                        mask = mask << (ilow);
                        etowerData[ntower] = etowerData[ntower] | ( (Xfiber[iFiber][ihword] & mask) >> ilbit );
                        // undo multilinear decoding
                        if( do_lconv){
                            undoMLE( etowerData[ntower] );
                        } 
                        else {
                        // sign extend etower data 
                        if( etowerData[ntower] & 0x00000800 ){ etowerData[ntower] = (etowerData[ntower] | 0xFFFFF000) ;}
                        etowerData[ntower] = etowerData[ntower]*4; 
                        }     
                    } 
                    else {
                        
                        std::stringstream sdetail;
                        sdetail  << "[gFexInputByteStreamTool::gtReconstructABC]: wrongly packed data "<< fiber_type<< ", "<<dataType<< ", "<<ilword<< ", " <<ihword ;
                        std::stringstream slocation;
                        slocation  << "Fiber type "<< fiber_type<< " and data type"<< dataType;
                        std::stringstream stitle;
                        stitle  << "Wrongly packed data" ;
                        printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str());                             
                        
                    }
                    break;

                    case 1:
                    ilow    = ihigh - 11;
                    ilword  = ilow/32;
                    ilbit   = ilow%32;
  
                    hTREXval = Xfiber[iFiber][ihword];
                    lTREXval = Xfiber[iFiber][ilword];
                    mask  =0;
                    lmask =0;
                    hmask =0; 
  
                    if(ilword == ihword){
                        mask = 0x00000FFF;
                        mask = mask << ilow;
                        htowerData[ntower] = htowerData[ntower] | ( (hTREXval & mask) >> (ilbit) );
                    } 
                    else if ( ihbit == 7 ) {
                        mask  = 0x0000000F;
                        hmask = 0x000000FF;
                        htowerData[ntower] = htowerData[ntower] | (   (hTREXval & hmask) << 4);
                        lmask = 0xF0000000;
                        htowerData[ntower] = htowerData[ntower] | ( ( (lTREXval & lmask) >> 28)&mask)  ; 
                    } 
                    else if ( ihbit == 3) {
                        mask  = 0x000000FF;
                        hmask = 0x0000000F;
                        htowerData[ntower] = htowerData[ntower] | (  ( hTREXval & hmask) << 8);
                        lmask = 0xFF000000;
                        htowerData[ntower] = htowerData[ntower] | ( ( (lTREXval & lmask) >> 24) &mask) ;
                    } 
                    else {
                        
                        std::stringstream sdetail;
                        sdetail  << "[gFexInputByteStreamTool::gtReconstructABC]: wrongly packed data "<< fiber_type<< ", "<<dataType<< ", "<<ilword<< ", " <<ihword ;
                        std::stringstream slocation;
                        slocation  << "Fiber type "<< fiber_type<< " and data type"<< dataType;
                        std::stringstream stitle;
                        stitle  << "Wrongly packed data" ;
                        printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str());                          
                        
                    }

                    // mulitply by 20 to make 50 MeV LSB
                    if( (Xin > 50) ) {
                        htowerData[ntower] = 20*htowerData[ntower];
                    } 
                    else {
                        if( do_lconv){
                            undoMLE( etowerData[ntower] );
                        } 
                        else {
                        // sign extend etower data 
                        if( htowerData[ntower] & 0x00000800 ){   htowerData[ntower] = (etowerData[ntower] | 0xFFFFF000) ;}
                        htowerData[ntower] = htowerData[ntower]*4; 
                        }      
                    }
                    break;

                    case 2:
                    ilow    = ihigh - 11;
                    ilword  = ilow/32;
                    ilbit   = ilow%32;
                    if( ntower > 32 ){
                        
                        std::stringstream sdetail;
                        sdetail  << "[gFexInputByteStreamTool::gtReconstructABC]: bad value of nTower for extended region 2.4 - 2.5 in eta" ;
                        std::stringstream slocation;
                        slocation  << "Fiber type "<< fiber_type<< " and data type"<< dataType;
                        std::stringstream stitle;
                        stitle  << "Bad value of nTower in extended eta" ;
                        printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str());                          
                        
                    }
                    if(ilword == ihword){
                        int mask = 0x00000FFF;
                        mask = mask << ilbit; 
                        xetowerData[ntower] = xetowerData[ntower] | ( (Xfiber[iFiber][ihword]&mask) >> (ilbit)  );
                    } 
                    else if ( ihbit == 7 ) {
                        mask  = 0x0000000F;
                        hmask = 0x000000FF;
                        xetowerData[ntower] = xetowerData[ntower] | (  (Xfiber[iFiber][ihword]&hmask) << 4);
                        lmask = 0xF0000000;
                        xetowerData[ntower] = xetowerData[ntower] | ( (  (Xfiber[iFiber][ilword]&lmask) >> 28)&mask)  ;
                    } 
                    else if ( ihbit == 3) {
                        mask  = 0x000000FF;
                        hmask = 0x000000F;
                        xetowerData[ntower] = xetowerData[ntower] | (  (Xfiber[iFiber][ihword]&hmask) << 8);
                        lmask = 0xFF000000;
                        xetowerData[ntower] = xetowerData[ntower] | ( (  (Xfiber[iFiber][ilword]&lmask) >> 24)&mask)  ;
                    } 
                    else {
                        
                        std::stringstream sdetail;
                        sdetail  << "[gFexInputByteStreamTool::gtReconstructABC]: wrongly packed data "<< fiber_type<< ", "<<dataType<< ", "<<ilword<< ", " <<ihword ;
                        std::stringstream slocation;
                        slocation  << "Fiber type "<< fiber_type<< " and data type"<< dataType;
                        std::stringstream stitle;
                        stitle  << "Wrongly packed data" ;
                        printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str());   
                        
                    }
                    // undo multilinear decoding
                    if( do_lconv){
                        undoMLE( xetowerData[ntower] );
                    } 
                    else {
                        // sign extend etower data 
                        if( xetowerData[ntower] & 0x00000800 ){   xetowerData[ntower] = (xetowerData[ntower] | 0xFFFFF000) ;}
                        xetowerData[ntower] = xetowerData[ntower]*4; 
                    }   
                    break;
    
                    case 3:
                    ilow    = ihigh - 11;
                    ilword  = ilow/32;
                    ilbit   = ilow%32;
                    if(ilword == ihword){
                        mask = 0x00000FFF;
                        mask = mask << ilbit;
                        xhtowerData[ntower] = xhtowerData[ntower] | ( (Xfiber[iFiber][ihword]&mask) >> (ilbit)  );
                    } 
                    else if ( ihbit == 7 ) {
                        mask  = 0x0000000F;
                        hmask = 0x000000FF;
                        xhtowerData[ntower] = xhtowerData[ntower] | (  (Xfiber[iFiber][ihword]&hmask) << 4);
                        lmask = 0xF0000000;
                        xhtowerData[ntower] = xhtowerData[ntower] | ( (  (Xfiber[iFiber][ilword]&lmask) >> 28)&mask)  ;
                    } 
                    else if ( ihbit == 3) {
                        mask  = 0x000000FF;
                        hmask = 0x0000000F;
                        xhtowerData[ntower] = xhtowerData[ntower] | (  (Xfiber[iFiber][ihword]&hmask) << 8);
                        lmask = 0xFF000000;
                        xhtowerData[ntower] = xhtowerData[ntower] | ( (  (Xfiber[iFiber][ilword]&lmask) >> 24)&mask)  ;
                    } 
                    else {
                        
                        std::stringstream sdetail;
                        sdetail  << "[gFexInputByteStreamTool::gtReconstructABC]: wrongly packed data "<< fiber_type<< ", "<<dataType<< ", "<<ilword<< ", " <<ihword ;
                        std::stringstream slocation;
                        slocation  << "Fiber type "<< fiber_type<< " and data type"<< dataType;
                        std::stringstream stitle;
                        stitle  << "Wrongly packed data" ;
                        printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str());                           
                        
                    }
                    if( ntower > 32 ){
                        
                        std::stringstream sdetail;
                        sdetail  << "[gFexInputByteStreamTool::gtReconstructABC]: bad value of nTower for extended region 2.4 - 2.5 in eta" ;
                        std::stringstream slocation;
                        slocation  << "Fiber type "<< fiber_type<< " and data type"<< dataType;
                        std::stringstream stitle;
                        stitle  << "Bad value of nTower in extended eta" ;
                        printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str());                        
                        
                    }
                    // undo multilinear decoding
                    if( do_lconv){
                        undoMLE( xhtowerData[ntower] );
                    } 
                    else {
                        // sign extend etower data 
                        if( xhtowerData[ntower] & 0x00000800 ){   xhtowerData[ntower] = (xhtowerData[ntower] | 0xFFFFF000) ;}
                        xhtowerData[ntower] = xhtowerData[ntower]*4; 
                    }   
                    break;
    
                    case 6:
                    ilow    = ihigh - 11;
                    ilword  = ilow/32;
                    ilbit   = ilow%32;
                    if(ilword == ihword){
                        mask = 0x00000FFF;
                        mask = mask << ilbit;
                        ohtowerData[ntower] = ohtowerData[ntower] | ( (Xfiber[iFiber][ihword]&mask) >> (ilbit)  );
                    } 
                    else if ( ihbit == 7 ) {
                        mask  = 0x0000000F;
                        hmask = 0x000000FF;
                        ohtowerData[ntower] = ohtowerData[ntower] | (  (Xfiber[iFiber][ihword]&hmask) << 4);
                        lmask = 0xF0000000;
                        ohtowerData[ntower] = ohtowerData[ntower] | ( (  (Xfiber[iFiber][ilword]&lmask) >> 28)&mask)  ;
                    } 
                    else if ( ihbit == 3) {
                        mask  = 0x000000FF;
                        hmask = 0x0000000F;
                        ohtowerData[ntower] = ohtowerData[ntower] | (  (Xfiber[iFiber][ihword]&hmask) << 8);
                        lmask = 0xFF000000;
                        ohtowerData[ntower] = ohtowerData[ntower] | ( (  (Xfiber[iFiber][ilword]&lmask) >> 24)&mask)  ;
                    } 
                    else {
                        
                        std::stringstream sdetail;
                        sdetail  << "[gFexInputByteStreamTool::gtReconstructABC]: wrongly packed data "<< fiber_type<< ", "<<dataType<< ", "<<ilword<< ", " <<ihword ;
                        std::stringstream slocation;
                        slocation  << "Fiber type "<< fiber_type<< " and data type"<< dataType;
                        std::stringstream stitle;
                        stitle  << "Wrongly packed data" ;
                        printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str());                            
                        
                    }
                    if( ntower > 32 ){
                        
                        std::stringstream sdetail;
                        sdetail  << "[gFexInputByteStreamTool::gtReconstructABC]: bad value of nTower for extended region 2.4 - 2.5 in eta" ;
                        std::stringstream slocation;
                        slocation  << "Fiber type "<< fiber_type<< " and data type"<< dataType;
                        std::stringstream stitle;
                        stitle  << "Bad value of nTower in extended eta" ;
                        printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str());                            
                        
                    }
                    if( do_lconv){
                        undoMLE( ohtowerData[ntower] );
                    } 
                    else {
                        // sign extend etower data 
                        if( ohtowerData[ntower] & 0x00000800 ){   ohtowerData[ntower] = (ohtowerData[ntower] | 0xFFFFF000) ;}
                         ohtowerData[ntower] = ohtowerData[ntower]*4; 
                    }   
                    break;

                    case 11:
                    ilow    = ihigh - 11;
                    ilword  = ilow/32;
                    ilbit   = ilow%32;

                    hHECval = Xfiber[iFiber][ihword];
                    lHECval = Xfiber[iFiber][ilword];
                    if(ilword == ihword){
                        mask = 0x00000FFF;
                        mask = mask << ilow;
                        htowerData[ntower] = htowerData[ntower] | ( (hHECval & mask) >> (ilbit) );
                    } 
                    else if ( ihbit == 7 ) {
                        mask  = 0x0000000F;
                        hmask = 0x000000FF;
                        htowerData[ntower] = htowerData[ntower] | (   (hHECval & hmask) << 4);
                        lmask = 0xFF000000;
                        htowerData[ntower] = htowerData[ntower] | ( ( (lHECval & lmask) >> 28)&mask)  ; 
                    } 
                    else if ( ihbit == 3) {
                        mask  = 0x000000FF;
                        hmask = 0x0000000F;
                        htowerData[ntower] = htowerData[ntower] | (  ( hHECval & hmask) << 8);
                        lmask = 0xFF000000;
                        htowerData[ntower] = htowerData[ntower] | ( ( (lHECval & lmask) >> 24) &mask) ;
                    } 
                    else {
                        
                        std::stringstream sdetail;
                        sdetail  << "[gFexInputByteStreamTool::gtReconstructABC]: wrongly packed data "<< fiber_type<< ", "<<dataType<< ", "<<ilword<< ", " <<ihword ;
                        std::stringstream slocation;
                        slocation  << "Fiber type "<< fiber_type<< " and data type"<< dataType;
                        std::stringstream stitle;
                        stitle  << "Wrongly packed data" ;
                        printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str());                         
                        
                    }
                    if( do_lconv){
                        undoMLE( htowerData[ntower] );
                    } 
                    else {
                        // sign extend etower data 
                        if( htowerData[ntower] & 0x00000800 ){   htowerData[ntower] = (htowerData[ntower] | 0xFFFFF000) ;}
                        htowerData[ntower] = htowerData[ntower]*4; 
                    }   
                    break;
                }
                // FPGA C EMEC/HEC + FCAL
                // These all have same dataType as extended ECAL and extended HCAL
            } 
            else {
            // only types 2 and 3 exist in FPGA C
    
                switch(dataType){ 
                case 2:
                ilow    = ihigh - 11;
                ilword  = ilow/32;
                ilbit   = ilow%32;
        
                if(ilword == ihword){
                    int mask = 0x00000FFF;
                    mask = mask << ilbit; 
                    etowerData[ntower] = etowerData[ntower] | ( (Xfiber[iFiber][ihword]&mask) >> (ilbit)  );
                } 
                else if ( ihbit == 7 ) {
                    mask  = 0x0000000F;
                    hmask = 0x000000FF;
                    etowerData[ntower] = etowerData[ntower] | (  (Xfiber[iFiber][ihword]&hmask) << 4);
                    lmask = 0xF0000000;
                    etowerData[ntower] = etowerData[ntower] | ( (  (Xfiber[iFiber][ilword]&lmask) >> 28)&mask)  ;
                } 
                else if ( ihbit == 3) {
                    mask  = 0x000000FF;
                    hmask = 0x000000F;
                    etowerData[ntower] = etowerData[ntower] | (  (Xfiber[iFiber][ihword]&hmask) << 8);
                    lmask = 0xFF000000;
                    etowerData[ntower] = etowerData[ntower] | ( (  (Xfiber[iFiber][ilword]&lmask) >> 24)&mask)  ;
                } 
                else {

                    std::stringstream sdetail;
                    sdetail  << "[gFexInputByteStreamTool::gtReconstructABC]: wrongly packed data "<< fiber_type<< ", "<<dataType<< ", "<<ilword<< ", " <<ihword ;
                    std::stringstream slocation;
                    slocation  << "Fiber type "<< fiber_type<< " and data type"<< dataType;
                    std::stringstream stitle;
                    stitle  << "Wrongly packed data" ;
                    printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str());                     
                    
                }
                // undo multilinear decoding
                if( do_lconv){
                    undoMLE( etowerData[ntower] );
                }  
                else {
                    // sign extend etower data 
                    if( etowerData[ntower] & 0x00000800 ){   etowerData[ntower] = (etowerData[ntower] | 0xFFFFF000) ;}
                    etowerData[ntower] = etowerData[ntower]*4; 
                }   
                break;
    
                case 3:
                ilow    = ihigh - 11;
                ilword  = ilow/32;
                ilbit   = ilow%32;
      
                if(ilword == ihword){
                    mask = 0x00000FFF;
                    mask = mask << ilbit;
                    htowerData[ntower] = htowerData[ntower] | ( (Xfiber[iFiber][ihword]&mask) >> (ilbit)  );
                } 
                else if ( ihbit == 7 ) {
                    mask  = 0x0000000F;
                    hmask = 0x000000FF;
                    htowerData[ntower] = htowerData[ntower] | (  (Xfiber[iFiber][ihword]&hmask) << 4);
                    lmask = 0xF0000000;
                    htowerData[ntower] = htowerData[ntower] | ( (  (Xfiber[iFiber][ilword]&lmask) >> 28)&mask)  ;
                } 
                else if ( ihbit == 3) {
                    mask  = 0x000000FF;
                    hmask = 0x0000000F;
                    htowerData[ntower] = htowerData[ntower] | (  (Xfiber[iFiber][ihword]&hmask) << 8);
                    lmask = 0xFF000000;
                    htowerData[ntower] = htowerData[ntower] | ( (  (Xfiber[iFiber][ilword]&lmask) >> 24)&mask)  ;
                } 
                else {

                    std::stringstream sdetail;
                    sdetail  << "[gFexInputByteStreamTool::gtReconstructABC]: wrongly packed data "<< fiber_type<< ", "<<dataType<< ", "<<ilword<< ", " <<ihword ;
                    std::stringstream slocation;
                    slocation  << "Fiber type "<< fiber_type<< " and data type"<< dataType;
                    std::stringstream stitle;
                    stitle  << "Wrongly packed data" ;
                    printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str());             
                            
                }
                // undo multilinear decoding
                if( do_lconv){
                    undoMLE( htowerData[ntower] );
                } 
                else {
                    // sign extend etower data 
                    if( htowerData[ntower] & 0x00000800 ){ htowerData[ntower] = (htowerData[ntower] | 0xFFFFF000) ;}
                    htowerData[ntower] = htowerData[ntower]*4; 
                }   
                break;

                case 15:
                break;

                case 99:
                break; 
    
                default:
                
                    std::stringstream sdetail;
                    sdetail  << "[gFexInputByteStreamTool::gtReconstructABC]: wrong detector type "<< dataType ;
                    std::stringstream slocation;
                    slocation  << "Fiber type "<< fiber_type<< " and data type"<< dataType;
                    std::stringstream stitle;
                    stitle  << "Wrong detector type" ;
                    printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str()); 
                    
                } // end of case statement for FPGAC 
            } // end of | eta | > 2,5 
        } // end of loop over words
    } // end of loop over fibers

  
    // no need for xdtower, xhtower and ohtower in FPGA C -- but these will all be zero in that case.
    if( XFPGA == 0 ) {
        for(int itower=0;itower<384;itower++){
            int icolumn = itower%12;
            int irow    =  itower/12;
            Xgt[irow][icolumn] = etowerData[itower] + htowerData[itower];
            if ( icolumn == 0) {
                Xgt[irow][icolumn] = Xgt[irow][icolumn] + xetowerData[irow] + xhtowerData[irow]; 
            }
            if ( icolumn == 4) {
                Xgt[irow][icolumn] = Xgt[irow][icolumn]  + ohtowerData[irow]; 
            }
        }
    } 
    else if ( XFPGA == 1 ) {
        for(int itower=0;itower<384;itower++){
            int icolumn = itower%12;
            int irow    =  itower/12;
            Xgt[irow][icolumn] = etowerData[itower] + htowerData[itower];
            if ( icolumn == 11) {
                Xgt[irow][icolumn] = Xgt[irow][icolumn] + xetowerData[irow] + xhtowerData[irow]; 
            }
            if ( icolumn == 7 ) {
                Xgt[irow][icolumn] = Xgt[irow][icolumn]  + ohtowerData[irow]; 
            }
        }  
    } 
    else if ( XFPGA == 2 ) {
        for(int itower=0;itower<384;itower++){
            int icolumn = itower%12;
            int irow    =  itower/12;
            Xgt[irow][icolumn] = etowerData[itower] + htowerData[itower];
        }
    } 
    else {
    ATH_MSG_DEBUG("[gFexInputByteStreamTool::gtReconstructABC]: Bad FPGA # "<< XFPGA);
    }
}



int gFexInputByteStreamTool::crc9d32(std::array<int, 6> inWords,int numWords,int reverse) const{
  // calculate this for reversed bits

    std::array<int, 32> dIn; 
    std::array<int, 9> crc_s;
    std::array<int, 9> crc_r;

    crc_s.fill(1);
    crc_r.fill(1);

    int crc_word = 0x000; 
    unsigned int mask = 0x00000001;
    
    for(int k =0; k < numWords; k++) {
        if( reverse == 1 ) {
            for (int i =0 ; i < 32; i++ ) {
                dIn[31-i] = (inWords[k] & (mask << i));
                dIn[31-i] = ((dIn[31-i] >> i) & 0x00000001);  
            }
        } 
        else {
            for (int i =0 ; i<32; i++ ) {
                dIn[i] = inWords[k] & (mask << i);
                dIn[i] = ((dIn[i] >> i) & 0x0000001);  
            }
        }
        for(int j=0; j<9; j++){
          crc_s[j] = crc_r[j];
        }

        crc_r[0] = crc_s[0] ^ crc_s[2] ^ crc_s[3] ^ crc_s[6] ^ crc_s[8] ^ dIn[0]  ^ dIn[2] ^ dIn[3] ^ dIn[5]  ^ dIn[6]  ^ dIn[7]  ^ dIn[8] ^ dIn[9] ^ dIn[10] ^ dIn[11] ^ dIn[15] ^ dIn[18] ^ dIn[19] ^ dIn[20] ^ dIn[21] ^ dIn[22] ^ dIn[23] ^ dIn[25] ^ dIn[26] ^ dIn[29] ^ dIn[31];
      
        crc_r[1] = crc_s[1] ^ crc_s[2] ^ crc_s[4] ^ crc_s[6] ^ crc_s[7] ^ crc_s[8] ^ dIn[0] ^ dIn[1] ^ dIn[2]  ^ dIn[4]  ^ dIn[5]  ^ dIn[12] ^ dIn[15] ^ dIn[16] ^ dIn[18] ^ dIn[24] ^ dIn[25] ^ dIn[27] ^ dIn[29] ^ dIn[30] ^ dIn[31];

        crc_r[2] = crc_s[2] ^ crc_s[3] ^ crc_s[5] ^ crc_s[7] ^ crc_s[8] ^ dIn[1]  ^ dIn[2] ^ dIn[3] ^ dIn[5]  ^ dIn[6]  ^ dIn[13] ^ dIn[16] ^ dIn[17] ^ dIn[19] ^ dIn[25] ^ dIn[26] ^ dIn[28] ^ dIn[30] ^ dIn[31];
      
        crc_r[3] = crc_s[0] ^ crc_s[2] ^ crc_s[4] ^ dIn[0]  ^ dIn[4]  ^ dIn[5]  ^ dIn[8] ^ dIn[9] ^ dIn[10] ^ dIn[11] ^ dIn[14] ^ dIn[15] ^ dIn[17] ^ dIn[19] ^ dIn[21] ^ dIn[22] ^ dIn[23] ^ dIn[25] ^ dIn[27];
      
        crc_r[4] = crc_s[1] ^ crc_s[2] ^ crc_s[5] ^ crc_s[6] ^ crc_s[8] ^ dIn[0]  ^ dIn[1] ^ dIn[2] ^ dIn[3]  ^ dIn[7]  ^ dIn[8]  ^ dIn[12] ^ dIn[16] ^ dIn[19] ^ dIn[21] ^ dIn[24] ^ dIn[25] ^ dIn[28] ^ dIn[29] ^ dIn[31];
      
        crc_r[5] = crc_s[0] ^ crc_s[7] ^ crc_s[8] ^ dIn[0]  ^ dIn[1]  ^ dIn[4]  ^ dIn[5] ^ dIn[6] ^ dIn[7]  ^ dIn[10] ^ dIn[11] ^ dIn[13] ^ dIn[15] ^ dIn[17] ^ dIn[18] ^ dIn[19] ^ dIn[21] ^ dIn[23] ^ dIn[30] ^ dIn[31];
      
        crc_r[6] = crc_s[0] ^ crc_s[1] ^ crc_s[2] ^ crc_s[3] ^ crc_s[6] ^ dIn[0]  ^ dIn[1] ^ dIn[3] ^ dIn[9]  ^ dIn[10] ^ dIn[12] ^ dIn[14] ^ dIn[15] ^ dIn[16] ^ dIn[21] ^ dIn[23] ^ dIn[24] ^ dIn[25] ^ dIn[26] ^ dIn[29];
      
        crc_r[7] = crc_s[0] ^ crc_s[1] ^ crc_s[4] ^ crc_s[6] ^ crc_s[7] ^ crc_s[8] ^ dIn[0] ^ dIn[1] ^ dIn[3]  ^ dIn[4]  ^ dIn[5]  ^ dIn[6] ^ dIn[7] ^ dIn[8] ^ dIn[9] ^ dIn[13] ^ dIn[16] ^ dIn[17] ^ dIn[18] ^ dIn[19] ^ dIn[20] ^ dIn[21] ^ dIn[23] ^ dIn[24] ^ dIn[27] ^ dIn[29] ^ dIn[30] ^ dIn[31];
      
        crc_r[8] = crc_s[1] ^ crc_s[2] ^ crc_s[5] ^ crc_s[7] ^ crc_s[8] ^ dIn[1]  ^ dIn[2] ^ dIn[4] ^ dIn[5]  ^ dIn[6]  ^ dIn[7]  ^ dIn[8] ^ dIn[9] ^ dIn[10] ^ dIn[14] ^ dIn[17] ^ dIn[18] ^ dIn[19] ^ dIn[20] ^ dIn[21] ^ dIn[22] ^ dIn[24] ^ dIn[25] ^ dIn[28] ^ dIn[30] ^ dIn[31];
 
    }
  
    if ( reverse == 1){    
        for (int i = 0; i < 9; i++) {
            crc_word = crc_word | (crc_r[8-i] << i) ;
        }
    } 
    else {
        for(int i = 0; i < 9; i++ ){
            crc_word = crc_word | (crc_r[i] << i);
        }
    }
    return crc_word;
}


int gFexInputByteStreamTool::crc9d23(int inword, int in_crc, int  reverse ) const{

    int mask  = 0x00000001; 
    //#dIn is a '23-bit input word'
    std::array<int, 23> dIn;

    std::array<int, 9> crc_r;
    std::array<int, 9> crc_in_s;
 
    crc_r.fill(1);
    crc_in_s.fill(1);
 
    int crc_word = 0x000; 
   
    //32-bit word crc calculation 

    if (reverse == 1) {
        for(int i = 0; i < 23;i++){ 
            dIn[22-i] = ( inword & (mask << i));
            dIn[22-i] = ( dIn[22-i] >> i);
        }
        for(int i = 0; i < 9;i++){ 
            crc_in_s[8-i] = ( in_crc & (mask << i) );
            crc_in_s[8-i] = ( crc_in_s[8-i] >> i ); 
        }
    }
    else{
        for(int i = 0; i < 23; i++) {
            dIn[i] = ( inword & (mask << i) );  
            dIn[i] = (dIn[i] >> i);
        }
        for(int i=0; i<9; i++){
            crc_in_s[i] = ( in_crc & (mask << i));
            crc_in_s[i] = (crc_in_s[i] >> i);
        }
    }
  
    crc_r[0] = crc_in_s[1] ^ crc_in_s[4] ^ crc_in_s[5] ^ crc_in_s[6] ^ crc_in_s[7] ^ crc_in_s[8] ^ dIn[0] ^ dIn[2] ^ dIn[3] ^ dIn[5] ^ dIn[6] ^ dIn[7] ^ dIn[8] ^ dIn[9] ^ dIn[10] ^ dIn[11] ^ dIn[15] ^ dIn[18] ^ dIn[19] ^ dIn[20] ^ dIn[21] ^ dIn[22];
    crc_r[1] = crc_in_s[1] ^ crc_in_s[2] ^ crc_in_s[4] ^ dIn[0] ^ dIn[1] ^ dIn[2] ^ dIn[4] ^ dIn[5] ^ dIn[12] ^ dIn[15] ^ dIn[16] ^ dIn[18];
    crc_r[2] = crc_in_s[2] ^ crc_in_s[3] ^ crc_in_s[5] ^ dIn[1] ^ dIn[2] ^ dIn[3] ^ dIn[5] ^ dIn[6] ^ dIn[13] ^ dIn[16] ^ dIn[17] ^ dIn[19];
    crc_r[3] = crc_in_s[0] ^ crc_in_s[1] ^ crc_in_s[3] ^ crc_in_s[5] ^ crc_in_s[7] ^ crc_in_s[8] ^ dIn[0] ^ dIn[4] ^ dIn[5] ^ dIn[8] ^ dIn[9] ^ dIn[10] ^ dIn[11] ^ dIn[14] ^ dIn[15] ^ dIn[17] ^ dIn[19] ^ dIn[21] ^ dIn[22];
    crc_r[4] = crc_in_s[2] ^ crc_in_s[5] ^ crc_in_s[7] ^ dIn[0] ^ dIn[1] ^ dIn[2] ^ dIn[3] ^ dIn[7] ^ dIn[8] ^ dIn[12] ^ dIn[16] ^ dIn[19] ^ dIn[21];
    crc_r[5] = crc_in_s[1] ^ crc_in_s[3] ^ crc_in_s[4] ^ crc_in_s[5] ^ crc_in_s[7] ^ dIn[0] ^ dIn[1] ^ dIn[4] ^ dIn[5] ^ dIn[6] ^ dIn[7] ^ dIn[10] ^ dIn[11] ^ dIn[13] ^ dIn[15] ^ dIn[17] ^ dIn[18] ^ dIn[19] ^ dIn[21];
    crc_r[6] = crc_in_s[0] ^ crc_in_s[1] ^ crc_in_s[2] ^ crc_in_s[7] ^ dIn[0] ^ dIn[1] ^ dIn[3] ^ dIn[9] ^ dIn[10] ^ dIn[12] ^ dIn[14] ^ dIn[15] ^ dIn[16] ^ dIn[21];
    crc_r[7] = crc_in_s[2] ^ crc_in_s[3] ^ crc_in_s[4] ^ crc_in_s[5] ^ crc_in_s[6] ^ crc_in_s[7] ^ dIn[0] ^ dIn[1] ^ dIn[3] ^ dIn[4] ^ dIn[5] ^ dIn[6] ^ dIn[7] ^ dIn[8] ^ dIn[9] ^ dIn[13] ^ dIn[16] ^ dIn[17] ^ dIn[18] ^ dIn[19] ^ dIn[20] ^ dIn[21];
    crc_r[8] = crc_in_s[0] ^ crc_in_s[3] ^ crc_in_s[4] ^ crc_in_s[5] ^ crc_in_s[6] ^ crc_in_s[7] ^ crc_in_s[8] ^ dIn[1] ^ dIn[2] ^ dIn[4] ^ dIn[5] ^ dIn[6] ^ dIn[7] ^ dIn[8] ^ dIn[9] ^ dIn[10] ^ dIn[14] ^ dIn[17] ^ dIn[18] ^ dIn[19] ^ dIn[20] ^ dIn[21] ^ dIn[22];

    crc_word = 0x000;
    if (reverse == 1){
        for(int i = 0; i < 9; i++) {
            crc_word = ( crc_word | (crc_r[8-i] << i));
        }
    }
    else{
        for(int i = 0; i < 9; i++) {
            crc_word = ( crc_word | (crc_r[i] << i) );
        }
    }
    return (crc_word);
}   

void  gFexInputByteStreamTool::undoMLE(int &datumPtr ) const{
    // limit input to 12 bits to avoid accidental sign extension
    int din = (0x00000FFF &  datumPtr );
    int dout = 0; 
  
    int FPGA_CONVLIN_TH1 = 5; 
    int FPGA_CONVLIN_TH2 = 749;
    int FPGA_CONVLIN_TH3 = 1773;
    int FPGA_CONVLIN_TH4 = 2541;
    int FPGA_CONVLIN_TH5 = 4029;

    int FPGA_CONVLIN_OF0 = -5072;
    int FPGA_CONVLIN_OF1 = -2012;
    int FPGA_CONVLIN_OF2 = -1262;
    int FPGA_CONVLIN_OF3 = -3036;
    int FPGA_CONVLIN_OF4 = -8120;
    int FPGA_CONVLIN_OF5 = -4118720;

    int oth0 = 0;
    int oth1 = 0;
    int oth2 = 0;
    int oth3 = 0;
    int oth4 = 0;
    int oth5 = 0;
  
    int r1shv = 0;
    int r2shv = 0;
    int r3shv = 0;
    int r4shv = 0;
    int r5shv = 0;
    int r6shv = 0;
    // int trxv = 0;

    int r1conv = 0;
    int r2conv = 0;
    int r3conv = 0;
    int r4conv = 0;
    int r5conv = 0;
    int r6conv = 0;
    // int r3offs = 0;

    r1shv = ((din & 0x0000007F) << 9 )  & 0x0000FE00 ;
    r2shv = ((din & 0x00000FFF) << 1 )  & 0x00001FFE ;
    r3shv = (din &  0x00000FFF) ;
    r4shv = ((din & 0x00000FFF) << 1 )  & 0x00001FFE ;
    r5shv = ((din & 0x00000FFF) << 2 )  & 0x00003FFC ;
    r6shv = ((din & 0x00000FFF) << 10 ) & 0x003FFC00 ;

    r1conv =  r1shv + FPGA_CONVLIN_OF0;
    r2conv =  r2shv + FPGA_CONVLIN_OF1;
    r3conv =  r3shv + FPGA_CONVLIN_OF2;
    r4conv =  r4shv + FPGA_CONVLIN_OF3;
    r5conv =  r5shv + FPGA_CONVLIN_OF4;
    r6conv =  r6shv + FPGA_CONVLIN_OF5;

    if( din > 0 ) {
        oth0 = 1;
    }
    else{
        oth0 = 0; 
    }
    if ( din > FPGA_CONVLIN_TH1 ){
        oth1 = 1;
    }
    else{
        oth1 = 0; 
    }
    if ( din > FPGA_CONVLIN_TH2 ){
        oth2 = 1;
    }else{
     oth2 = 0; 
    }
    if ( din > FPGA_CONVLIN_TH3 ){
        oth3 = 1;
    }else{
        oth3 = 0; 
    }
    if ( din > FPGA_CONVLIN_TH4 ){
        oth4 = 1;
    }else{
        oth4 = 0; 
    }
    if ( din > FPGA_CONVLIN_TH5 ){
        oth5 = 1;
    }
    else{
        oth5 = 0; 
    }

    // divide by 2 to 50 MeV LSB

    if( (! oth0) & (! oth1 ) & (! oth2 ) & (! oth3 ) &  (! oth4 ) & (! oth5 )  ) {
        dout = 0;
    } 
    else if( ( oth0) & (! oth1 ) & (! oth2 ) & (! oth3 ) &  (! oth4 ) & (! oth5 )  ) {
        dout =  r1conv/2;
    } 
    else if( ( oth0) & (  oth1 ) & (! oth2 ) & (! oth3 ) &  (! oth4 ) & (! oth5 )  ) {
        dout = r2conv/2;
    } 
    else if( ( oth0) & (  oth1 ) & ( oth2 ) & (! oth3 ) &  (! oth4 ) & (! oth5 )  ) {
        dout = r3conv/2;
    }  
    else if( ( oth0) & (  oth1 ) & (  oth2 ) & ( oth3 ) &  (! oth4 ) & (! oth5 )  ) {
        dout = r4conv/2;
    }  
    else if( ( oth0) & (  oth1 ) & (  oth2 ) & ( oth3 ) &  (  oth4 ) & (! oth5 )  ) {
        dout = r5conv/2;
    }  
    else if( ( oth0) & (  oth1 ) & (  oth2 ) & ( oth3 ) &  (  oth4 ) & (  oth5 )  ) {
        dout = r6conv/2;
    } 
    else {
        dout = 0; 
    }

    datumPtr = dout;
}

void gFexInputByteStreamTool::gtRescale(gtFPGA twr, gtFPGA &twrScaled, int scale) const{
    int rows = twr.size();
    int cols = twr[0].size();
    for(int irow=0; irow<rows; irow++){
        for( int icolumn=0; icolumn<cols; icolumn++){
            twrScaled[irow][icolumn] = twr[irow][icolumn]/scale; 
        }
    }
}

void gFexInputByteStreamTool::getEtaPhi ( float &Eta, float &Phi, int iEta, int iPhi, int gFEXtowerID) const{
    
    float s_centralPhiWidth = (2*M_PI)/32; //In central region, gFex has 32 bins in phi
    float s_forwardPhiWidth = (2*M_PI)/16; //In forward region, gFex has 16 bins in phi (before rearranging bins)

    const std::vector<float> s_EtaCenter = { -4.7, -4.2, -3.7, -3.4, -3.2, -3, 
                                             -2.8, -2.6, -2.35, -2.1, -1.9, -1.7, -1.5, -1.3, -1.1, -0.9,  
                                             -0.7, -0.5, -0.3, -0.1, 0.1, 0.3, 0.5, 0.7, 0.9, 1.1,                                                 
                                             1.3, 1.5, 1.7, 1.9, 2.1, 2.35, 2.6, 2.8, 3.0,
                                             3.2, 3.4, 3.7, 4.2, 4.7};

    // Transform Eta and Phi indices for the most forward towers into the "original" indices, 
    // as before rearranging the towers such that the forward region is 12(ieta)x32(iphi).
    // The FPGA-C has now the same format (12*32) as FPGA-A and FPGA-B. 
    // This is the result of a transformation in the firmware.
    // Note that for the most forward towers, the Phi index and Eta index have been considered accordingly, 
    // so in order to get the correct float values of Phi and Eta we need to retrieve the "original" indices.
    int towerID_base = 20000;
    int iEtaOld=0, iPhiOld=0;

    if (iEta == 2){
        if (iPhi == ((gFEXtowerID - towerID_base)/24)*2){
            iEtaOld = 0;
            iPhiOld = iPhi/2;
        }
        if (iPhi == (((gFEXtowerID - towerID_base - 12)/24)*2) + 1){
            iEtaOld = 1;
            iPhiOld = (iPhi-1)/2;
        }
    }

    else if (iEta == 3){
        if (iPhi == ((gFEXtowerID - towerID_base - 1)/24)*2){
            iEtaOld = 2;
            iPhiOld = iPhi/2;
        }
        if (iPhi == (((gFEXtowerID - towerID_base - 13)/24)*2) + 1){
            iEtaOld = 3;
            iPhiOld = (iPhi-1)/2;
        }
    }

    else if (iEta == 36){
        if (iPhi == (((gFEXtowerID - towerID_base - 22)/24)*2) + 1){
            iEtaOld = 36;
            iPhiOld = (iPhi-1)/2;
        }
        if (iPhi == ((gFEXtowerID - towerID_base - 10)/24)*2){
            iEtaOld = 37;
            iPhiOld = iPhi/2;
        }
    }

    else if (iEta == 37){
        if (iPhi == (((gFEXtowerID - towerID_base - 23)/24)*2) + 1){
            iEtaOld = 38;
            iPhiOld = (iPhi-1)/2;
        }
        if (iPhi == ((gFEXtowerID - towerID_base - 11)/24)*2){
            iEtaOld = 39;
            iPhiOld = iPhi/2;
        }
    }

    else {
        iEtaOld = iEta;
        iPhiOld = iPhi;
    }
    
    Eta = s_EtaCenter[iEtaOld]; 

    float Phi_gFex = -99;

    if (( iEtaOld <= 3 ) || ( (iEtaOld >= 36) )){
       Phi_gFex = ( (iPhiOld * s_forwardPhiWidth) + s_forwardPhiWidth/2);
    }  
    else {
       Phi_gFex = ( (iPhiOld * s_centralPhiWidth) + s_centralPhiWidth/2);
    }
   
    if (Phi_gFex < M_PI) {
       Phi = Phi_gFex;
    }
    else {
       Phi = (Phi_gFex - 2*M_PI);
    }
}

/// xAOD->BS conversion
StatusCode gFexInputByteStreamTool::convertToBS(std::vector<WROBF*>& /*vrobf*/, const EventContext& /*eventContext*/) {
    
    return StatusCode::SUCCESS;
}

void  gFexInputByteStreamTool::printError(const std::string& location, const std::string& title, MSG::Level type, const std::string& detail) const{
    
    if(m_UseMonitoring){
        Monitored::Group(m_monTool,
                     Monitored::Scalar("gfexDecoderErrorLocation",location.empty() ? std::string("UNKNOWN") : location),
                     Monitored::Scalar("gfexDecoderErrorTitle"   ,title.empty()    ? std::string("UNKNOWN") : title)
                     );
    }
    else {
        msg() << type << detail << endmsg;
    }
}
