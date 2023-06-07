/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/***************************************************************************
                          jFEXCompression.h  -  description
                             -------------------
    begin                : 07-02-2019 
    email                : Alan.Watson@cern.ch antonio.jacques.costa@cern.ch
 ***************************************************************************/

                                                                                
 #ifndef jFEXCompression_H
 #define jFEXCompression_H
                                                                                
#include <array>

namespace LVL1 {

/**
LAr supercell data are received by the jFEX in a 10-bit multi-linear encoded form.
This simple utility class contains 3 functions:
  - Compress:  encodes a signed integer MeV value
  - Expand:    decodes a 10 bit unsigned int into a signed integer MeV value
  - Threshold: applies a threshold (in MeV) to the compressed code, zeroing if below
  - Linearize: decodes a 10 bit unsigned int into the unsigned 16 bit jFEX ET with
               least count of 25 MeV. A user-defined threshold (in MeV) can be 
               applied, but as the jFEX ET is positive a negative threshold is
               equivalent to a threshold of 0.
  */
class jFEXCompression {

public: 
  /** Compress data */
  static unsigned int Compress(float floatEt);
  /** Uncompress data */
  static int Expand(unsigned int code);
  /** Apply threshold to compressed data */
  static unsigned int Threshold(unsigned int code, int threshold = -800);
  /** Linearize LAr code to jFEX internal format */
  static unsigned int Linearize(unsigned int code, int threshold = 0);
 
private: 
  /** Maximum ET value that can be encoded */
  static const int s_maxET = 800000;
  /** Number of ranges */
  static const unsigned int s_nRanges = 5;
  /** Step sizes in each range, MeV */
  static const int s_steps[s_nRanges];
  /** Minimum ET values in each range, MeV */
  static const int s_minET[s_nRanges];
  /** Minimum code value in each range */
  static const int s_minCode[s_nRanges];
  /** Indicates no data present */
  static const int s_NoData = 0;
  /** LAr underflow code */
  static const unsigned int s_LArUnderflow = 1;
  /** LAr overflow code */
  static const unsigned int s_LArOverflow  = 4048;
  /** Maximum code value, values 4049 to 4094 reserved */
  static const unsigned int s_LArMaxCode   = 4095;
  /** Invalid code value */
  static const unsigned int s_LArInvalid   = 4095;
  /** L1Calo ET digit step */
  static const unsigned int s_jFEXstep     = 25;
  /** L1Calo saturated/overflow */
  static const unsigned int s_jFEXOverflow = 0xffff;
  /** Error return value */
  static const int s_error = -99999;
};


}//end of ns

#endif
