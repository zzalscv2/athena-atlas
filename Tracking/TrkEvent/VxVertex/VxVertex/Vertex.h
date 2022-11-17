/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VXVERTEX_VERTEX_H
#define VXVERTEX_VERTEX_H
#include "GeoPrimitives/GeoPrimitives.h"
#include "TrkEventPrimitives/TrkObjectCounter.h"
/**
 * @class Trk::Vertex
 *
 * This class is a simplest representation of a vertex candidate.
 * The 3-position (X,Y,Z) is stored.
 * <br>
 *   begin   : Autumn 2003
 * <br>
 *   changes : 11.02.04 added docu
 *   @authors Andreas Wildauer (CERN PH-ATC), andreas.wildauer@cern.ch
 */

class MsgStream;
class RecVertexCnv_p1;

namespace Trk {
class Vertex : public Trk::ObjectCounter<Trk::Vertex>
{
public:
  /**
   * Contructors: default, copy and a constructor taking a
   * vertex position (Amg::Vector3D) as argument.
   */
  Vertex(); //!< default constructor
  //!< constructor with Amg::Vector3D (== Amg::Vector3D)
  Vertex(const Amg::Vector3D& p);
  Vertex(const Vertex&) = default;
  Vertex& operator=(const Vertex&) = default;
  Vertex(Vertex&&) = default;
  Vertex& operator=(Vertex&&) = default;
  virtual ~Vertex() = default;

  /** Output Method for MsgStream, to be overloaded by child classes */
  virtual MsgStream& dump(MsgStream& sl) const;
  /** Output Method for std::ostream, to be overloaded by child classes */
  virtual std::ostream& dump(std::ostream& sl) const;

  const Amg::Vector3D& position() const; //!< return position of vertex

private:
  friend class ::RecVertexCnv_p1;

  Amg::Vector3D m_position; //!< vertex position
};

/**Overload of << operator for both, MsgStream and std::ostream for debug
output*/
MsgStream&
operator<<(MsgStream& sl, const Vertex& sf);
std::ostream&
operator<<(std::ostream& sl, const Vertex& sf);

} // end of namespace Trk

#endif

