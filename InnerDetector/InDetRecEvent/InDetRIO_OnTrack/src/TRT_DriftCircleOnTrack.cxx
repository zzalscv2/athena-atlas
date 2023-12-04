/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TRT_DriftCircleOnTrack.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "InDetRIO_OnTrack/TRT_DriftCircleOnTrack.h"
#include "TRT_ReadoutGeometry/TRT_BaseElement.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "InDetPrepRawData/TRT_DriftCircle.h"
#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkSurfaces/Surface.h"
#include "TrkSurfaces/StraightLineSurface.h"

#include "GaudiKernel/MsgStream.h"

#include <cassert>
#include <limits>
#include <memory>
#include <ostream>
#include <utility>

InDet::TRT_DriftCircleOnTrack::TRT_DriftCircleOnTrack(
  const InDet::TRT_DriftCircle* RIO,
  Trk::LocalParameters&& driftRadius,
  Amg::MatrixX&& errDriftRadius,
  const IdentifierHash& idDE,
  double predictedLocZ,
  const Amg::Vector3D& predictedTrackDirection,
  const Trk::DriftCircleStatus status)
  // call base class constructor
  : Trk::RIO_OnTrack(std::move(driftRadius), std::move(errDriftRadius), RIO->identify())
  , m_globalPosition{}
  , m_positionAlongWire(predictedLocZ)
  , m_idDE(idDE)
  , m_status(status)
  , m_highLevel(RIO->highLevel())
  , m_timeOverThreshold(RIO->timeOverThreshold())
  , m_detEl(RIO->detectorElement())
{

  m_rio.setElement(RIO);
  const Trk::Surface& detElSurf = m_detEl->surface(RIO->identify());
  if (detElSurf.type() == Trk::SurfaceType::Line) {
    const Trk::StraightLineSurface& slsf =
        static_cast<const Trk::StraightLineSurface&>(detElSurf);
    m_globalPosition =
        slsf.localToGlobal(m_localParams, predictedTrackDirection, predictedLocZ);
  }
  Amg::Vector3D loc_gDirection = predictedTrackDirection;
  const double dr = m_localParams[Trk::driftRadius];

  // scaling the direction with drift radius
  if (dr != 0.) {
    m_localAngle = std::atan2(loc_gDirection.y(), loc_gDirection.x());
  } else {
    m_localAngle = 0.;
  }
}

//Constructor used by the converter
InDet::TRT_DriftCircleOnTrack::TRT_DriftCircleOnTrack(
  const ElementLinkToIDCTRT_DriftCircleContainer& RIO,
  const Trk::LocalParameters& driftRadius,
  const Amg::MatrixX& errDriftRadius,
  IdentifierHash idDE,
  const Identifier& id,
  double predictedLocZ,
  float localAngle,
  const Trk::DriftCircleStatus status,
  bool highLevel,
  double timeOverThreshold)
  : Trk::RIO_OnTrack(Trk::LocalParameters(driftRadius), Amg::MatrixX(errDriftRadius), id)
  , m_globalPosition{}
  , m_localAngle(localAngle)
  , m_positionAlongWire(predictedLocZ)
  , m_rio(RIO)
  , m_idDE(idDE)
  , m_status(status)
  , m_highLevel(highLevel)
  , m_timeOverThreshold(timeOverThreshold)
  , m_detEl(nullptr)
{}

// Destructor:
InDet::TRT_DriftCircleOnTrack::~TRT_DriftCircleOnTrack()
= default;

// default constructor:
InDet::TRT_DriftCircleOnTrack::TRT_DriftCircleOnTrack()
	:
  Trk::RIO_OnTrack(),
  m_globalPosition{},
  m_localAngle(std::numeric_limits<float>::quiet_NaN()),
  m_positionAlongWire(std::numeric_limits<float>::quiet_NaN()),
  m_rio(),
  m_idDE(),
  m_status(Trk::UNDECIDED),
  m_highLevel(false),
  m_timeOverThreshold(0.),
  m_detEl(nullptr)
{}

//copy constructor:
InDet::TRT_DriftCircleOnTrack::TRT_DriftCircleOnTrack( const InDet::TRT_DriftCircleOnTrack& rot)

= default;

//assignment operator:
InDet::TRT_DriftCircleOnTrack& InDet::TRT_DriftCircleOnTrack::operator=( const InDet::TRT_DriftCircleOnTrack& rot)
{
  if ( &rot != this) {
    Trk::RIO_OnTrack::operator= (rot);
    m_globalPosition = rot.m_globalPosition;
    m_rio                   = rot.m_rio;
    m_localAngle            = rot.m_localAngle;
    m_positionAlongWire     = rot.m_positionAlongWire;
    m_idDE                  = rot.m_idDE;
    m_status                = rot.m_status;
    m_highLevel             = rot.m_highLevel;
    m_timeOverThreshold     = rot.m_timeOverThreshold;
    m_detEl                 = rot.m_detEl;
   }
  return *this;
}

//move assignment operator:
InDet::TRT_DriftCircleOnTrack& InDet::TRT_DriftCircleOnTrack::operator=( InDet::TRT_DriftCircleOnTrack&& rot)
 noexcept {
  if ( &rot != this) {
    Trk::RIO_OnTrack::operator= (rot);
    m_globalPosition        = std::move(rot.m_globalPosition);
    m_rio                   = rot.m_rio;
    m_localAngle            = rot.m_localAngle;
    m_positionAlongWire     = rot.m_positionAlongWire;
    m_idDE                  = rot.m_idDE;
    m_status                = rot.m_status;
    m_highLevel             = rot.m_highLevel;
    m_timeOverThreshold     = rot.m_timeOverThreshold;
    m_detEl                 = rot.m_detEl;
   }
  return *this;
}

Trk::DriftCircleSide InDet::TRT_DriftCircleOnTrack::side() const{
  if (m_status == Trk::UNDECIDED) return Trk::NONE;
  if (localParameters()[Trk::driftRadius] < 0. ) return Trk::LEFT;
  return Trk::RIGHT;
}


const Trk::Surface& InDet::TRT_DriftCircleOnTrack::associatedSurface() const
{
    assert(0!=m_detEl);
    return (m_detEl->surface(identify()));
}

const Amg::Vector3D& InDet::TRT_DriftCircleOnTrack::globalPosition() const {

  return m_globalPosition;
}


//Global Position Helper for the converter
void
InDet::TRT_DriftCircleOnTrack::setGlobalPositionHelper()
{

  //default
  Amg::Vector3D loc3Dframe(0., 0., m_positionAlongWire);
  if (side() != Trk::NONE) {
    // get global position where track and drift radius intersect.
    double Sf, Cf;
    sincos(m_localAngle, &Sf, &Cf);
    double x = localParameters()[Trk::driftRadius] * Sf;
    double y = localParameters()[Trk::driftRadius] * Cf;
    /*
    double x = localParameters()[Trk::driftRadius]*std::sin(m_localAngle);
    double y = localParameters()[Trk::driftRadius]*std::cos(m_localAngle);
    */
    // get local position
    loc3Dframe = Amg::Vector3D(x, y, m_positionAlongWire);
  }

  //We need a surface for the global position
  const Trk::StraightLineSurface* slsf =
    dynamic_cast<const Trk::StraightLineSurface*>(&(associatedSurface()));
  if (slsf) {
    m_globalPosition = Amg::Vector3D(slsf->transform() * loc3Dframe);
  } else {
    throw GaudiException("Dynamic_cast to StraightLineSurface failed!",
                         "TRT_DriftCircleOnTrack::setGlobalPosition()",
                         StatusCode::FAILURE);
  }
}

//set Values to be used by the converter
void
InDet::TRT_DriftCircleOnTrack::setValues(const Trk::TrkDetElementBase* detEl,
                                         const Trk::PrepRawData*)
{
    m_detEl = dynamic_cast<const InDetDD::TRT_BaseElement* >(detEl);
    // If we have a m_detEL we can set the global position
    if (m_detEl) {
      setGlobalPositionHelper();
    }
}

MsgStream& InDet::TRT_DriftCircleOnTrack::dump( MsgStream& sl ) const
{
    Trk::RIO_OnTrack::dump(sl);
	std::string name("TRT_DriftCircleOnTrack: ");
	sl <<name<< "\t  identifier  = "<< identify()<<endmsg;
        sl <<name<< "\t  time-over-threshold = " << timeOverThreshold()
           << (highLevel() ? " with TR flag ON":" with TR flag OFF") << endmsg;
	sl <<name<< "\t  driftradius = ("
		 << (localParameters())[Trk::loc1] << ") " <<endmsg;
	sl <<name<< "\t  has Error Matrix: "<<endmsg;
	sl<<localCovariance()<<endmsg;
	return sl;
}

std::ostream& InDet::TRT_DriftCircleOnTrack::dump( std::ostream& sl ) const
{
    sl << "TRT_DriftCircleOnTrack {"<<std::endl;

    Trk::RIO_OnTrack::dump(sl);

    sl << "Global position (x,y,z) = (";
    this->globalPosition();
    sl  <<this->globalPosition().x()<<", "
      <<this->globalPosition().y()<<", "
      <<this->globalPosition().z()<<")"<<std::endl;

    sl << "\t  time-over-threshold = " << timeOverThreshold()
        << (highLevel() ? " with TR flag ON":" with TR flag OFF")<<std::endl;
    sl<<"}"<<std::endl;

	return sl;
}
