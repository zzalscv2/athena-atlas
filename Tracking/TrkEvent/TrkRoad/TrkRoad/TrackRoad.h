/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKROAD_TRACKROAD_H
#define TRKROAD_TRACKROAD_H

#include "EventPrimitives/EventPrimitives.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include <iostream>

class MsgStream;

namespace Trk {

/**
 * Encapsulates the information required by the find()
 * method of the muon segment makers.
 */
class TrackRoad final
{
public:
  /**
   * TrackRoad Types
   */
  enum TrackRoadTypes
  {
    Unknown = 0,
    RPCTGC = 1,
    MDT = 2,
    IDSeeded = 3
  };
  /**
   * Constructor.
   * @param globalPosition  The global position of the road.
   * @param globalDirection The global direction of the road.
   * @param deltaEta        The width of the road in the eta direction.
   * @param deltaPhi        The width of the road in the phi direction.
   * @param type            The road type (defaults to Unnown)
   * @param weight          The road weight (defaults to 1.0)
   */
  TrackRoad(const Amg::Vector3D& globalPosition,
            const Amg::Vector3D& globalDirection,
            double deltaEta,
            double deltaPhi,
            int type = Unknown,
            double weight = 1.0);

  TrackRoad() = default;
  TrackRoad(const TrackRoad&) = default;
  TrackRoad(TrackRoad&&) = default;
  TrackRoad& operator=(const TrackRoad&) = default;
  TrackRoad& operator=(TrackRoad&&) = default;
  ~TrackRoad() = default;
  /**
   * Get the global position of the road.
   */
  const Amg::Vector3D& globalPosition() const;
  /**
   * Get the global direction of the road.
   */
  const Amg::Vector3D& globalDirection() const;
  /**
   * Get the width of the road in the eta direction.
   */
  double deltaEta() const;
  /**
   * Get the width of the road in the phi direction.
   */
  double deltaPhi() const;
  /**
   * Get the type of the road
   */
  int type() const;
  /**
   * Get the weight of the road.
   */
  double weight() const;

  /**
   * Get a readable form for debugging.
   */
  std::string toString() const;
  /**
   * Dump the road into a message stream.
   */
  MsgStream& dump(MsgStream& stream) const;
  /**
   * Dump the road into a standard output stream.
   */
  std::ostream& dump(std::ostream& stream) const;
  /**
   * Set the type of the road
   */
  void setType(int type);
  /**
   * Set the weight of the road.
   */
  void setWeight(double weight);

protected:
  Amg::Vector3D m_globalPos{}; /**< The global position of the road. */
  Amg::Vector3D m_globalDir{}; /**< The global direction of the road. */
  double m_dDeltaEta{ 0 }; /**< The width of the road in the eta direction. */
  double m_dDeltaPhi{ 0 }; /**< The width of the road in the phi direction. */
  int m_type{ 0 }; /**< The type of the road (Unknown, RPCTGC, MDT, IDSeeded) */
  double m_weight{ 1.0 }; /**< The weight of the road (default is 1.0) */
};

/**
 * Dump the road into a message stream.
 */
MsgStream&
operator<<(MsgStream& stream, const TrackRoad& tr);
/**
 * Dump the road into a standard output stream.
 */
std::ostream&
operator<<(std::ostream& stream, const TrackRoad& tr);

inline TrackRoad::TrackRoad(const Amg::Vector3D& globalPosition,
                            const Amg::Vector3D& globalDirection,
                            double deltaEta,
                            double deltaPhi,
                            int type,
                            double weight)
  : m_globalPos(globalPosition)
  , m_globalDir(globalDirection)
  , m_dDeltaEta(deltaEta)
  , m_dDeltaPhi(deltaPhi)
  , m_type(type)
  , m_weight(weight)
{
}

inline const Amg::Vector3D&
TrackRoad::globalPosition() const
{
  return m_globalPos;
}

inline const Amg::Vector3D&
TrackRoad::globalDirection() const
{
  return m_globalDir;
}

inline double
TrackRoad::deltaEta() const
{
  return m_dDeltaEta;
}

inline double
TrackRoad::deltaPhi() const
{
  return m_dDeltaPhi;
}

inline int
TrackRoad::type() const
{
  return m_type;
}

inline double
TrackRoad::weight() const
{
  return m_weight;
}

inline void
TrackRoad::setType(int type)
{
  m_type = type;
}

inline void
TrackRoad::setWeight(double weight)
{
  m_weight = weight;
}

}

#endif
