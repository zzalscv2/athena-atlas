/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// PixelClusterParts.h
//   Header file for class PixelClusterParts
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKPREPRAWDATA_PIXELCLUSTERPARTS_H
#define TRKPREPRAWDATA_PIXELCLUSTERPARTS_H

#include "EventPrimitives/EventPrimitives.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "Identifier/Identifier.h"

namespace InDet {
/** @class PixelClusterParts

groups Idetifier, tot vector and lvl1 vector together
facilitates interface of IClusterSplitter

@author Andreas.Salzburger@cern.ch
@author Christos Anastopoulos MT related changes
*/
class PixelClusterParts
{

public:
  /** constructor */
  PixelClusterParts(const std::vector<Identifier>& group,
                    const std::vector<int>& totgroup,
                    const std::vector<int>& lvl1group)
    : m_idgroup(group)
    , m_totgroup(totgroup)
    , m_lvl1group(lvl1group)
    , m_localPosition(Amg::Vector2D::Zero())
    , m_errorMatrix{}
  {
  }

  /** constructor */
  PixelClusterParts(const std::vector<Identifier>& group,
                    const std::vector<int>& totgroup,
                    const std::vector<int>& lvl1group,
                    const Amg::Vector2D& position)
    : m_idgroup(group)
    , m_totgroup(totgroup)
    , m_lvl1group(lvl1group)
    , m_localPosition(position)
    , m_errorMatrix{}
  {
  }

  /** constructor */
  PixelClusterParts(const std::vector<Identifier>& group,
                    const std::vector<int>& totgroup,
                    const std::vector<int>& lvl1group,
                    const Amg::Vector2D& position,
                    const Amg::MatrixX& error)
    : m_idgroup(group)
    , m_totgroup(totgroup)
    , m_lvl1group(lvl1group)
    , m_localPosition(position)
    , m_errorMatrix(error)
  {
  }

  /** default ctors and move  */
  PixelClusterParts(const PixelClusterParts& pcp) = default;
  PixelClusterParts(PixelClusterParts&& pcp) noexcept = default;
  PixelClusterParts& operator=(const PixelClusterParts& pcp) = default;
  PixelClusterParts& operator=(PixelClusterParts&& pcp) noexcept = default;
  /** destructor */
  ~PixelClusterParts() = default;

  /** return method identifiers */
  const std::vector<Identifier>& identifierGroup() const;

  /** return method tot */
  const std::vector<int>& totGroup() const;

  /** return method lvl1 Group */
  const std::vector<int>& lvl1Group() const;

  /** return lcoal positions */
  const Amg::Vector2D& localPosition() const;

  /** return error description */
  const Amg::MatrixX& errorMatrix() const;

private:
  std::vector<Identifier> m_idgroup;
  std::vector<int> m_totgroup;
  std::vector<int> m_lvl1group;
  Amg::Vector2D m_localPosition;
  Amg::MatrixX m_errorMatrix;
};

// inline implementation of return methods
inline const std::vector<Identifier>&
PixelClusterParts::identifierGroup() const
{
  return m_idgroup;
}
inline const std::vector<int>&
PixelClusterParts::totGroup() const
{
  return m_totgroup;
}
inline const std::vector<int>&
PixelClusterParts::lvl1Group() const
{
  return m_lvl1group;
}
inline const Amg::Vector2D&
PixelClusterParts::localPosition() const
{
  return m_localPosition;
}
inline const Amg::MatrixX&
PixelClusterParts::errorMatrix() const
{
  return m_errorMatrix;
}

}

#endif
