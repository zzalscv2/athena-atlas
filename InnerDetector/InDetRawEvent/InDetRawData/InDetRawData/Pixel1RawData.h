/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// Pixel1RawData.h
//   Header file for class Pixel1RawData
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Class to implement RawData for Pixel, using method 1 of decoding the word
///////////////////////////////////////////////////////////////////
// Version 1.0 13/08/2002 Veronique Boisvert
///////////////////////////////////////////////////////////////////

#ifndef INDETRAWDATA_PIXEL1RAWDATA_H
#define INDETRAWDATA_PIXEL1RAWDATA_H

// Base class
#include "InDetRawData/PixelRDORawData.h"


class Pixel1RawData final : public PixelRDORawData{

public:
  // Constructor with parameters:
  // offline hashId of the readout element, 
  // the word

 Pixel1RawData();
 Pixel1RawData(const Identifier rdoId, const unsigned int word);
 //
 Pixel1RawData(const Pixel1RawData&) = default;
 Pixel1RawData(Pixel1RawData&&) noexcept = default;
 Pixel1RawData& operator=(const Pixel1RawData&) = default;
 Pixel1RawData& operator=(Pixel1RawData&&) noexcept = default;
 virtual ~Pixel1RawData() = default;

 // Constructor with full parameter list: hashId, ToT, BCO ID,
 // LVL1 accept, ATLAS wide LVL1
 Pixel1RawData(const Identifier rdoId, const unsigned int ToT,
               const unsigned int BCID, const unsigned int LVL1ID,
               const unsigned int LVL1A = 0);

 ///////////////////////////////////////////////////////////////////
 // Virtual methods
 ///////////////////////////////////////////////////////////////////

 virtual int getToT() const override;    // Time over Threshold value 0-255
 virtual int getBCID() const override;   // Beam Crossing ID
 virtual int getLVL1A() const override;  // Level 1 accept, 0-15, used if
                                         // reading consecutive BCOs
 virtual int getLVL1ID() const override;  // ATLAS LVL1 0-255
};

///////////////////////////////////////////////////////////////////
// Inline methods:
///////////////////////////////////////////////////////////////////
// decode TOT information (taken from Calvet RawData class) 
inline int Pixel1RawData::getToT() const
{
  return (m_word & 0xFF);
}

// decode BCID information 
inline int Pixel1RawData::getBCID() const
{
  return ( (m_word>>8) & 0xFF);
}

// decode LVL1 accept information 
inline int Pixel1RawData::getLVL1A() const
{
  return ( (m_word>>24) & 0xF);
}

// decode Atlas wide LVL1 information 
inline int Pixel1RawData::getLVL1ID() const
{
  return ( (m_word>>16) & 0xFF);
}

#endif // INDETRAWDATA_PIXEL1RAWDATA_H
