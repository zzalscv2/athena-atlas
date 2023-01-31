/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
                           VertexPositions.cxx  -  Description
                             -------------------
    begin   : Autumn 2006
    authors : Giacinto Piacquadio (Freiburg University)
    email   : Giacinto.Piacquadio@physik.uni-freiburg.de
    comments:
    changes :

 ***************************************************************************/

#include "VxJetVertex/VertexPositions.h"

namespace Trk {

VertexPositions::VertexPositions()
  : Trk::ObjectCounter<Trk::VertexPositions>()
  , m_position(Amg::VectorX())
  , m_useWeightTimesPosition(false)
{
  m_position.setZero();
}

VertexPositions::VertexPositions(const Amg::VectorX& p)
  : Trk::ObjectCounter<Trk::VertexPositions>()
  , m_position(p)
  , m_useWeightTimesPosition(false)
{
}

MsgStream&
VertexPositions::dump(MsgStream& sl) const
{
  if (m_useWeightTimesPosition) {
    sl << "Trk::VertexPositions weight times position: (";
  } else {
    sl << "Trk::VertexPositions position: (";
  }
  sl << "xv " << m_position[jet_xv] << ", "
     << "yv " << m_position[jet_yv] << ", "
     << "zv " << m_position[jet_zv] << ", "
     << "phi " << m_position[jet_phi] << ", "
     << "theta " << m_position[jet_theta] << endmsg;
  for (int i = 5; i < m_position.rows(); i++) {
    sl << "dist" << i << " " << m_position[i] << " ." << endmsg;
  }
  return sl;
}

std::ostream&
VertexPositions::dump(std::ostream& sl) const
{
  if (m_useWeightTimesPosition) {
    sl << "Trk::VertexPositions weight times position: (";
  } else {
    sl << "Trk::VertexPositions position: (";
  }
  sl << "xv " << m_position[jet_xv] << ", "
     << "yv " << m_position[jet_yv] << ", "
     << "zv " << m_position[jet_zv] << ", "
     << "phi " << m_position[jet_phi] << ", "
     << "theta " << m_position[jet_theta] << std::endl;
  for (int i = 5; i < m_position.rows(); i++) {
    sl << "dist" << i << " " << m_position[i] << " ." << std::endl;
  }
  return sl;
}

MsgStream&
operator<<(MsgStream& sl, const VertexPositions& sf)
{
  return sf.dump(sl);
}

std::ostream&
operator<<(std::ostream& sl, const VertexPositions& sf)
{
  return sf.dump(sl);
}

const Amg::VectorX&
VertexPositions::position() const
{
  if (!m_useWeightTimesPosition) {
    return m_position;
  }
  std::cout << "FATAL: VertexPositions is not able to return a valid position "
            << " as a const object: need to go from Update to Use mode. "
               "Unrecovered Bug!"
            << std::endl;
  std::abort();
  //    return m_position;
}

void
VertexPositions::setPosition(const Amg::VectorX& newposition)
{
  m_position = newposition;
}
} // end of namespace
