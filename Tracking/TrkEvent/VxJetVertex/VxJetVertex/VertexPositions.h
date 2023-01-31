/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
                           VertexPositions.h  -  Description
                             -------------------
    begin   : Autumn 2006
    authors : Giacinto Piacquadio (Freiburg University)
    email   : giacinto.piacquadio@cern.ch
    changes :

 ***************************************************************************/

#ifndef VXJETVERTEX_VERTEXPOSITIONS_H
#define VXJETVERTEX_VERTEXPOSITIONS_H

#include "EventPrimitives/EventPrimitives.h"
#include "TrkEventPrimitives/TrkObjectCounter.h"
#include "GaudiKernel/MsgStream.h"
#include "VxJetVertex/JetVtxParamDefs.h"
#include <atomic>
/** The standard namespace for VxVertexPositions */
namespace Trk {

/** VertexPositions class to represent and store a vertex */
class VertexPositions : public Trk::ObjectCounter<Trk::VertexPositions>
{
public:
  VertexPositions();                                 //!< default constructor
  VertexPositions(const Amg::VectorX& p);            //!< constructor with variable-size vector
  VertexPositions(const VertexPositions&) = default; //!< copy constructor
  VertexPositions(VertexPositions&&) = default;      //!< move constructor
  VertexPositions& operator=(const VertexPositions&) = default; //!< Assignment operator
  VertexPositions& operator=(VertexPositions&&) = default;      //!< move Assignment operator
  virtual ~VertexPositions() = default;

  /** Output Method for MsgStream, to be overloaded by child classes */
  virtual MsgStream& dump(MsgStream& sl) const;
  /** Output Method for std::ostream, to be overloaded by child classes */
  virtual std::ostream& dump(std::ostream& sl) const;

  const Amg::VectorX& position() const; //!< return position of vertex

  void setPosition(const Amg::VectorX&);

protected:
  Amg::VectorX m_position;       //!< vertex position
  bool m_useWeightTimesPosition; // bool for storing weightTimesPosition
};

/**Overload of << operator for both, MsgStream and std::ostream for debug
output*/
MsgStream&
operator<<(MsgStream& sl, const VertexPositions& sf);
std::ostream&
operator<<(std::ostream& sl, const VertexPositions& sf);

} // end of namespace Trk

#endif
