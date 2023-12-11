/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackToVertex.cxx, (c) ATLAS Detector software 2005
///////////////////////////////////////////////////////////////////

// Reco & Rec
#include "TrackToVertex.h"
#include "Particle/TrackParticle.h"
// Trk
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkSurfaces/StraightLineSurface.h"
#include "TrkDetDescrUtils/GeometryStatics.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkTrack/Track.h"
#include "VxVertex/VxCandidate.h"
#include "VxVertex/RecVertex.h"


const Amg::Vector3D Reco::TrackToVertex::s_origin(0.,0.,0.);

// constructor
Reco::TrackToVertex::TrackToVertex(const std::string& t, const std::string& n, const IInterface* p) :
  base_class(t,n,p)
{
}

// initialize
StatusCode Reco::TrackToVertex::initialize()
{
    // Get the GeometryBuilder AlgTool
    ATH_CHECK(m_extrapolator.retrieve());
    ATH_MSG_DEBUG( name() << " initialize() successful");
    return StatusCode::SUCCESS;
}


// incident listener waiting for BeginEvent
std::unique_ptr<Trk::StraightLineSurface> Reco::TrackToVertex::GetBeamLine(const InDet::BeamSpotData* beamSpotHandle) const {
    // get the transform
    Amg::Transform3D beamTransform = Amg::Transform3D(Amg::AngleAxis3D(beamSpotHandle->beamTilt(0),Amg::Vector3D(0.,1.,0.)));
    beamTransform *= Amg::AngleAxis3D(beamSpotHandle->beamTilt(1),Amg::Vector3D(1.,0.,0.));
    beamTransform.pretranslate(beamSpotHandle->beamPos());
    // create the new beam line
    return std::make_unique< Trk::StraightLineSurface >(beamTransform);
}

// finalize
StatusCode Reco::TrackToVertex::finalize()
{
    ATH_MSG_DEBUG( name() << " finalize() successful");
    return StatusCode::SUCCESS;
}


std::unique_ptr<Trk::Perigee> Reco::TrackToVertex::perigeeAtVertex(const EventContext& ctx, const Rec::TrackParticle& tp) const {

  // retrieve the reconstructed Vertex from the TrackParticle
  const Trk::VxCandidate* vxCandidate = tp.reconstructedVertex();
  if (vxCandidate!=nullptr) {
     // create a global position from this
     const Trk::RecVertex& reconVertex = vxCandidate->recVertex();
     const Amg::Vector3D& vertexPosition = reconVertex.position();
     Amg::Vector3D persfPosition(vertexPosition.x(), vertexPosition.y(), vertexPosition.z());
     return(this->perigeeAtVertex(ctx, tp, persfPosition));
  }
  ATH_MSG_DEBUG("No reconstructed vertex found in TrackParticle, perigee will be expressed to (0.,0.,0.).");
  return (perigeeAtVertex(ctx, tp,Trk::s_origin));
}

std::unique_ptr<Trk::Perigee> Reco::TrackToVertex::perigeeAtVertex(const EventContext& ctx, const xAOD::TrackParticle& tp) const {
    return perigeeAtVertex(ctx, tp, Amg::Vector3D(tp.vx(),tp.vy(),tp.vz()));
}


std::unique_ptr<Trk::Perigee> Reco::TrackToVertex::perigeeAtVertex(const EventContext& ctx, const xAOD::TrackParticle& tp, const Amg::Vector3D& gp) const {

  // preparation
  Trk::PerigeeSurface persf(gp);
  std::unique_ptr<Trk::Perigee> vertexPerigee;
  // retrieve the Perigee from the track particle
  const Trk::Perigee& trackparPerigee = tp.perigeeParameters();
  if (trackparPerigee.associatedSurface() == persf) {
    ATH_MSG_DEBUG("Perigee of TrackParticle is already expressed to given "
                  "vertex, a copy is returned.");
    return std::unique_ptr<Trk::Perigee>(trackparPerigee.clone());
  } else {
    std::unique_ptr<Trk::TrackParameters> extrapResult =
      m_extrapolator->extrapolateDirectly(ctx,trackparPerigee, persf);
    if (extrapResult && extrapResult->surfaceType() == Trk::SurfaceType::Perigee) {
      vertexPerigee.reset(static_cast<Trk::Perigee*>(extrapResult.release()));
    }
  }
  if (!vertexPerigee)
    ATH_MSG_DEBUG(
      "Extrapolation to Perigee failed, a NULL pointer is returned.");
  return vertexPerigee;
}

std::unique_ptr<Trk::Perigee> Reco::TrackToVertex::perigeeAtVertex(const EventContext& ctx, const Rec::TrackParticle& tp, const Amg::Vector3D& gp) const {

  // preparation
  Trk::PerigeeSurface persf(gp);
  std::unique_ptr<Trk::Perigee> vertexPerigee = nullptr;
  // retrieve the Perigee from the track particle
  const Trk::Perigee* trackparPerigee = tp.measuredPerigee();
  if (trackparPerigee){
     if ( trackparPerigee->associatedSurface() == persf)
     {
       ATH_MSG_DEBUG("Perigee of TrackParticle is already expressed to given vertex, a copy is returned.");
       return std::unique_ptr<Trk::Perigee>(trackparPerigee->clone());
     } else {
       auto extrapResult =
         m_extrapolator->extrapolateDirectly(ctx,*trackparPerigee, persf);
       if (extrapResult &&
           extrapResult->surfaceType() == Trk::SurfaceType::Perigee) {
         vertexPerigee.reset(static_cast<Trk::Perigee*>(extrapResult.release()));
       }
     }
  } else {
    ATH_MSG_DEBUG(
      "No Perigee found in  TrackParticle, a NULL pointer is returned.");
    return nullptr;
  }
  if (!vertexPerigee){
    ATH_MSG_DEBUG(
      "Extrapolation to Perigee failed, a NULL pointer is returned.");
  }
  return vertexPerigee;
}


std::unique_ptr<Trk::Perigee> Reco::TrackToVertex::perigeeAtVertex(const EventContext& ctx, const Trk::Track& track, const Amg::Vector3D& gp) const {

  Trk::PerigeeSurface persf(gp);
  std::unique_ptr<Trk::Perigee> vertexPerigee;
  std::unique_ptr<Trk::TrackParameters> extrapResult =
    m_extrapolator->extrapolateTrack(ctx,track, persf);
  if (extrapResult && extrapResult->surfaceType() == Trk::SurfaceType::Perigee) {
    vertexPerigee.reset( static_cast<Trk::Perigee*>(extrapResult.release()));
  }
  if (!vertexPerigee) {
    const Trk::Perigee* trackPerigee = track.perigeeParameters();
    if (trackPerigee && trackPerigee->associatedSurface() == persf) {
      ATH_MSG_DEBUG("Perigee of Track is already expressed to given vertex, a "
                    "copy is returned.");
      vertexPerigee.reset(trackPerigee->clone());
    } else{
      ATH_MSG_DEBUG(
        "Extrapolation to Perigee failed, NULL pointer is returned.");
    }
  }
  return (vertexPerigee);
}


std::unique_ptr<Trk::Perigee>
Reco::TrackToVertex::perigeeAtBeamline(
  const EventContext& ctx,
  const Trk::Track& track,
  const InDet::BeamSpotData* beamspotptr) const
{

  Amg::Vector3D beamspot(s_origin);
  float tiltx = 0.0;
  float tilty = 0.0;
  if (beamspotptr) {
    beamspot = Amg::Vector3D(beamspotptr->beamVtx().position());
    tiltx =  beamspotptr->beamTilt(0);
    tilty =  beamspotptr->beamTilt(1);
  }
  Amg::Translation3D amgtranslation(beamspot);
  Amg::Transform3D pAmgTransf =
    amgtranslation * Amg::RotationMatrix3D::Identity();
  pAmgTransf *= Amg::AngleAxis3D(tilty, Amg::Vector3D(0., 1., 0.));
  pAmgTransf *= Amg::AngleAxis3D(tiltx, Amg::Vector3D(1., 0., 0.));
  // preparation
  Trk::PerigeeSurface persf(pAmgTransf);

  std::unique_ptr<Trk::Perigee> vertexPerigee;
  std::unique_ptr<Trk::TrackParameters> extrapResult =
    m_extrapolator->extrapolateTrack(ctx,track, persf);
  if (extrapResult && extrapResult->surfaceType() == Trk::SurfaceType::Perigee) {
    vertexPerigee.reset(static_cast<Trk::Perigee*>(extrapResult.release()));
  }
  if (!vertexPerigee) {
    // workaround.
    // try again using the first track parameter set, since the current extrapolator will
    // use "the closest" track parameterset which is not necessarily the mostuseful one to
    // start the extrapolation with.
    const DataVector<const Trk::TrackParameters> *track_parameter_list= track.trackParameters();
    if (track_parameter_list) {
      for(const Trk::TrackParameters *trk_params: *track_parameter_list) {
        if (!trk_params) {
          continue;
        }
        extrapResult = m_extrapolator->extrapolate(ctx,*trk_params, persf);
        if (extrapResult &&
            extrapResult->surfaceType() == Trk::SurfaceType::Perigee) {
          vertexPerigee.reset(static_cast<Trk::Perigee*>(extrapResult.release()));
        }
        break;
      }
    }
  }
  if (!vertexPerigee) {
    const Trk::Perigee* trackPerigee = track.perigeeParameters();
    if (trackPerigee && trackPerigee->associatedSurface() == persf) {
      ATH_MSG_DEBUG("Perigee of Track is already expressed to given vertex, a "
                    "copy is returned.");
      vertexPerigee.reset(trackPerigee->clone());
    } else {
      ATH_MSG_DEBUG(
        "Extrapolation to Beamline Perigee failed, NULL pointer is returned.");
    }
  }
  return (vertexPerigee);
}

std::unique_ptr<Trk::TrackParameters> Reco::TrackToVertex::trackAtBeamline(const EventContext&, const Rec::TrackParticle& /*tp*/) const
{
  ATH_MSG_WARNING(" Method not implemented!! ");
  return {};
  //return m_extrapolator->extrapolate(tp, *m_beamLine);
}

std::unique_ptr<Trk::TrackParameters> Reco::TrackToVertex::trackAtBeamline(const EventContext& ctx, const xAOD::TrackParticle& tp,
                const InDet::BeamSpotData* beamspotptr) const
{

  Amg::Vector3D beamspot(s_origin);
  float tiltx = 0.0;
  float tilty = 0.0;
  if (beamspotptr) {
    beamspot = Amg::Vector3D(beamspotptr->beamVtx().position());
    tiltx =  beamspotptr->beamTilt(0);
    tilty =  beamspotptr->beamTilt(1);
  }
  Amg::Transform3D amgTransf;
  Amg::Translation3D amgtranslation(beamspot);
  amgTransf = amgtranslation * Amg::RotationMatrix3D::Identity();
  amgTransf *= Amg::AngleAxis3D(tilty, Amg::Vector3D(0.,1.,0.));
  amgTransf *= Amg::AngleAxis3D(tiltx, Amg::Vector3D(1.,0.,0.));
 // preparation
  Trk::PerigeeSurface persf(amgTransf);
  std::unique_ptr<Trk::TrackParameters> vertexPerigee;
  // retrieve the Perigee from the track particle
  const Trk::Perigee& trackparPerigee = tp.perigeeParameters();
  if ( trackparPerigee.associatedSurface() == persf) {
       ATH_MSG_DEBUG("Perigee of TrackParticle is already expressed to given vertex, a copy is returned.");
       return std::unique_ptr<Trk::TrackParameters>(trackparPerigee.clone());
  } else
      vertexPerigee = m_extrapolator->extrapolateDirectly(ctx,trackparPerigee, persf);
  if (!vertexPerigee){
     ATH_MSG_DEBUG("Extrapolation to Beam Line failed, a NULL pointer is returned.");
  }
  return vertexPerigee;

}

std::unique_ptr<Trk::TrackParameters> Reco::TrackToVertex::trackAtBeamline(const EventContext& ctx, const Trk::Track& trk,
                                const Trk::StraightLineSurface* beamline) const
{
  return m_extrapolator->extrapolateTrack(ctx, trk, *beamline);
}

std::unique_ptr<Trk::TrackParameters> Reco::TrackToVertex::trackAtBeamline(const EventContext& ctx, const Trk::TrackParameters& tpars,
                                const Trk::StraightLineSurface* beamline) const
{
  return m_extrapolator->extrapolate(ctx, tpars, *beamline);
}


