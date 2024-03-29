/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// InDetRawData.cxx
//   Implementation file for class InDetRawData
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Version 1.0 13/08/2002 Veronique Boisvert
///////////////////////////////////////////////////////////////////

#include "InDetRawData/InDetRawData.h"
#include "GaudiKernel/MsgStream.h"

// Constructor with parameters:
InDetRawData::InDetRawData(const Identifier rdoId, const unsigned int word) :
  Identifiable(),
  m_rdoId(rdoId),
  m_word(word)
{

}

MsgStream& operator << ( MsgStream& sl, const InDetRawData& rdo) {
  sl << " Identifier " << rdo.identify() << " word " << rdo.getWord();
  return sl;
}

std::ostream& operator << ( std::ostream& sl, const InDetRawData& rdo) {
  sl << " Identifier " << rdo.identify() << " word " << rdo.getWord();
  return sl;
}
