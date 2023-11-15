/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////
// Runge-Kutta method for tracking a particle through a magnetic field
// (c) ATLAS Detector software
//////////////////////////////////////////////////////////////////////

#ifndef TRKEXRUNGEKUTTAINTERSECTOR_RUNGEKUTTAINTERSECTOR_H
#define TRKEXRUNGEKUTTAINTERSECTOR_RUNGEKUTTAINTERSECTOR_H

#include <atomic>

#include "AthenaBaseComps/AthAlgTool.h"
#include "EventPrimitives/EventPrimitives.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiKernel/ToolHandle.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "TrkExInterfaces/IIntersector.h"
#include "TrkExUtils/TrackSurfaceIntersection.h"

namespace Trk {

class RungeKuttaIntersector final : public extends<AthAlgTool, IIntersector> {

 public:
  RungeKuttaIntersector(const std::string& type, const std::string& name,
                        const IInterface* parent);
  virtual ~RungeKuttaIntersector() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  /**IIntersector interface method for general Surface type */
  virtual std::optional<TrackSurfaceIntersection> intersectSurface(
      const Surface& surface, const TrackSurfaceIntersection& trackIntersection,
      const double qOverP) const override;

  /**IIntersector interface method for specific Surface type : PerigeeSurface */
  virtual std::optional<TrackSurfaceIntersection> approachPerigeeSurface(
      const PerigeeSurface& surface,
      const TrackSurfaceIntersection& trackIntersection,
      const double qOverP) const override;

  /**IIntersector interface method for specific Surface type :
   * StraightLineSurface */
  virtual std::optional<TrackSurfaceIntersection> approachStraightLineSurface(
      const StraightLineSurface& surface,
      const TrackSurfaceIntersection& trackIntersection,
      const double qOverP) const override;

  /**IIntersector interface method for specific Surface type : CylinderSurface
   */
  virtual std::optional<TrackSurfaceIntersection> intersectCylinderSurface(
      const CylinderSurface& surface,
      const TrackSurfaceIntersection& trackIntersection,
      const double qOverP) const override;

  /**IIntersector interface method for specific Surface type : DiscSurface */
  virtual std::optional<TrackSurfaceIntersection> intersectDiscSurface(
      const DiscSurface& surface,
      const TrackSurfaceIntersection& trackIntersection,
      const double qOverP) const override;

  /**IIntersector interface method for specific Surface type : PlaneSurface */
  virtual std::optional<TrackSurfaceIntersection> intersectPlaneSurface(
      const PlaneSurface& surface,
      const TrackSurfaceIntersection& trackIntersection,
      const double qOverP) const override;

  /**IIntersector interface method for validity check over a particular
   * extrapolation range */
  virtual bool isValid(Amg::Vector3D /*startPosition*/,
                       Amg::Vector3D /*endPosition*/) const override {
    return true;
  }

 private:
  // private methods
  void assignStepLength(const TrackSurfaceIntersection& isect,
                        double& stepLength) const;
  void debugFailure(TrackSurfaceIntersection&& isect,
                    const Surface& surface, const double qOverP,
                    const double rStart, const double zStart,
                    const bool trapped) const;
  double distanceToCylinder(const TrackSurfaceIntersection& isect,
                            const double cylinderRadius,
                            const double offsetRadius,
                            const Amg::Vector3D& offset,
                            double& stepLength) const;
  double distanceToDisc(const TrackSurfaceIntersection& isect,
                        const double discZ, double& stepLength) const;

  double distanceToLine(const TrackSurfaceIntersection& isect,
                        const Amg::Vector3D& linePosition,
                        const Amg::Vector3D& lineDirection,
                        double& stepLength) const;

  double distanceToPlane(const TrackSurfaceIntersection& isect,
                         const Amg::Vector3D& planePosition,
                         const Amg::Vector3D& planeNormal,
                         double& stepLength) const;

  Amg::Vector3D field(const Amg::Vector3D& point,
                      MagField::AtlasFieldCache& fieldCache) const;

  void initializeFieldCache(MagField::AtlasFieldCache& fieldCache) const;

  std::optional<TrackSurfaceIntersection> newIntersection(
      TrackSurfaceIntersection&& isect, const Surface& surface,
      const double qOverP, const double rStart, const double zStart) const;

  void shortStep(TrackSurfaceIntersection& isect,
                 const Amg::Vector3D& fieldValue, const double stepLength,
                 const double qOverP) const;

  void step(TrackSurfaceIntersection& isect, Amg::Vector3D& fieldValue,
            double& stepLength, const double qOverP,
            MagField::AtlasFieldCache& fieldCache) const;

  SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCacheCondObjInputKey{
      this, "AtlasFieldCacheCondObj", "fieldCondObj",
      "Name of the Magnetic Field conditions object key"};

  // additional configuration
  bool m_productionMode;

  // some precalculated constants:
  const double m_caloR0;
  const double m_caloR1;
  const double m_caloR2;
  const double m_caloR3;
  const double m_caloR4;
  const double m_caloZ0;
  const double m_caloZ1;
  const double m_caloZ2;
  const double m_caloZ3;
  const double m_inDetR0;
  const double m_inDetR1;
  const double m_inDetR2;
  const double m_inDetZ0;
  const double m_inDetZ1;
  const double m_inDetZ2;
  const double m_momentumThreshold;
  const double m_momentumWarnThreshold;
  const double m_muonR0;
  const double m_muonZ0;
  double m_shortStepMax;
  const double m_shortStepMin;
  const double m_solenoidR;
  const double m_solenoidZ;
  double m_stepMax0;
  double m_stepMax1;
  double m_stepMax2;
  double m_stepMax3;
  double m_stepMax4;
  int m_stepsUntilTrapped;
  const double m_third;
  const double m_toroidR0;
  const double m_toroidR1;
  const double m_toroidR2;
  const double m_toroidR3;
  const double m_toroidZ0;
  const double m_toroidZ1;
  const double m_toroidZ2;
  const double m_toroidZ3;
  const double m_toroidZ4;
  const double m_toroidZ5;
  const double m_toroidZ6;
  const double m_toroidZ7;
  const double m_toroidZ8;

  // counters
  mutable std::atomic<unsigned long long> m_countExtrapolations;
  mutable std::atomic<unsigned long long> m_countShortStep;
  mutable std::atomic<unsigned long long> m_countStep;
  mutable std::atomic<unsigned long long> m_countStepReduction;
};

//<<<<<< INLINE PRIVATE MEMBER FUNCTIONS                                >>>>>>

inline double RungeKuttaIntersector::distanceToCylinder(
    const TrackSurfaceIntersection& isect, const double cylinderRadius,
    const double offsetRadius, const Amg::Vector3D& offset,
    double& stepLength) const {
  const Amg::Vector3D& dir = isect.direction();

  double sinThsqinv = 1. / dir.perp2();
  stepLength = (offset.x() * dir.x() + offset.y() * dir.y()) * sinThsqinv;
  double deltaRSq = (cylinderRadius - offsetRadius) *
                        (cylinderRadius + offsetRadius) * sinThsqinv +
                    stepLength * stepLength;
  if (deltaRSq > 0.)
    stepLength += sqrt(deltaRSq);
  return std::abs(stepLength);
}

inline double RungeKuttaIntersector::distanceToDisc(
    const TrackSurfaceIntersection& isect, const double discZ,
    double& stepLength) const {
  const Amg::Vector3D& pos = isect.position();
  const Amg::Vector3D& dir = isect.direction();

  double distance = discZ - pos.z();
  stepLength = distance / dir.z();
  return std::abs(distance);
}

inline double RungeKuttaIntersector::distanceToLine(
    const TrackSurfaceIntersection& isect, const Amg::Vector3D& linePosition,
    const Amg::Vector3D& lineDirection, double& stepLength) const {
  // offset joining track to line is given by
  //   offset = linePosition + a*lineDirection - trackPosition -
  //   b*trackDirection
  //
  // offset is perpendicular to both line and track at solution i.e.
  //   lineDirection.dot(offset) = 0
  //   trackDirection.dot(offset) = 0
  const Amg::Vector3D& pos = isect.position();
  const Amg::Vector3D& dir = isect.direction();

  double cosAngle = lineDirection.dot(dir);
  stepLength = (linePosition - pos).dot(dir - lineDirection * cosAngle) /
               (1. - cosAngle * cosAngle);
  return std::abs(stepLength);
}

inline double RungeKuttaIntersector::distanceToPlane(
    const TrackSurfaceIntersection& isect, const Amg::Vector3D& planePosition,
    const Amg::Vector3D& planeNormal, double& stepLength) const {
  // take the normal component of the offset from track position to plane
  // position this is equal to the normal component of the required distance
  // along the track direction
  const Amg::Vector3D& pos = isect.position();
  const Amg::Vector3D& dir = isect.direction();

  double distance = planeNormal.dot(planePosition - pos);
  stepLength = distance / planeNormal.dot(dir);
  return std::abs(distance);
}

inline void RungeKuttaIntersector::initializeFieldCache(
    MagField::AtlasFieldCache& fieldCache) const {
  SG::ReadCondHandle<AtlasFieldCacheCondObj> fieldCondObj{
      m_fieldCacheCondObjInputKey};
  if (!fieldCondObj.isValid()) {
    ATH_MSG_FATAL("Failed to get magnetic field conditions data "
                  << m_fieldCacheCondObjInputKey.key());
  }
  fieldCondObj->getInitializedCache(fieldCache);
}

inline Amg::Vector3D RungeKuttaIntersector::field(
    const Amg::Vector3D& position,
    MagField::AtlasFieldCache& fieldCache) const {
  Amg::Vector3D fieldValue;
  fieldCache.getField(position.data(), fieldValue.data());
  return fieldValue;
}

inline std::optional<TrackSurfaceIntersection> RungeKuttaIntersector::newIntersection(
    TrackSurfaceIntersection&& isect, const Surface& surface,
    const double qOverP, const double rStart, const double zStart) const {
  // ensure intersection is valid (ie. on surface)
  Intersection SLIntersect = surface.straightLineIntersection(
      isect.position(), isect.direction(), false, false);
  if (SLIntersect.valid) {
    isect.position() = SLIntersect.position;
    return std::move(isect);
  }

  // invalid: take care to invalidate cache!
  if (msgLvl(MSG::DEBUG)){
    debugFailure(std::move(isect), surface, qOverP, rStart, zStart, false);
  }

  return std::nullopt;
}

}  // namespace Trk

#endif  // TRKEXRUNGEKUTTAINTERSECTOR_RUNGEKUTTAINTERSECTOR_H
