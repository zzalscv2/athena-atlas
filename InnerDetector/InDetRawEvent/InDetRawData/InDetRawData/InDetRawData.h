/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// InDetRawData.h
//   Header file for class InDetRawData
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Class to handle raw data objects for Pixel and SCT and TRT
///////////////////////////////////////////////////////////////////
// Version 1.0 13/08/2002 Veronique Boisvert
///////////////////////////////////////////////////////////////////

#ifndef INDETRAWDATA_INDETRAWDATA_H
#define INDETRAWDATA_INDETRAWDATA_H

// Base class
#include "Identifier/Identifiable.h"
#include "Identifier/Identifier.h"
class MsgStream;


class InDetRawData : public Identifiable {

  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////
public:

  // Constructor with parameters:
  // offline compact identifier of the readout channel
  InDetRawData(const Identifier rdoId, const unsigned int word);
  virtual ~InDetRawData() = default;

  ///////////////////////////////////////////////////////////////////
  // Virtual methods:
  ///////////////////////////////////////////////////////////////////

  virtual Identifier identify() const override final
  {
      return m_rdoId;
  }


  unsigned int getWord() const
  {
     return m_word;
  }

  ///////////////////////////////////////////////////////////////////
  // Clients should not use default constructor, rather use the one
  // above. It must be public for pool I/O.
  ///////////////////////////////////////////////////////////////////
  InDetRawData() = default;

  // OR the data word from OTHER into our data word.
  // Used by InDetOverlay.
  void merge (const InDetRawData& other)
  {
     m_word |= other.m_word;
  }

  ///////////////////////////////////////////////////////////////////
  // Private data:
  ///////////////////////////////////////////////////////////////////
private:
  Identifier m_rdoId; //Offline ID for readout channel
  
protected:
  unsigned int m_word = 0; // raw data word 
};

/**Overload of << operator for MsgStream for debug output*/
MsgStream& operator << ( MsgStream& sl, const InDetRawData& rdo);

/**Overload of << operator for std::ostream for debug output*/ 
std::ostream& operator << ( std::ostream& sl, const InDetRawData& rdo);

#endif // INDETRAWDATA_INDETRAWDATA_H

