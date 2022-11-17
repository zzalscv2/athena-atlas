/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
                           Vertex.cxx  -  Description
                             -------------------
    begin   : Autumn 2003
    authors : Andreas Wildauer (CERN PH-ATC), Fredrik Akesson (CERN PH-ATC)
    email   : andreas.wildauer@cern.ch, fredrik.akesson@cern.ch
    comments: original version by M. Elsing
    changes :

 ***************************************************************************/

#include "VxVertex/Vertex.h"
#include "GaudiKernel/MsgStream.h"

namespace Trk {

Vertex::Vertex()
  : Trk::ObjectCounter<Trk::Vertex>()
  , m_position(0., 0., 0.)
{
}

Vertex::Vertex(const Amg::Vector3D& p)
  : Trk::ObjectCounter<Trk::Vertex>()
  , m_position(p)
{
}


MsgStream&
Vertex::dump(MsgStream& sl) const
{
  sl << "Trk::Vertex position: (" << m_position[0] << ", " << m_position[1]
     << ", " << m_position[2] << ") mm." << endmsg;
  return sl;
}

std::ostream&
Vertex::dump(std::ostream& sl) const
{
  sl << "Trk::Vertex position: (" << m_position[0] << ", " << m_position[1]
     << ", " << m_position[2] << ") mm." << std::endl;
  return sl;
}

MsgStream&
operator<<(MsgStream& sl, const Vertex& sf)
{
  return sf.dump(sl);
}

std::ostream&
operator<<(std::ostream& sl, const Vertex& sf)
{
  return sf.dump(sl);
}

const Amg::Vector3D&
Vertex::position() const
{
  return m_position;
}

} // end of namespace
