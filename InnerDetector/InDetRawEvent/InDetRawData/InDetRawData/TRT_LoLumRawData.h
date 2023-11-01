/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TRT_LoLumRawData.h
//   Header file for class TRT_LoLumRawData
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Class to implement RawData for TRT, full encoding
///////////////////////////////////////////////////////////////////
// Version 1.0 14/10/2002 Veronique Boisvert
///////////////////////////////////////////////////////////////////

#ifndef INDETRAWDATA_TRT_LOLUMRAWDATA_H
#define INDETRAWDATA_TRT_LOLUMRAWDATA_H

// Base class
#include "InDetRawData/TRT_RDORawData.h"
#include "CxxUtils/CachedValue.h"

// Data members classes

class TRT_LoLumRawData final : public TRT_RDORawData{

  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////
public:

  // Constructor with parameters:
  // offline hashId of the readout element,
  // the word
 TRT_LoLumRawData(const Identifier rdoId, const unsigned int word);
 TRT_LoLumRawData(const TRT_LoLumRawData&) = default;
 TRT_LoLumRawData(TRT_LoLumRawData&&) noexcept = default;
 TRT_LoLumRawData& operator=(const TRT_LoLumRawData&) = default;
 TRT_LoLumRawData& operator=(TRT_LoLumRawData&&) noexcept = default;
 virtual ~TRT_LoLumRawData() = default;

 // High level threshold:
 virtual bool highLevel() const override final;
 bool highLevel(int /* BX */) const;

 // Time over threshold in ns for valid digits; zero otherwise:
 virtual double timeOverThreshold() const override final {
   unsigned int leadingEdge = driftTimeBin();
   unsigned int trailingEdge = this->trailingEdge();
   if (leadingEdge && trailingEdge) {
     return (trailingEdge - leadingEdge + 1) * m_driftTimeBinWidth;
   };
   return 0.;
 };

  // drift time in bin
  virtual int driftTimeBin() const override final {
    if (!m_island.isValid()) {
     Island tmpIsland;
     findLargestIsland(m_word, tmpIsland);
     m_island.set(tmpIsland);
   }
   return m_island.ptr()->m_leadingEdge;
  };

  int trailingEdge() const {
    if (!m_island.isValid()) {
     Island tmpIsland;
     findLargestIsland(m_word, tmpIsland);
     m_island.set(tmpIsland);
   }
   return m_island.ptr()->m_trailingEdge;
  };

  bool firstBinHigh() const;  // True if first time bin is high
  bool lastBinHigh() const;   // True if last time bin is high

protected:
  // width of the drift time bins
  static constexpr double m_driftTimeBinWidth = 3.125;

  // bit masks used in interpretation of bit pattern
  static constexpr unsigned int m_maskFourLastBits=0xFFFFFF0;  // 1 1 11111111 1 11111111 1 11110000
  static constexpr unsigned int m_maskThreeLastBits=0xFFFFFF8;  // 1 1 11111111 1 11111111 1 11111000

public:
  // width of the drift time bins
  static constexpr double getDriftTimeBinWidth() {
    return m_driftTimeBinWidth;
  };

  struct Island {
    unsigned int m_leadingEdge = 0;
    unsigned int m_trailingEdge = 0;
  };
  // Find the relevant island of bits from the bit pattern, defined as the largest island with the latest leading edge
  static void findLargestIsland(unsigned int word, Island& island);
  // Check if the middle HT bit is set
  inline
  static bool highLevel(unsigned int word) {
    // return (m_word & 0x04020100); // check any of the three HT bits
    return (word & 0x00020000); // check only middle HT bit
  }

public:
  // public default constructor needed for I/O, but should not be
  // called from an alg
  TRT_LoLumRawData();

  ///////////////////////////////////////////////////////////////////
  // Private data:
  ///////////////////////////////////////////////////////////////////
private:
  CxxUtils::CachedValue<Island> m_island{};

};

///////////////////////////////////////////////////////////////////
// Inline methods:
///////////////////////////////////////////////////////////////////

/*
 * highLevel() -
 * Returns true if there is a high threshold hit in the middle bunch crossing, false
 * otherwise
 */
inline
bool TRT_LoLumRawData::highLevel() const
{
  return highLevel(m_word);
}

/*
 * highLevel( BX ) -
 * Returns true if there is a high threshold hit in bunch crossing BX, false
 * otherwise.  BX is 1 for the earliest bunch crossing and 3 for the latest
 * bunch crossing.
 */
inline
bool TRT_LoLumRawData::highLevel(int BX) const
{
  if ( (BX < 1) || (BX > 3) )
    return false;

  return (m_word & ( 1 << (9 * BX - 1) ));

}

/*
 * firstBinHigh() -
 * Returns true if the first low threshold time bin it high, false otherwise.
 */
inline bool
TRT_LoLumRawData::firstBinHigh() const
{
  return (m_word & 0x02000000);
}


/*
 * lastBinHigh() -
 * Returns true if the last low threshold time bin it high, false otherwise.
 */
inline bool
TRT_LoLumRawData::lastBinHigh() const
{
  return (m_word & 0x1);
}


#endif // INDETRAWDATA_TRT_LOLUMRAWDATA_H

