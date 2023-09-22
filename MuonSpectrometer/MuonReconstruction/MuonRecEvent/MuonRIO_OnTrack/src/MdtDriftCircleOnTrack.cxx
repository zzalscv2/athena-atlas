/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "MuonRIO_OnTrack/MdtDriftCircleOnTrack.h"
#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkSurfaces/StraightLineSurface.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/StatusCode.h"
#include <cmath>


// Full Constructor :
namespace Muon{
MdtDriftCircleOnTrack::MdtDriftCircleOnTrack(const MdtPrepData* RIO,
                                             const Trk::LocalParameters& locPos,
                                             const Amg::MatrixX& errDriftRadius,
                                             const double driftTime,
                                             const Trk::DriftCircleStatus status,
                                             const Amg::Vector3D& predictedTrackDirection,
                                             const double positionAlongWire,
                                             const MuonDriftCircleErrorStrategy& errorStrategy) :
  Trk::RIO_OnTrack{locPos, errDriftRadius, RIO->identify()}, 
  m_status{status},
  m_positionAlongWire{positionAlongWire},
  m_driftTime{driftTime},
  m_errorStrategy{errorStrategy} {
    
    assert(m_status!=Trk::UNDECIDED); // use of this constructor implies that the side is decided

    m_rio.setElement(RIO);
    m_globalPosition.store(std::make_unique<Amg::Vector3D>(associatedSurface().localToGlobal(locPos, predictedTrackDirection, positionAlongWire)));
    
    Amg::Vector3D loc_gDirection = predictedTrackDirection;

    ///scaling the direction with drift radius
    if(driftRadius() > std::numeric_limits<float>::epsilon()) {
        // Set loc_gDirection's perpendicular distance equal to driftRadius
        Amg::setPerp(loc_gDirection, std::abs(driftRadius()));
        const double ratio = loc_gDirection.x() / std::abs(driftRadius());
        const double calc_angle = std::abs(ratio) >= 1. ?  M_PI * (ratio < 0.) : std::acos(ratio);
        m_localAngle = (loc_gDirection.y()<0.) ? 2*M_PI - calc_angle : calc_angle;
    }
}

// Partial Constructor :
MdtDriftCircleOnTrack::MdtDriftCircleOnTrack(const MdtPrepData* RIO,
                                             const Trk::LocalParameters& locPos,
                                             const Amg::MatrixX& errDriftRadius,
                                             const double driftTime,
                                             const Trk::DriftCircleStatus status,
                                             const double positionAlongWire,
                                             const MuonDriftCircleErrorStrategy& errorStrategy) :
    Trk::RIO_OnTrack{locPos, errDriftRadius, RIO->identify()},
    m_status{status},
    m_positionAlongWire{positionAlongWire},
    m_driftTime{driftTime},
    m_errorStrategy{errorStrategy} {    
    assert(m_status!=Trk::DECIDED); // use of this constructor implies that the side is not decided
    m_rio.setElement(RIO);
}
 
MdtDriftCircleOnTrack::MdtDriftCircleOnTrack(const MdtDriftCircleOnTrack & other):
    Trk::RIO_OnTrack{ other },
    m_status{other.m_status},
    m_rio{other.m_rio},
    m_localAngle{other.m_localAngle},
    m_positionAlongWire{other.m_positionAlongWire},
    m_driftTime{other.m_driftTime},
    m_errorStrategy{other.m_errorStrategy},
    m_detEl{other.m_detEl} {
}
MdtDriftCircleOnTrack::MdtDriftCircleOnTrack(const ElementLinkToIDC_MDT_Container& RIO,
                                             const Trk::LocalParameters& locPos,
                                             const Amg::MatrixX& errDriftRadius,
                                             const Identifier& id,
                                             const MuonGM::MdtReadoutElement* detEl,
                                             const double driftTime,
                                             const Trk::DriftCircleStatus status,
                                             const double positionAlongWire,
                                             const double localAngle,
                                             const MuonDriftCircleErrorStrategy& errorStrategy):
    Trk::RIO_OnTrack{locPos, errDriftRadius, id},
    m_status{status},
    m_rio{RIO},    
    m_localAngle{localAngle},
    m_positionAlongWire{positionAlongWire},
    m_driftTime{driftTime},
    m_errorStrategy{errorStrategy},
    m_detEl{detEl} {}

   
MdtDriftCircleOnTrack & MdtDriftCircleOnTrack::operator=(const MdtDriftCircleOnTrack & other) {
     if (&other != this){
         static_cast<Trk::RIO_OnTrack&>(*this) = other;        
         m_status = other.m_status;
         m_rio = other.m_rio;
         m_localAngle = other.m_localAngle;
         m_driftTime = other.m_driftTime;
         m_errorStrategy = other.m_errorStrategy;
         m_positionAlongWire = other.m_positionAlongWire;
         m_detEl = other.m_detEl;
         m_globalPosition.release();        
     }
     return *this;
}
const Amg::Vector3D& MdtDriftCircleOnTrack::globalPosition() const {
    if (!m_globalPosition) {
        if (side()==Trk::NONE) {
            // side not defined, so we cannot determine the global position better than the position along the wire
            Amg::Vector3D  loc3Dframe = m_positionAlongWire * Amg::Vector3D::UnitZ();
            setGlobalPosition(std::move(loc3Dframe));
        } else {
            // get global position where track and drift radius intersect.
            double x = driftRadius()*std::sin(m_localAngle);
            double y = driftRadius()*std::cos(m_localAngle);
            Amg::Vector3D loc3Dframe(x, y, m_positionAlongWire);
            setGlobalPosition(std::move(loc3Dframe));
        }
    }
    return *m_globalPosition;
}

void MdtDriftCircleOnTrack::setGlobalPosition(Amg::Vector3D&& loc3Dframe) const {
    Amg::Vector3D globPos = associatedSurface().transform() * loc3Dframe;
    m_globalPosition.set(std::make_unique<Amg::Vector3D>(std::move(globPos)));
}

MsgStream& MdtDriftCircleOnTrack::dump( MsgStream&    stream) const
{
    stream << MSG::INFO<<"MdtDriftCircleOnTrack {"<<std::endl;
    Trk::RIO_OnTrack::dump(stream);
    stream << "DriftTime: "<<m_driftTime<<std::endl;
    stream << "Status: "<<m_status<<std::endl;
    stream << "Global position (x,y,z) = ";
    stream << Amg::toString(globalPosition(),2)<<std::endl;

    stream << "Position along wire: "<<positionAlongWire()<<", \tlocal angle="<<localAngle()<<std::endl;
    stream << "Creation strategy: "<<m_errorStrategy;
    stream<<"}"<<endmsg;
    return stream;
}

std::ostream& MdtDriftCircleOnTrack::dump( std::ostream&    stream) const
{
    stream << "MdtDriftCircleOnTrack {"<<std::endl;
    Trk::RIO_OnTrack::dump(stream);
    stream << "DriftTime: "<<m_driftTime<<std::endl;
    stream << "Status: "<<m_status<<std::endl;
   
    stream << "Position along wire: "<<positionAlongWire()<<", \tlocal angle="<<localAngle()<<std::endl;
    // stream << "Creation strategy: "<<m_errorStrategy; //FIXME!
    stream<<"}"<<std::endl;
    return stream;
}

}

