/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


#ifndef L1CALOFEXBYTESTREAM_GFEXPOS_H
#define L1CALOFEXBYTESTREAM_GFEXPOS_H

#include <cstdint>

namespace LVL1::gFEXPos {

        // Header word constants
        constexpr unsigned int BLOCK_TYPE_BIT      = 28 ;
        constexpr unsigned int BLOCK_TYPE_MASK     = 0xf ;
        constexpr unsigned int HEADER_SIZE_BIT     = 22 ;
        constexpr unsigned int HEADER_SIZE_MASK    = 0x3 ;
        constexpr unsigned int ERROR_FLAG_BIT      = 12 ;
        constexpr unsigned int ERROR_FLAG_MASK     = 0x1 ;
        constexpr unsigned int DATA_SIZE_MASK      = 0xfff ;

        constexpr unsigned int  WORDS_PER_SLICE    = 14 ;// Number of words per slice is always 14

        
        //The following definitions reflect the position of the 32-bits words in the gFEX bytestream
        
        constexpr std::array<unsigned int, 2> TRAILER_POSITION        = { 6, 13 };//position of trailer in Jet/Global TOB dataframe, wihin he same slice 
        constexpr std::array<unsigned int, 5> JET_UNUSED_POSITION     = { 0, 4, 5, 11, 12 };//position of unused word in Jet TOB dataframe, wihin he same slice 
        constexpr std::array<unsigned int, 4> GLOBAL_UNUSED_POSITION  = { 4, 5, 11, 12 };//position of unused word in Global TOB dataframe, wihin he same slice 

        constexpr std::array<unsigned int, 4> GBLOCK_POSITION         = { 1, 2, 8, 9 };//position of gBlocks word in Jet TOB dataframe, within the same slice
        constexpr std::array<unsigned int, 2> GJET_POSITION           = { 3, 10 };//position of gBlocks word in Jet TOB dataframe, within the same slice
        constexpr unsigned int GRHO_POSITION                          = 7 ;//position of gRho word in Jet TOB dataframe, within the same slice
 
        constexpr unsigned int JWOJ_MHT_POSITION    = 0 ;//position of JwoJ MHT word in Global TOB dataframe, within the same slice
        constexpr unsigned int JWOJ_MST_POSITION    = 1 ;//position of JwoJ MST word in Global TOB dataframe, within the same slice
        constexpr unsigned int JWOJ_MET_POSITION    = 2 ;//position of JwoJ MET word in Global TOB dataframe, within the same slice
        constexpr unsigned int JWOJ_SCALAR_POSITION = 3 ;//position of JwoJ Scalar word in Global TOB dataframe, within the same slice

        constexpr unsigned int NC_MET_POSITION      = 7 ;//position of Noise Cut MET word in Global TOB dataframe, within the same slice
        constexpr unsigned int NC_SCALAR_POSITION   = 9 ;//position of Noise Cut Scalar word in Global TOB dataframe, within the same slice
 
        constexpr unsigned int RMS_MET_POSITION     = 8 ;//position of Rho+RMS MET word in Global TOB dataframe, within the same slice
        constexpr unsigned int RMS_SCALAR_POSITION  = 10;//position of Rho+RMS Scalar word in Global TOB dataframe, within the same slice

        constexpr unsigned int GLOBAL_X_MASK = 0x7FFF; // 16 bit mask to extract the x component
        constexpr unsigned int GLOBAL_Y_MASK = 0x7FFF; // 16 bit mask to extract the y component
        constexpr unsigned int GLOBAL_X_BIT = 16; // bit position to extract the x and y components
        constexpr unsigned int GLOBAL_Y_BIT = 0; // bit position to extract the x and y components
        constexpr unsigned int GLOBAL_BIT_TRUNCATION = 4; // truncation

        constexpr uint32_t FPGA_A_INPUT_HEADER  = 0xa14002bc;
        constexpr uint32_t FPGA_B_INPUT_HEADER  = 0xb14002bc;
        constexpr uint32_t FPGA_C_INPUT_HEADER  = 0xc14002bc;

        constexpr int MAX_FIBERS   = 100; //maximum number of fibers in A, B and C
        constexpr int AB_FIBERS    = 80;  //Number of fibers A and B
        constexpr int C_FIBERS     = 50;  //Number of fibers C
        constexpr int MAX_E_FIELDS = 16;  //maximum number of gCaloTowers per fiber
        constexpr int MAX_FIELDS   = 20;  //maximum number of fields on fiber including BCID, etc.
        constexpr int ABC_ROWS     = 32;  //Number of rows in FPGA A, B, C
        constexpr int AB_COLUMNS   = 12;  //Number of columns in FPGA A and B
        constexpr int AB_TOWERS    = 384; //Total towers in FPGA => 12*32 = 384 towers
        constexpr int W280         = 7;   //Number of 32 bit words per clock

        constexpr int FINE_CEILING  =  255; //ceiling value used for 50 MeV gTowers
        constexpr int FINE_FLOOR    = -256; //floor value used for 50 MeV gTowers



        constexpr std::array<unsigned int, 80>  GTRX_MAP_A_IND =  
        {
          88, 89, 93, 92, 94, 95, 90, 96, 97, 98, 99, 91, 76, 77, 79, 78,
          80, 81, 82, 83, 87, 84, 86, 85, 64, 65, 67, 66, 68, 69, 70, 71,
          75, 72, 74, 73, 52, 53, 55, 54, 56, 57, 58, 59, 63, 60, 62, 61,
          10,  9, 11,  8,  7,  6,  4,  5,  0,  3,  1,  2, 22, 21, 23, 20,
          34, 33, 35, 32, 31, 30, 28, 29, 24, 27, 25, 26, 47, 44, 45, 46
        };

        constexpr std::array<unsigned int, 80>  GTRX_MAP_B_IND = 
        {
          88, 89, 93, 92, 94, 95, 90, 96, 91, 98, 99, 97, 76, 77, 79, 78,
          80, 81, 82, 83, 87, 84, 86, 85, 64, 65, 67, 66, 68, 69, 70, 71,
          75, 72, 74, 73, 52, 53, 55, 54, 56, 57, 58, 59, 63, 60, 62, 61,
          39, 47, 38, 45, 46, 44, 42, 43, 36, 41, 37, 40, 34, 33, 35, 32,
          22, 21, 23, 20, 19, 18, 16, 17, 12, 15, 13, 14, 10,  9, 11,  8
        };

        constexpr std::array<unsigned int, 50>  GTRX_MAP_C_IND =  
        {
          39, 38, 47, 45, 46,
          43, 41, 36, 42, 37, 40, 34, 33,
          22, 21, 20, 19,
          16, 17, 12, 15, 13, 14, 10, 9,
          88, 89, 90, 91, 92,
          94, 95, 96, 97, 98, 99, 76, 77,
          64, 65, 67, 68,
          70, 71, 72, 73, 74, 75, 52, 53
        };


        constexpr std::array<int, 100> AMPD_NFI = 
        {
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,
          1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2
        };

        constexpr std::array<int, 100> ACALO_TYPE = 
        {
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,
          1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2
        };


        constexpr std::array<std::array<int, 16>, 100> AMPD_GTRN_ARR =
        {{
          {  3,  3,  1,  1, 15, 15, 13, 13,  2,  2,  0,  0, 12, 12, 14, 14}, //  f0  -- EMECC_1
          { 27, 27, 25, 25, 39, 39, 37, 37, 26, 26, 24, 24, 36, 36, 38, 38}, //  f1  -- EMECC_2
          { 51, 51, 49, 49, 63, 63, 61, 61, 50, 50, 48, 48, 60, 60, 62, 62}, //  f2  -- EMECC_3
          { 75, 75, 73, 73, 87, 87, 85, 85, 74, 74, 72, 72, 84, 84, 86, 86}, //  f3  -- EMECC_4
          { 99, 99, 97, 97,111,111,109,109, 98, 98, 96, 96,108,108,110,110}, //  f4  -- EMECC_5
          {123,123,121,121,135,135,133,133,122,122,120,120,132,132,134,134}, //  f5  -- EMECC_6
          {147,147,145,145,159,159,157,157,146,146,144,144,156,156,158,158}, //  f6  -- EMECC_7
          {171,171,169,169,183,183,181,181,170,170,168,168,180,180,182,182}, //  f7  -- EMECC_8
          {  7,  7,  5,  5, 19, 19, 17, 17,  6,  6,  4,  4, 16, 16, 18, 18}, //  f8  -- EMBC-EMECC_1
          { 31, 31, 29, 29, 43, 43, 41, 41, 30, 30, 28, 28, 40, 40, 42, 42}, //  f9  -- EMBC-EMECC_2
          { 55, 55, 53, 53, 67, 67, 65, 65, 54, 54, 52, 52, 64, 64, 66, 66}, //  f10 -- EMBC-EMECC_3
          { 79, 79, 77, 77, 91, 91, 89, 89, 78, 78, 76, 76, 88, 88, 90, 90}, //  f11 -- EMBC-EMECC_4
          {103,103,101,101,115,115,113,113,102,102,100,100,112,112,114,114}, //  f12 -- EMBC-EMECC_5
          {127,127,125,125,139,139,137,137,126,126,124,124,136,136,138,138}, //  f13 -- EMBC-EMECC_6
          {151,151,149,149,163,163,161,161,150,150,148,148,160,160,162,162}, //  f14 -- EMBC-EMECC_7
          {175,175,173,173,187,187,185,185,174,174,172,172,184,184,186,186}, //  f15 -- EMBC-EMECC_8
          { 11, 11,  9,  9, 23, 23, 21, 21, 10, 10,  8,  8, 20, 20, 22, 22}, //  f16 -- EMBC_1
          { 35, 35, 33, 33, 47, 47, 45, 45, 34, 34, 32, 32, 44, 44, 46, 46}, //  f17 -- EMBC_2
          { 59, 59, 57, 57, 71, 71, 69, 69, 58, 58, 56, 56, 68, 68, 70, 70}, //  f18 -- EMBC_3
          { 83, 83, 81, 81, 95, 95, 93, 93, 82, 82, 80, 80, 92, 92, 94, 94}, //  f19 -- EMBC_4
          {107,107,105,105,119,119,117,117,106,106,104,104,116,116,118,118}, //  f20 -- EMBC_5
          {131,131,129,129,143,143,141,141,130,130,128,128,140,140,142,142}, //  f21 -- EMBC_6
          {155,155,153,153,167,167,165,165,154,154,152,152,164,164,166,166}, //  f22 -- EMBC_7
          {179,179,177,177,191,191,189,189,178,178,176,176,188,188,190,190}, //  f23 -- EMBC_8
          {195,195,193,193,207,207,205,205,194,194,192,192,204,204,206,206}, //  f24 -- EMECC_9
          {219,219,217,217,231,231,229,229,218,218,216,216,228,228,230,230}, //  f25 -- EMECC_10
          {243,243,241,241,255,255,253,253,242,242,240,240,252,252,254,254}, //  f26 -- EMECC_11
          {267,267,265,265,279,279,277,277,266,266,264,264,276,276,278,278}, //  f27 -- EMECC_12
          {291,291,289,289,303,303,301,301,290,290,288,288,300,300,302,302}, //  f28 -- EMECC_13
          {315,315,313,313,327,327,325,325,314,314,312,312,324,324,326,326}, //  f29 -- EMECC_14
          {339,339,337,337,351,351,349,349,338,338,336,336,348,348,350,350}, //  f30 -- EMECC_15
          {363,363,361,361,375,375,373,373,362,362,360,360,372,372,374,374}, //  f31 -- EMECC_16
          {199,199,197,197,211,211,209,209,198,198,196,196,208,208,210,210}, //  f32 -- EMBC-EMECC_9
          {223,223,221,221,235,235,233,233,222,222,220,220,232,232,234,234}, //  f33 -- EMBC-EMECC_10
          {247,247,245,245,259,259,257,257,246,246,244,244,256,256,258,258}, //  f34 -- EMBC-EMECC_11
          {271,271,269,269,283,283,281,281,270,270,268,268,280,280,282,282}, //  f35 -- EMBC-EMECC_12
          {295,295,293,293,307,307,305,305,294,294,292,292,304,304,306,306}, //  f36 -- EMBC-EMECC_13
          {319,319,317,317,331,331,329,329,318,318,316,316,328,328,330,330}, //  f37 -- EMBC-EMECC_14
          {343,343,341,341,355,355,353,353,342,342,340,340,352,352,354,354}, //  f38 -- EMBC-EMECC_15
          {367,367,365,365,379,379,377,377,366,366,364,364,376,376,378,378}, //  f39 -- EMBC-EMECC_16
          {203,203,201,201,215,215,213,213,202,202,200,200,212,212,214,214}, //  f40 -- EMBC_9
          {227,227,225,225,239,239,237,237,226,226,224,224,236,236,238,238}, //  f41 -- EMBC_10
          {251,251,249,249,263,263,261,261,250,250,248,248,260,260,262,262}, //  f42 -- EMBC_11
          {275,275,273,273,287,287,285,285,274,274,272,272,284,284,286,286}, //  f43 -- EMBC_12
          {299,299,297,297,311,311,309,309,298,298,296,296,308,308,310,310}, //  f44 -- EMBC_13
          {323,323,321,321,335,335,333,333,322,322,320,320,332,332,334,334}, //  f45 -- EMBC_14
          {347,347,345,345,359,359,357,357,346,346,344,344,356,356,358,358}, //  f46 -- EMBC_15
          {371,371,369,369,383,383,381,381,370,370,368,368,380,380,382,382}, //  f47 -- EMBC_16
          {  4,  5, 17, 16, 28, 29, 41, 40, 52, 53, 65, 64, 76, 77, 89, 88}, //  f48 -- TREXC_4_1
          {  6,  7, 19, 18, 30, 31, 43, 42, 54, 55, 67, 66, 78, 79, 91, 90}, //  f49 -- TREXC_5_1
          {  8,  9, 21, 20, 32, 33, 45, 44, 56, 57, 69, 68, 80, 81, 93, 92}, //  f50 -- TREXC_6_1
          { 10, 11, 23, 22, 34, 35, 47, 46, 58, 59, 71, 70, 82, 83, 95, 94}, //  f51 -- TREXC_7_1
          {100,101,113,112,124,125,137,136,148,149,161,160,172,173,185,184}, //  f52 -- TREXC_4_2
          {102,103,115,114,126,127,139,138,150,151,163,162,174,175,187,186}, //  f53 -- TREXC_5_2
          {104,105,117,116,128,129,141,140,152,153,165,164,176,177,189,188}, //  f54 -- TREXC_6_2
          {106,107,119,118,130,131,143,142,154,155,167,166,178,179,191,190}, //  f55 -- TREXC_7_2
          {  0,  0,  3,  2,  1,  0,  0,  1,  1, 15, 14, 13, 12,  1, -1, -1}, //  f56 -- EMECC-HECC_1
          {  2,  2, 27, 26, 25, 24,  2,  3,  3, 39, 38, 37, 36,  3, -1, -1}, //  f57 -- EMECC-HECC_1
          {  4,  4, 51, 50, 49, 48,  4,  5,  5, 63, 62, 61, 60,  5, -1, -1}, //  f58 -- EMECC-HECC_2
          {  6,  6, 75, 74, 73, 72,  6,  7,  7, 87, 86, 85, 84,  7, -1, -1}, //  f59 -- EMECC-HECC_2
          {  8,  8, 99, 98, 97, 96,  8,  9,  9,111,110,109,108,  9, -1, -1}, //  f60 -- EMECC-HECC_3
          { 10, 10,123,122,121,120, 10, 11, 11,135,134,133,132, 11, -1, -1}, //  f61 -- EMECC-HECC_3
          { 12, 12,147,146,145,144, 12, 13, 13,159,158,157,156, 13, -1, -1}, //  f62 -- EMECC-HECC_4
          { 14, 14,171,170,169,168, 14, 15, 15,183,182,181,180, 15, -1, -1}, //  f63 -- EMECC-HECC_4
          {196,197,209,208,220,221,233,232,244,245,257,256,268,269,281,280}, //  f64 -- TREXC_4_3
          {198,199,211,210,222,223,235,234,246,247,259,258,270,271,283,282}, //  f65 -- TREXC_5_3
          {200,201,213,212,224,225,237,236,248,249,261,260,272,273,285,284}, //  f66 -- TREXC_6_3
          {202,203,215,214,226,227,239,238,250,251,263,262,274,275,287,286}, //  f67 -- TREXC_7_3
          {292,293,305,304,316,317,329,328,340,341,353,352,364,365,377,376}, //  f68 -- TREXC_4_4
          {294,295,307,306,318,319,331,330,342,343,355,354,366,367,379,378}, //  f69 -- TREXC_5_4
          {296,297,309,308,320,321,333,332,344,345,357,356,368,369,381,380}, //  f70 -- TREXC_6_4
          {298,299,311,310,322,323,335,334,346,347,359,358,370,371,383,382}, //  f71 -- TREXC_7_4
          { 16, 16,195,194,193,192, 16, 17, 17,207,206,205,204, 17, -1, -1}, //  f72 -- EMECC-HECC_5
          { 18, 18,219,218,217,216, 18, 19, 19,231,230,229,228, 19, -1, -1}, //  f73 -- EMECC-HECC_5
          { 20, 20,243,242,241,240, 20, 21, 21,255,254,253,252, 21, -1, -1}, //  f74 -- EMECC-HECC_6
          { 22, 22,267,266,265,264, 22, 23, 23,279,278,277,276, 23, -1, -1}, //  f75 -- EMECC-HECC_6
          { 24, 24,291,290,289,288, 24, 25, 25,303,302,301,300, 25, -1, -1}, //  f76 -- EMECC-HECC_7
          { 26, 26,315,314,313,312, 26, 27, 27,327,326,325,324, 27, -1, -1}, //  f77 -- EMECC-HECC_7
          { 28, 28,339,338,337,336, 28, 29, 29,351,350,349,348, 29, -1, -1}, //  f78 -- EMECC-HECC_8
          { 30, 30,363,362,361,360, 30, 31, 31,375,374,373,372, 31, -1, -1}, //  f79 -- EMECC-HECC_8
        }};

        constexpr std::array<std::array<int, 20>, 4> AMPD_DSTRT_ARR =
        {{          
          {11,15,27,31,75,79,91,95,107,111,139,143,155,159,171,175,199,207,214,223}, //f0 - f47  EMEC,EMB
          {11,23,35,47,59,71,83,95,107,119,131,143,155,167,179,191,199,207,214,223}, 
          {11,23,35,47,59,71,83,95,107,119,131,143,155,167,179,191,199,207,214,223},  // f56 - f63, f72 - f79 EMECHEC
          {11,23,35,47,59,71,83,95,107,119,131,143,155,167,179,191,199,207,214,223}  // unsued    
        }};

        constexpr std::array<std::array<char, 20>, 4> AMPD_DTYP_ARR = 
        {{
  
          {0b0000,0b0100,0b0000,0b0100,0b0000,0b0100,0b0000,0b0100,0b0000,0b0100,0b0000,0b0100,0b0000,0b0100,0b0000,0b0100,
           0b1111,0b1000,0b1010,0b0101}, //f0      EMEC,EMB
   
          {0b0001,0b0001,0b0001,0b0001,0b0001,0b0001,0b0001,0b0001,0b0001,0b0001,0b0001,0b0001,0b0001,0b0001,0b0001,0b0001,
           0b1111, 0b1111,0b1010,0b0101}, //f48/f64 TREX
   
          {0b0010, 0b0110, 0b1011, 0b1011, 0b1011, 0b1011, 0b0011 ,  
           0b0010, 0b0110, 0b1011, 0b1011, 0b1011, 0b1011, 0b0011 , 
           0b1111, 0b1111, 
           0b1111, 0b1000, 0b1010,0b0101} //f56 EMEC/HEC
 
        }};


        constexpr std::array<int, 100> AMSK =
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};


        constexpr std::array<int, 100> BMPD_NFI = 
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,
         1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2};


        constexpr std::array<int, 100> BCALO_TYPE = 
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,
         1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2};


        constexpr std::array<std::array<int, 16>, 100> BMPD_GTRN_ARR =
        {{
          {  8,  8, 10, 10, 20, 20, 22, 22,  9,  9, 11, 11, 23, 23, 21, 21},  // EMECA_1 
          { 32, 32, 34, 34, 44, 44, 46, 46, 33, 33, 35, 35, 47, 47, 45, 45},  //EMECA_2 
          { 56, 56, 58, 58, 68, 68, 70, 70, 57, 57, 59, 59, 71, 71, 69, 69},  //EMECA_3 
          { 80, 80, 82, 82, 92, 92, 94, 94, 81, 81, 83, 83, 95, 95, 93, 93},  //EMECA_4 
          {104,104,106,106,116,116,118,118,105,105,107,107,119,119,117,117},  //EMECA_5 
          {128,128,130,130,140,140,142,142,129,129,131,131,143,143,141,141},  //EMECA_6 
          {152,152,154,154,164,164,166,166,153,153,155,155,167,167,165,165},  //EMECA_7 
          {176,176,178,178,188,188,190,190,177,177,179,179,191,191,189,189},  //EMECA_8 
          {  4,  4,  6,  6, 16, 16, 18, 18,  5,  5,  7,  7, 19, 19, 17, 17},  //EMBA-EMECA_1 
          { 28, 28, 30, 30, 40, 40, 42, 42, 29, 29, 31, 31, 43, 43, 41, 41},  //EMBA-EMECA_2 
          { 52, 52, 54, 54, 64, 64, 66, 66, 53, 53, 55, 55, 67, 67, 65, 65},  //EMBA-EMECA_3 
          { 76, 76, 78, 78, 88, 88, 90, 90, 77, 77, 79, 79, 91, 91, 89, 89},  //EMBA-EMECA_4 
          {100,100,102,102,112,112,114,114,101,101,103,103,115,115,113,113},  //EMBA-EMECA_5 
          {124,124,126,126,136,136,138,138,125,125,127,127,139,139,137,137},  //EMBA-EMECA_6 
          {148,148,150,150,160,160,162,162,149,149,151,151,163,163,161,161},  //EMBA-EMECA_7 
          {172,172,174,174,184,184,186,186,173,173,175,175,187,187,185,185},  //EMBA-EMECA_8 
          {  0,  0,  2,  2, 12, 12, 14, 14,  1,  1,  3,  3, 15, 15, 13, 13},  //EMBA_1 
          { 24, 24, 26, 26, 36, 36, 38, 38, 25, 25, 27, 27, 39, 39, 37, 37},  //EMBA_2 
          { 48, 48, 50, 50, 60, 60, 62, 62, 49, 49, 51, 51, 63, 63, 61, 61},  //EMBA_3 
          { 72, 72, 74, 74, 84, 84, 86, 86, 73, 73, 75, 75, 87, 87, 85, 85},  //EMBA_4 
          { 96, 96, 98, 98,108,108,110,110, 97, 97, 99, 99,111,111,109,109},  //EMBA_5 
          {120,120,122,122,132,132,134,134,121,121,123,123,135,135,133,133},  //EMBA_6 
          {144,144,146,146,156,156,158,158,145,145,147,147,159,159,157,157},  //EMBA_7 
          {168,168,170,170,180,180,182,182,169,169,171,171,183,183,181,181},  //EMBA_8 
          {200,200,202,202,212,212,214,214,201,201,203,203,215,215,213,213},  //EMECA_9 
          {224,224,226,226,236,236,238,238,225,225,227,227,239,239,237,237},  //EMECA_10 
          {248,248,250,250,260,260,262,262,249,249,251,251,263,263,261,261},  //EMECA_11 
          {272,272,274,274,284,284,286,286,273,273,275,275,287,287,285,285},  //EMECA_12 
          {296,296,298,298,308,308,310,310,297,297,299,299,311,311,309,309},  //EMECA_13 
          {320,320,322,322,332,332,334,334,321,321,323,323,335,335,333,333},  //EMECA_14 
          {344,344,346,346,356,356,358,358,345,345,347,347,359,359,357,357},  //EMECA_15 
          {368,368,370,370,380,380,382,382,369,369,371,371,383,383,381,381},  //EMECA_16 
          {196,196,198,198,208,208,210,210,197,197,199,199,211,211,209,209},  //EMBA-EMECA_9 
          {220,220,222,222,232,232,234,234,221,221,223,223,235,235,233,233},  //EMBA-EMECA_10 
          {244,244,246,246,256,256,258,258,245,245,247,247,259,259,257,257},  //EMBA-EMECA_11 
          {268,268,270,270,280,280,282,282,269,269,271,271,283,283,281,281},  //EMBA-EMECA_12 
          {292,292,294,294,304,304,306,306,293,293,295,295,307,307,305,305},  //EMBA-EMECA_13 
          {316,316,318,318,328,328,330,330,317,317,319,319,331,331,329,329},  //EMBA-EMECA_14 
          {340,340,342,342,352,352,354,354,341,341,343,343,355,355,353,353},  //EMBA-EMECA_15 
          {364,364,366,366,376,376,378,378,365,365,367,367,379,379,377,377},  //EMBA-EMECA_16 
          {192,192,194,194,204,204,206,206,193,193,195,195,207,207,205,205},  //EMBA_9 
          {216,216,218,218,228,228,230,230,217,217,219,219,231,231,229,229},  //EMBA_10 
          {240,240,242,242,252,252,254,254,241,241,243,243,255,255,253,253},  //EMBA_11 
          {264,264,266,266,276,276,278,278,265,265,267,267,279,279,277,277},  //EMBA_12 
          {288,288,290,290,300,300,302,302,289,289,291,291,303,303,301,301},  //EMBA_13 
          {312,312,314,314,324,324,326,326,313,313,315,315,327,327,325,325},  //EMBA_14 
          {336,336,338,338,348,348,350,350,337,337,339,339,351,351,349,349},  //EMBA_15 
          {360,360,362,362,372,372,374,374,361,361,363,363,375,375,373,373},  //EMBA_16 
          {  6,  7, 19, 18, 30, 31, 43, 42, 54, 55, 67, 66, 78, 79, 91, 90},  //TREXA_B_1 
          {  4,  5, 17, 16, 28, 29, 41, 40, 52, 53, 65, 64, 76, 77, 89, 88},  //TREXA_A_1 
          {  2,  3, 15, 14, 26, 27, 39, 38, 50, 51, 63, 62, 74, 75, 87, 86},  //TREXA_9_1 
          {  0,  1, 13, 12, 24, 25, 37, 36, 48, 49, 61, 60, 72, 73, 85, 84},  //TREXA_8_1 
          {102,103,115,114,126,127,139,138,150,151,163,162,174,175,187,186},  //TREXA_B_2 
          {100,101,113,112,124,125,137,136,148,149,161,160,172,173,185,184},  //TREXA_A_2 
          { 98, 99,111,110,122,123,135,134,146,147,159,158,170,171,183,182},  //TREXA_9_2 
          { 96, 97,109,108,120,121,133,132,144,145,157,156,168,169,181,180},  //TREXA_8_2 
          {  0,  0,  8,  9, 10, 11,  0,  1,  1, 20, 21, 22, 23,  1, -1, -1},  //EMECA-HECA_1 
          {  2,  2, 32, 33, 34, 35,  2,  3,  3, 44, 45, 46, 47,  3, -1, -1},  //EMECA-HECA_1 
          {  4,  4, 56, 57, 58, 59,  4,  5,  5, 68, 69, 70, 71,  5, -1, -1},  //EMECA-HECA_2 
          {  6,  6, 80, 81, 82, 83,  6,  7,  7, 92, 93, 94, 95,  7, -1, -1},  //EMECA-HECA_2 
          {  8,  8,104,105,106,107,  8,  9,  9,116,117,118,119,  9, -1, -1},  //EMECA-HECA_3 
          { 10, 10,128,129,130,131, 10, 11, 11,140,141,142,143, 11, -1, -1},  //EMECA-HECA_3 
          { 12, 12,152,153,154,155, 12, 13, 13,164,165,166,167, 13, -1, -1},  //EMECA-HECA_4 
          { 14, 14,176,177,178,179, 14, 15, 15,188,189,190,191, 15, -1, -1},  //EMECA-HECA_4 
          {198,199,211,210,222,223,235,234,246,247,259,258,270,271,283,282},  //TREXA_B_3 
          {196,197,209,208,220,221,233,232,244,245,257,256,268,269,281,280},  //TREXA_A_3 
          {194,195,207,206,218,219,231,230,242,243,255,254,266,267,279,278},  //TREXA_9_3 
          {192,193,205,204,216,217,229,228,240,241,253,252,264,265,277,276},  //TREXA_8_3 
          {294,295,307,306,318,319,331,330,342,343,355,354,366,367,379,378},  //TREXA_B_4 
          {292,293,305,304,316,317,329,328,340,341,353,352,364,365,377,376},  //TREXA_A_4 
          {290,291,303,302,314,315,327,326,338,339,351,350,362,363,375,374},  //TREXA_9_4 
          {288,289,301,300,312,313,325,324,336,337,349,348,360,361,373,372},  //TREXA_8_4 
          { 16, 16,200,201,202,203, 16, 17, 17,212,213,214,215, 17, -1, -1},  //EMECA-HECA_5 
          { 18, 18,224,225,226,227, 18, 19, 19,236,237,238,239, 19, -1, -1},  //EMECA-HECA_5 
          { 20, 20,248,249,250,251, 20, 21, 21,260,261,262,263, 21, -1, -1},  //EMECA-HECA_6 
          { 22, 22,272,273,274,275, 22, 23, 23,284,285,286,287, 23, -1, -1},  //EMECA-HECA_6 
          { 24, 24,296,297,298,299, 24, 25, 25,308,309,310,311, 25, -1, -1},  //EMECA-HECA_7 
          { 26, 26,320,321,322,323, 26, 27, 27,332,333,334,335, 27, -1, -1},  //EMECA-HECA_7 
          { 28, 28,344,345,346,347, 28, 29, 29,356,357,358,359, 29, -1, -1},  //EMECA-HECA_8 
          { 30, 30,368,369,370,371, 30, 31, 31,380,381,382,383, 31, -1, -1} // EMECA-HECA_8 
    
        }};

        constexpr std::array<std::array<int, 20>, 4> BMPD_DSTRT_ARR= 
        {{
          {11,15,27,31,75,79,91,95,107,111,139,143,155,159,171,175,199,207,214,223}, //f0 - f47  EMEC,EMB
          {11,23,35,47,59,71,83,95,107,119,131,143,155,167,179,191,199,207,214,223}, 
          {11,23,35,47,59,71,83,95,107,119,131,143,155,167,179,191,199,207,214,223},  // f56 - f63, f72 - f79 EMECHEC
          {11,23,35,47,59,71,83,95,107,119,131,143,155,167,179,191,199,207,214,223}  // unsued
    
        }};

        constexpr std::array<std::array<char, 20>, 4> BMPD_DTYP_ARR =
        {{
          {0b0000,0b0100,0b0000,0b0100,0b0000,0b0100,0b0000,0b0100,0b0000,0b0100,0b0000,0b0100,0b0000,0b0100,0b0000,0b0100,
           0b1111,0b1000,0b1010,0b0101}, //f0      EMEC,EMB
   
          {0b0001,0b0001,0b0001,0b0001,0b0001,0b0001,0b0001,0b0001,0b0001,0b0001,0b0001,0b0001,0b0001,0b0001,0b0001,0b0001,
           0b1111, 0b1111,0b1010,0b0101}, //f48/f64 TREX
   
          {0b0010, 0b0110, 0b1011, 0b1011, 0b1011, 0b1011, 0b0011 ,  
           0b0010, 0b0110, 0b1011, 0b1011, 0b1011, 0b1011, 0b0011 , 
           0b1111, 0b1111, 
           0b1111, 0b1000, 0b1010,0b0101} //f56 EMEC/HEC
   
        }};


        constexpr std::array<int, 100> BMSK =
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}; 


        constexpr std::array<int, 100> CMPD_NFI =
        {
          1,1,2,3,3,0,0,0,0,0,0,0,0,1,1,3,
          3,0,0,0,0,0,0,0,0,1,1,2,3,3,0,0,
          0,0,0,0,0,0,1,1,3,3,0,0,0,0,0,0,
          0,0,
          -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  //unsed in C 
          -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  //unsed in C 
          -1,-1,-1,-1,-1,-1,-1,-1,-1,-1   //unsed in C
          -1,-1,-1,-1,-1,-1,-1,-1,-1,-1   //unsed in C
          -1,-1,-1,-1,-1,-1,-1,-1,-1,-1   //unsed in C

        };

        constexpr std::array<int, 100> CCALO_TYPE=     
        {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
          3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
          3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
          3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
          3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
          3,3,3,3};

        constexpr std::array<std::array<int, 16>, 100> CMPD_GTRN_ARR = 
        {{
          { -1, 12, -1, 60, -1, 36, -1, 84,  1,  0, 49, 48, 25, 24, 73, 72}, // f0  -- FCAL1C
          { -1,108, -1,156, -1,132, -1,180, 97, 96,145,144,121,120,169,168}, // f1  -- FCAL1C
          { 13, 61,109,157,205,253,301,349, 37, 85,133,181,229,277,325,373}, // f2  -- FCAL1C
          { 13,  1, 61, 49, 37, 25, 85, 73, 12,  0, 60, 48, 36, 24, 84, 72}, // f3  -- FCAL2C
          {109, 97,157,145,133,121,181,169,108, 96,156,144,132,120,180,168}, // f4  -- FCAL2C
          {  5,  4,  5,  4,  3,  2,  3,  2, 17, 16, 17, 16, 15, 14, 15, 14}, // f5  -- EMECC-HECC_1
          { 29, 28, 29, 28, 27, 26, 27, 26, 41, 40, 41, 40, 39, 38, 39, 38}, // f6  -- EMECC-HECC_1
          { 53, 52, 53, 52, 51, 50, 51, 50, 65, 64, 65, 64, 63, 62, 63, 62}, // f7  -- EMECC-HECC_2
          { 77, 76, 77, 76, 75, 74, 75, 74, 89, 88, 89, 88, 87, 86, 87, 86}, // f8  -- EMECC-HECC_2
          {101,100,101,100, 99, 98, 99, 98,113,112,113,112,111,110,111,110}, // f9  -- EMECC-HECC_3
          {125,124,125,124,123,122,123,122,137,136,137,136,135,134,135,134}, // f10 -- EMECC-HECC_3
          {149,148,149,148,147,146,147,146,161,160,161,160,159,158,159,158}, // f11 -- EMECC-HECC_4
          {173,172,173,172,171,170,171,170,185,184,185,184,183,182,183,182}, // f12 -- EMECC-HECC_4
          { -1,204, -1,252, -1,228, -1,276,193,192,241,240,217,216,265,264}, // f13 -- FCAL1C
          { -1,300, -1,348, -1,324, -1,372,289,288,337,336,313,312,361,360}, // f14 -- FCAL1C
          {205,193,253,241,229,217,277,265,204,192,252,240,228,216,276,264}, // f15 -- FCAL2C
          {301,289,349,337,325,313,373,361,300,288,348,336,324,312,372,360}, // f16 -- FCAL2C
          {197,196,197,196,195,194,195,194,209,208,209,208,207,206,207,206}, // f17 -- EMECC-HECC_5
          {221,220,221,220,219,218,219,218,233,232,233,232,231,230,231,230}, // f18 -- EMECC-HECC_5
          {245,244,245,244,243,242,243,242,257,256,257,256,255,254,255,254}, // f19 -- EMECC-HECC_6
          {269,268,269,268,267,266,267,266,281,280,281,280,279,278,279,278}, // f20 -- EMECC-HECC_6
          {293,292,293,292,291,290,291,290,305,304,305,304,303,302,303,302}, // f21 -- EMECC-HECC_7
          {317,316,317,316,315,314,315,314,329,328,329,328,327,326,327,326}, // f22 -- EMECC-HECC_7
          {341,340,341,340,339,338,339,338,353,352,353,352,351,350,351,350}, // f23 -- EMECC-HECC_8
          {365,364,365,364,363,362,363,362,377,376,377,376,375,374,375,374}, // f24 -- EMECC-HECC_8
          { -1, 23, -1, 71, -1, 47, -1, 95, 10, 11, 58, 59, 34, 35, 82, 83}, // f25 -- FCAL1A
          { -1,119, -1,167, -1,143, -1,191,106,107,154,155,130,131,178,179}, // f26 -- FCAL1A
          { 22, 70,118,166,214,262,310,358, 46, 94,142,190,238,286,334,382}, // f27 -- FCAL1A
          { 22, 10, 70, 58, 46, 34, 94, 82, 23, 11, 71, 59, 47, 35, 95, 83}, // f28 -- FCAL2A
          {118,106,166,154,142,130,190,178,119,107,167,155,143,131,191,179}, // f29 -- FCAL2A
          {  6,  7,  6,  7,  8,  9,  8,  9, 18, 19, 18, 19, 20, 21, 20, 21}, // f30 -- EMECA-HECA_1
          { 30, 31, 30, 31, 32, 33, 32, 33, 42, 43, 42, 43, 44, 45, 44, 45}, // f31 -- EMECA-HECA_1
          { 54, 55, 54, 55, 56, 57, 56, 57, 66, 67, 66, 67, 68, 69, 68, 69}, // f32 -- EMECA-HECA_2
          { 78, 79, 78, 79, 80, 81, 80, 81, 90, 91, 90, 91, 92, 93, 92, 93}, // f33 -- EMECA-HECA_2
          {102,103,102,103,104,105,104,105,114,115,114,115,116,117,116,117}, // f34 -- EMECA-HECA_3
          {126,127,126,127,128,129,128,129,138,139,138,139,140,141,140,141}, // f35 -- EMECA-HECA_3
          {150,151,150,151,152,153,152,153,162,163,162,163,164,165,164,165}, // f36 -- EMECA-HECA_4
          {174,175,174,175,176,177,176,177,186,187,186,187,188,189,188,189}, // f37 -- EMECA-HECA_4
          { -1,215, -1,263, -1,239, -1,287,202,203,250,251,226,227,274,275}, // f38 -- FCAL1A
          { -1,311, -1,359, -1,335, -1,383,298,299,346,347,322,323,370,371}, // f39 -- FCAL1A
          {214,202,262,250,238,226,286,274,215,203,263,251,239,227,287,275}, // f40 -- FCAL2A
          {310,298,358,346,334,322,382,370,311,299,359,347,335,323,383,371}, // f41 -- FCAL2A
          {198,199,198,199,200,201,200,201,210,211,210,211,212,213,212,213}, // f42 -- EMECA-HECA_5
          {222,223,222,223,224,225,224,225,234,235,234,235,236,237,236,237}, // f43 -- EMECA-HECA_5
          {246,247,246,247,248,249,248,249,258,259,258,259,260,261,260,261}, // f44 -- EMECA-HECA_6
          {270,271,270,271,272,273,272,273,282,283,282,283,284,285,284,285}, // f45 -- EMECA-HECA_6
          {294,295,294,295,296,297,296,297,306,307,306,307,308,309,308,309}, // f46 -- EMECA-HECA_7
          {318,319,318,319,320,321,320,321,330,331,330,331,332,333,332,333}, // f47 -- EMECA-HECA_7
          {342,343,342,343,344,345,344,345,354,355,354,355,356,357,356,357}, // f48 -- EMECA-HECA_8
          {366,367,366,367,368,369,368,369,378,379,378,379,380,381,380,381}, // f49 -- EMECA-HECA_8
          
          // total to here 32 + 10 + 8 = 50 
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // unused
          {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1} // unused 
        }};


        constexpr std::array<std::array<int, 20>, 4> CMPD_DSTRT_ARR = 
        {{
          {11,23,35,47,59,71,83,95,107,119,131,143,155,167,179,191,199,207,214,223},
          {11,23,35,47,59,71,83,95,107,119,131,143,155,167,179,191,199,207,214,223},
          {11,23,35,47,59,71,83,95,107,119,131,143,155,167,179,191,199,207,214,223},
          {11,23,35,47,59,71,83,95,107,119,131,143,155,167,179,191,199,207,214,223},
        }};

        constexpr std::array<std::array<char, 20>, 4> CMPD_DTYP_ARR = 
        {{
          {0b0010,0b0010,0b0011,0b0011,0b0010,0b0010,0b0011,0b0011,0b0010,0b0010,
           0b0011,0b0011,0b0010,0b0010,0b0011,0b0011,0b1111,0b1000,0b1010,0b0101}, // EMECHEC
          {0b1111,0b0010,0b1111,0b0010,0b1111,0b0010,0b1111,0b0010,0b0010,0b0010,
           0b0010,0b0010,0b0010,0b0010,0b0010,0b0010,0b1111,0b1000,0b1010,0b0101}, // FCAL1 first type
          {0b0010,0b0010,0b0010,0b0010,0b0010,0b0010,0b0010,0b0010,0b0010,0b0010,
           0b0010,0b0010,0b0010,0b0010,0b0010,0b0010,0b1111,0b1000,0b1010,0b0101}, // FCAL1 second type
          {0b0011,0b0011,0b0011,0b0011,0b0011,0b0011,0b0011,0b0011,0b0011,0b0011,
           0b0011,0b0011,0b0011,0b0011,0b0011,0b0011,0b1111,0b1000,0b1010,0b0101} // FCAL2
        }};


        constexpr std::array<int, 100> CMSK =
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1
        }; 

        constexpr std::array<unsigned int, 385> INV19 = 
        {
          0x00000,0x45000,0x22800,0x17000,0x11400,0x0DCC8,0x0B800,0x09DB0,
          0x08A00,0x07AA8,0x06E60,0x06458,0x05C00,0x054E8,0x04ED8,0x04998,
          0x04500,0x040F0,0x03D50,0x03A18,0x03730,0x03490,0x03228,0x03000,
          0x02E00,0x02C28,0x02A70,0x028E0,0x02768,0x02610,0x024C8,0x02398,
          0x02280,0x02170,0x02078,0x01F88,0x01EA8,0x01DD0,0x01D08,0x01C48,
          0x01B98,0x01AE8,0x01A48,0x019A8,0x01910,0x01888,0x01800,0x01778,
          0x01700,0x01680,0x01610,0x015A0,0x01538,0x014D0,0x01470,0x01410,
          0x013B0,0x01358,0x01308,0x012B0,0x01260,0x01218,0x011C8,0x01180,
          0x01140,0x010F8,0x010B8,0x01078,0x01038,0x01000,0x00FC0,0x00F88,
          0x00F50,0x00F18,0x00EE8,0x00EB8,0x00E80,0x00E50,0x00E20,0x00DF8,
          0x00DC8,0x00DA0,0x00D70,0x00D48,0x00D20,0x00CF8,0x00CD0,0x00CB0,
          0x00C88,0x00C60,0x00C40,0x00C20,0x00C00,0x00BD8,0x00BB8,0x00B98,
          0x00B80,0x00B60,0x00B40,0x00B20,0x00B08,0x00AE8,0x00AD0,0x00AB0,
          0x00A98,0x00A80,0x00A68,0x00A50,0x00A38,0x00A20,0x00A08,0x009F0,
          0x009D8,0x009C0,0x009A8,0x00998,0x00980,0x00968,0x00958,0x00940,
          0x00930,0x00918,0x00908,0x008F8,0x008E0,0x008D0,0x008C0,0x008B0,
          0x008A0,0x00888,0x00878,0x00868,0x00858,0x00848,0x00838,0x00828,
          0x00818,0x00808,0x00800,0x007F0,0x007E0,0x007D0,0x007C0,0x007B8,
          0x007A8,0x00798,0x00788,0x00780,0x00770,0x00768,0x00758,0x00748,
          0x00740,0x00730,0x00728,0x00718,0x00710,0x00708,0x006F8,0x006F0,
          0x006E0,0x006D8,0x006D0,0x006C0,0x006B8,0x006B0,0x006A0,0x00698,
          0x00690,0x00688,0x00678,0x00670,0x00668,0x00660,0x00658,0x00648,
          0x00640,0x00638,0x00630,0x00628,0x00620,0x00618,0x00610,0x00608,
          0x00600,0x005F0,0x005E8,0x005E0,0x005D8,0x005D0,0x005C8,0x005C0,
          0x005C0,0x005B8,0x005B0,0x005A8,0x005A0,0x00598,0x00590,0x00588,
          0x00580,0x00578,0x00574,0x00570,0x00568,0x00560,0x00558,0x00550,
          0x0054C,0x00548,0x00540,0x00538,0x00530,0x0052C,0x00528,0x00520,
          0x00518,0x00514,0x00510,0x00508,0x00500,0x004FC,0x004F8,0x004F0,
          0x004EC,0x004E8,0x004E0,0x004D8,0x004D4,0x004D0,0x004C8,0x004C4,
          0x004C0,0x004B8,0x004B4,0x004B0,0x004AC,0x004A8,0x004A0,0x0049C,
          0x00498,0x00490,0x0048C,0x00488,0x00484,0x00480,0x0047C,0x00478,
          0x00470,0x0046C,0x00468,0x00464,0x00460,0x0045C,0x00458,0x00454,
          0x00450,0x00448,0x00444,0x00440,0x0043C,0x00438,0x00434,0x00430,
          0x0042C,0x00428,0x00424,0x00420,0x0041C,0x00418,0x00414,0x00410,
          0x0040C,0x00408,0x00405,0x00402,0x00400,0x003FC,0x003F8,0x003F4,
          0x003F0,0x003EC,0x003E8,0x003E4,0x003E0,0x003DD,0x003DA,0x003D8,
          0x003D4,0x003D0,0x003CC,0x003C8,0x003C5,0x003C2,0x003C0,0x003BC,
          0x003B8,0x003B5,0x003B2,0x003B0,0x003AC,0x003A8,0x003A5,0x003A2,
          0x003A0,0x0039D,0x0039A,0x00398,0x00394,0x00390,0x0038D,0x0038A,
          0x00388,0x00385,0x00382,0x00380,0x0037D,0x0037A,0x00378,0x00375,
          0x00372,0x00370,0x0036D,0x0036A,0x00368,0x00365,0x00362,0x00360,
          0x0035D,0x0035A,0x00358,0x00355,0x00352,0x00350,0x0034D,0x0034A,
          0x00348,0x00345,0x00342,0x00340,0x0033D,0x0033A,0x00338,0x00336,
          0x00334,0x00332,0x00330,0x0032D,0x0032A,0x00328,0x00326,0x00324,
          0x00322,0x00320,0x0031D,0x0031A,0x00318,0x00316,0x00314,0x00312,
          0x00310,0x0030E,0x0030C,0x0030A,0x00308,0x00306,0x00304,0x00302,
          0x00300,0x002FD,0x002FA,0x002F8,0x002F6,0x002F4,0x002F2,0x002F0,
          0x002EE,0x002EC,0x002EA,0x002E8,0x002E6,0x002E5,0x002E3,0x002E1,
          0x002E0
        };



} // namespace LVL1::gFEXPos

#endif // L1CALOFEXBYTESTREAM_GFEXPOS_H
