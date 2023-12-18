/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TRT_DriftCircle.cxx
//   Implementation file for class TRT_DriftCircle
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Version 1.0 15/07/2003 Veronique Boisvert
///////////////////////////////////////////////////////////////////

#include "InDetPrepRawData/TRT_DriftCircle.h"
#include "GaudiKernel/MsgStream.h"
#include <ostream>
#include <sstream>

namespace InDet{


TRT_DriftCircle::TRT_DriftCircle(
  const Identifier& Id,
  const Amg::Vector2D& driftRadius,
  std::vector<Identifier>&& rdoList,
  Amg::MatrixX&& errDriftRadius,
  const InDetDD::TRT_BaseElement* detEl,
  const unsigned int word)
  : PrepRawData(Id, driftRadius, std::move(rdoList), std::move(errDriftRadius))
  , m_detEl(detEl)
  , m_word(word)
{
}

TRT_DriftCircle::TRT_DriftCircle(
  const Identifier& Id,
  const Amg::Vector2D& driftRadius,
  Amg::MatrixX&& errDriftRadius,
  const InDetDD::TRT_BaseElement* detEl,
  const unsigned int word)
  : PrepRawData(Id, driftRadius, std::move(errDriftRadius))
  , m_detEl(detEl)
  , m_word(word)
{
}

// Default constr
TRT_DriftCircle::TRT_DriftCircle()
  : PrepRawData()
  , m_detEl(nullptr)
  , m_word(0)

{
}

MsgStream&
TRT_DriftCircle::dump(MsgStream& stream) const
{
  std::ostringstream out;
  dump(out);
  stream<<out.str();
  return stream;
}

std::ostream&
TRT_DriftCircle::dump(std::ostream& stream) const
{
  stream << "TRT_DriftCircle object" << std::endl;
  stream << "Level (true/false)		 " << highLevel() << std::endl;
  stream << "Valid (true/false)		 " << driftTimeValid() << std::endl;
  stream << "timeOverThreshold:               " << timeOverThreshold()
         << std::endl;
  stream << "driftTime:                       " << rawDriftTime() << std::endl;
  stream << "dataWord:                        " << m_word << std::endl;

  stream << "Base class (PrepRawData):" << std::endl;
  this->PrepRawData::dump(stream);

  return stream;
}

MsgStream&
operator<<(MsgStream& stream, const TRT_DriftCircle& prd)
{
  return prd.dump(stream);
}

std::ostream&
operator<<(std::ostream& stream, const TRT_DriftCircle& prd)
{
  return prd.dump(stream);
}

}//end of ns

