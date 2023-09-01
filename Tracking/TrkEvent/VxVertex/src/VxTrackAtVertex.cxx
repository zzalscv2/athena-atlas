/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
                           VxTrackAtVertex.cxx  -  Description
                             -------------------
    begin   : Autumn 2003
    authors : Andreas Wildauer (CERN PH-ATC), Fredrik Akesson (CERN PH-ATC)
    email   : andreas.wildauer@cern.ch, fredrik.akesson@cern.ch
    comments: original version by M. Elsing
    changes :

 ***************************************************************************/

#include "VxVertex/VxTrackAtVertex.h"
#include "GaudiKernel/MsgStream.h"

namespace Trk {

VxTrackAtVertex::VxTrackAtVertex()
  : Trk::ObjectCounter<Trk::VxTrackAtVertex>()
  , m_fitQuality(Trk::FitQuality(0., 2.))
  , m_trkWeight(1.)
  , m_VertexCompatibility(0.)
  , m_perigeeAtVertex(nullptr)
  , m_neutralPerigeeAtVertex(nullptr)
  , m_linState(nullptr)
  , m_initialPerigee(nullptr)
  , m_initialNeutralPerigee(nullptr)
  , m_ImpactPoint3dAtaPlane(nullptr)
  , m_ImpactPoint3dNeutralAtaPlane(nullptr)
  , m_trackOrParticleLink(nullptr)
{
}

VxTrackAtVertex::VxTrackAtVertex(double chi2PerTrk,
                                 Trk::TrackParameters* perigeeAtVertex,
                                 Trk::NeutralParameters* neutralPerigeeAtVertex)
  : Trk::ObjectCounter<Trk::VxTrackAtVertex>()
  , m_fitQuality(Trk::FitQuality(chi2PerTrk, 2.))
  , m_trkWeight(1.)
  , m_VertexCompatibility(0.)
  , m_perigeeAtVertex(perigeeAtVertex)
  , m_neutralPerigeeAtVertex(neutralPerigeeAtVertex)
  , m_linState(nullptr)
  , m_initialPerigee(nullptr)
  , m_initialNeutralPerigee(nullptr)
  , m_ImpactPoint3dAtaPlane(nullptr)
  , m_ImpactPoint3dNeutralAtaPlane(nullptr)
  , m_trackOrParticleLink(nullptr)
{
}

VxTrackAtVertex::VxTrackAtVertex(double chi2PerTrk,
                                 Trk::TrackParameters* perigeeAtVertex,
                                 Trk::NeutralParameters* neutralPerigeeAtVertex,
                                 double ndfPerTrk,
                                 Trk::LinearizedTrack* linState)
  : Trk::ObjectCounter<Trk::VxTrackAtVertex>()
  , m_fitQuality(Trk::FitQuality(chi2PerTrk, ndfPerTrk))
  , m_trkWeight(1.)
  , m_VertexCompatibility(0.)
  , m_perigeeAtVertex(perigeeAtVertex)
  , m_neutralPerigeeAtVertex(neutralPerigeeAtVertex)
  , m_linState(linState)
  , m_initialPerigee(nullptr)
  , m_initialNeutralPerigee(nullptr)
  , m_ImpactPoint3dAtaPlane(nullptr)
  , m_ImpactPoint3dNeutralAtaPlane(nullptr)
  , m_trackOrParticleLink(nullptr)
{
}
VxTrackAtVertex::VxTrackAtVertex(double chi2PerTrk,
                                 Trk::TrackParameters* perigeeAtVertex,
                                 double ndfPerTrk,
                                 Trk::LinearizedTrack* linState)
  : Trk::ObjectCounter<Trk::VxTrackAtVertex>()
  , m_fitQuality(Trk::FitQuality(chi2PerTrk, ndfPerTrk))
  , m_trkWeight(1.)
  , m_VertexCompatibility(0.)
  , m_perigeeAtVertex(perigeeAtVertex)
  , m_neutralPerigeeAtVertex(nullptr)
  , m_linState(linState)
  , m_initialPerigee(nullptr)
  , m_initialNeutralPerigee(nullptr)
  , m_ImpactPoint3dAtaPlane(nullptr)
  , m_ImpactPoint3dNeutralAtaPlane(nullptr)
  , m_trackOrParticleLink(nullptr)
{
}

// new constructors, including initial perigee state (refitting support)
// Kirill Prokofiev 27-03-06
VxTrackAtVertex::VxTrackAtVertex(
  double chi2PerTrk,
  Trk::TrackParameters* perigeeAtVertex,
  Trk::NeutralParameters* neutralPerigeeAtVertex,
  const Trk::TrackParameters* initialPerigee,
  const Trk::NeutralParameters* initialNeutralPerigee)
  : Trk::ObjectCounter<Trk::VxTrackAtVertex>()
  , m_fitQuality(Trk::FitQuality(chi2PerTrk, 2.))
  , m_trkWeight(1.)
  , m_VertexCompatibility(0.)
  , m_perigeeAtVertex(perigeeAtVertex)
  , m_neutralPerigeeAtVertex(neutralPerigeeAtVertex)
  , m_linState(nullptr)
  , m_initialPerigee(initialPerigee)
  , m_initialNeutralPerigee(initialNeutralPerigee)
  , m_ImpactPoint3dAtaPlane(nullptr)
  , m_ImpactPoint3dNeutralAtaPlane(nullptr)
  , m_trackOrParticleLink(nullptr)
{
}

VxTrackAtVertex::VxTrackAtVertex(double chi2PerTrk,
                                 Trk::TrackParameters* perigeeAtVertex,
                                 const Trk::TrackParameters* initialPerigee)
  : Trk::ObjectCounter<Trk::VxTrackAtVertex>()
  , m_fitQuality(Trk::FitQuality(chi2PerTrk, 2.))
  , m_trkWeight(1.)
  , m_VertexCompatibility(0.)
  , m_perigeeAtVertex(perigeeAtVertex)
  , m_neutralPerigeeAtVertex(nullptr)
  , m_linState(nullptr)
  , m_initialPerigee(initialPerigee)
  , m_initialNeutralPerigee(nullptr)
  , m_ImpactPoint3dAtaPlane(nullptr)
  , m_ImpactPoint3dNeutralAtaPlane(nullptr)
  , m_trackOrParticleLink(nullptr)
{
}

VxTrackAtVertex::VxTrackAtVertex(
  double chi2PerTrk,
  Trk::TrackParameters* perigeeAtVertex,
  Trk::NeutralParameters* neutralPerigeeAtVertex,
  const Trk::TrackParameters* initialPerigee,
  const Trk::NeutralParameters* initialNeutralPerigee,
  double ndfPerTrk,
  Trk::LinearizedTrack* linState)
  : Trk::ObjectCounter<Trk::VxTrackAtVertex>()
  , m_fitQuality(Trk::FitQuality(chi2PerTrk, ndfPerTrk))
  , m_trkWeight(1.)
  , m_VertexCompatibility(0.)
  , m_perigeeAtVertex(perigeeAtVertex)
  , m_neutralPerigeeAtVertex(neutralPerigeeAtVertex)
  , m_linState(linState)
  , m_initialPerigee(initialPerigee)
  , m_initialNeutralPerigee(initialNeutralPerigee)
  , m_ImpactPoint3dAtaPlane(nullptr)
  , m_ImpactPoint3dNeutralAtaPlane(nullptr)
  , m_trackOrParticleLink(nullptr)
{
}
VxTrackAtVertex::VxTrackAtVertex(double chi2PerTrk,
                                 Trk::TrackParameters* perigeeAtVertex,
                                 const Trk::TrackParameters* initialPerigee,
                                 double ndfPerTrk,
                                 Trk::LinearizedTrack* linState)
  : Trk::ObjectCounter<Trk::VxTrackAtVertex>()
  , m_fitQuality(Trk::FitQuality(chi2PerTrk, ndfPerTrk))
  , m_trkWeight(1.)
  , m_VertexCompatibility(0.)
  , m_perigeeAtVertex(perigeeAtVertex)
  , m_neutralPerigeeAtVertex(nullptr)
  , m_linState(linState)
  , m_initialPerigee(initialPerigee)
  , m_initialNeutralPerigee(nullptr)
  , m_ImpactPoint3dAtaPlane(nullptr)
  , m_ImpactPoint3dNeutralAtaPlane(nullptr)
  , m_trackOrParticleLink(nullptr)
{
}

VxTrackAtVertex::VxTrackAtVertex(Trk::ITrackLink* trackOrParticleLink)
  : Trk::ObjectCounter<Trk::VxTrackAtVertex>()
  , m_fitQuality(Trk::FitQuality(0., 2.))
  , m_trkWeight(1.)
  , m_VertexCompatibility(0.)
  , m_perigeeAtVertex(nullptr)
  , m_neutralPerigeeAtVertex(nullptr)
  , m_linState(nullptr)
  , m_initialPerigee(nullptr)
  , m_initialNeutralPerigee(nullptr)
  , m_ImpactPoint3dAtaPlane(nullptr)
  , m_ImpactPoint3dNeutralAtaPlane(nullptr)
  , m_trackOrParticleLink(nullptr)
{
  this->setOrigTrack(trackOrParticleLink);
}

// destructor
VxTrackAtVertex::~VxTrackAtVertex()
{
    delete m_perigeeAtVertex;
    delete m_neutralPerigeeAtVertex;
    delete m_linState;
    delete m_ImpactPoint3dAtaPlane;
    delete m_ImpactPoint3dNeutralAtaPlane;
    delete m_trackOrParticleLink;
}

// copy constructor
VxTrackAtVertex::VxTrackAtVertex(const VxTrackAtVertex& rhs)
  : Trk::ObjectCounter<Trk::VxTrackAtVertex>(rhs)
  , m_fitQuality(rhs.m_fitQuality)
  , m_trkWeight(rhs.m_trkWeight)
  , m_VertexCompatibility(rhs.m_VertexCompatibility)
  , m_perigeeAtVertex(rhs.m_perigeeAtVertex ? rhs.m_perigeeAtVertex->clone()
                                            : nullptr)
  , m_neutralPerigeeAtVertex(rhs.m_neutralPerigeeAtVertex
                               ? rhs.m_neutralPerigeeAtVertex->clone()
                               : nullptr)
  , m_linState(rhs.m_linState ? rhs.m_linState->clone() : nullptr)
  ,
  m_initialPerigee(rhs.m_initialPerigee)
  , m_initialNeutralPerigee(rhs.m_initialNeutralPerigee)
  , m_ImpactPoint3dAtaPlane(rhs.m_ImpactPoint3dAtaPlane
                              ? new AtaPlane(*rhs.m_ImpactPoint3dAtaPlane)
                              : nullptr)
  , m_ImpactPoint3dNeutralAtaPlane(
      rhs.m_ImpactPoint3dNeutralAtaPlane
        ? new NeutralAtaPlane(*rhs.m_ImpactPoint3dNeutralAtaPlane)
        : nullptr)
  , m_trackOrParticleLink(
      rhs.m_trackOrParticleLink ? rhs.m_trackOrParticleLink->clone() : nullptr)
{
}

// assignment operator changed
VxTrackAtVertex&
VxTrackAtVertex::operator=(const VxTrackAtVertex& rhs)
{
  if (this != &rhs) {
    m_fitQuality = rhs.m_fitQuality;
    m_trkWeight = rhs.m_trkWeight;
    delete m_perigeeAtVertex;
    m_perigeeAtVertex =
      rhs.m_perigeeAtVertex ? rhs.m_perigeeAtVertex->clone() : nullptr;
    delete m_neutralPerigeeAtVertex;
    m_neutralPerigeeAtVertex = rhs.m_neutralPerigeeAtVertex
                                 ? rhs.m_neutralPerigeeAtVertex->clone()
                                 : nullptr;
    delete m_linState;
    m_linState = rhs.m_linState ? rhs.m_linState->clone() : nullptr;
    //       m_initialPerigee = rhs.m_initialPerigee ?
    //       rhs.m_initialPerigee->clone() : 0;
    m_initialPerigee = rhs.m_initialPerigee;
    m_initialNeutralPerigee = rhs.m_initialNeutralPerigee;
    delete m_ImpactPoint3dAtaPlane;
    m_ImpactPoint3dAtaPlane = rhs.m_ImpactPoint3dAtaPlane
                                ? new AtaPlane(*rhs.m_ImpactPoint3dAtaPlane)
                                : nullptr;
    delete m_ImpactPoint3dNeutralAtaPlane;
    m_ImpactPoint3dNeutralAtaPlane =
      rhs.m_ImpactPoint3dNeutralAtaPlane
        ? new NeutralAtaPlane(*rhs.m_ImpactPoint3dNeutralAtaPlane)
        : nullptr;
    m_VertexCompatibility = rhs.m_VertexCompatibility;
    delete m_trackOrParticleLink;
    m_trackOrParticleLink =
      (rhs.m_trackOrParticleLink ? rhs.m_trackOrParticleLink->clone()
                                 : nullptr);
  }
  return *this;
}

VxTrackAtVertex::VxTrackAtVertex(VxTrackAtVertex&& rhs) noexcept
  : m_fitQuality(rhs.m_fitQuality)
  , m_trkWeight(rhs.m_trkWeight)
{
  m_perigeeAtVertex = rhs.m_perigeeAtVertex;
  rhs.m_perigeeAtVertex = nullptr;

  m_neutralPerigeeAtVertex = rhs.m_neutralPerigeeAtVertex;
  rhs.m_neutralPerigeeAtVertex = nullptr;

  m_linState = rhs.m_linState;
  rhs.m_linState = nullptr;

  m_initialPerigee = rhs.m_initialPerigee;
  m_initialNeutralPerigee = rhs.m_initialNeutralPerigee;
  m_VertexCompatibility = rhs.m_VertexCompatibility;

  m_ImpactPoint3dAtaPlane = rhs.m_ImpactPoint3dAtaPlane;
  rhs.m_ImpactPoint3dAtaPlane = nullptr;

  m_ImpactPoint3dNeutralAtaPlane = rhs.m_ImpactPoint3dNeutralAtaPlane;
  rhs.m_ImpactPoint3dNeutralAtaPlane = nullptr;

  m_trackOrParticleLink = rhs.m_trackOrParticleLink;
  rhs.m_trackOrParticleLink = nullptr;
}

VxTrackAtVertex&
VxTrackAtVertex::operator=(VxTrackAtVertex&& rhs) noexcept
{
  if (this != &rhs) {
    m_fitQuality = rhs.m_fitQuality;
    m_trkWeight = rhs.m_trkWeight;

    delete m_perigeeAtVertex;
    m_perigeeAtVertex = rhs.m_perigeeAtVertex;
    rhs.m_perigeeAtVertex = nullptr;

    delete m_neutralPerigeeAtVertex;
    m_neutralPerigeeAtVertex = rhs.m_neutralPerigeeAtVertex;
    rhs.m_neutralPerigeeAtVertex = nullptr;

    delete m_linState;
    m_linState = rhs.m_linState;
    rhs.m_linState = nullptr;

    m_initialPerigee = rhs.m_initialPerigee;
    m_initialNeutralPerigee = rhs.m_initialNeutralPerigee;
    m_VertexCompatibility = rhs.m_VertexCompatibility;

    delete m_ImpactPoint3dAtaPlane;
    m_ImpactPoint3dAtaPlane = rhs.m_ImpactPoint3dAtaPlane;
    rhs.m_ImpactPoint3dAtaPlane = nullptr;

    delete m_ImpactPoint3dNeutralAtaPlane;
    m_ImpactPoint3dNeutralAtaPlane = rhs.m_ImpactPoint3dNeutralAtaPlane;
    rhs.m_ImpactPoint3dNeutralAtaPlane = nullptr;

    delete m_trackOrParticleLink;
    m_trackOrParticleLink = rhs.m_trackOrParticleLink;
    rhs.m_trackOrParticleLink = nullptr;
  }
  return *this;
}

bool
VxTrackAtVertex::operator==(const VxTrackAtVertex& data) const
{

  return (this->initialPerigee() == data.initialPerigee());
}

const TrackParameters*
VxTrackAtVertex::initialPerigee(void) const
{
  if (m_initialPerigee != nullptr)
    return m_initialPerigee; // as default always return the proper object (be
                             // consistent!)
  if (m_trackOrParticleLink != nullptr)
    return m_trackOrParticleLink->parameters();
  return nullptr;
}

const NeutralParameters*
VxTrackAtVertex::initialNeutralPerigee(void) const
{
  if (m_initialNeutralPerigee != nullptr)
    return m_initialNeutralPerigee; // as default always return the proper
                                    // object (be consistent!)
  if (m_trackOrParticleLink != nullptr)
    return m_trackOrParticleLink->neutralParameters();
  return nullptr;
}

VxTrackAtVertex*
VxTrackAtVertex::clone() const
{
  return new VxTrackAtVertex(*this);
}

void
VxTrackAtVertex::setImpactPoint3dAtaPlane(const AtaPlane* myIP3dAtaPlane)
{
  if (m_ImpactPoint3dAtaPlane != nullptr)
    delete m_ImpactPoint3dAtaPlane;
  m_ImpactPoint3dAtaPlane = myIP3dAtaPlane;
}

void
VxTrackAtVertex::setImpactPoint3dNeutralAtaPlane(
  const NeutralAtaPlane* myIP3dNeutralAtaPlane)
{
  if (m_ImpactPoint3dNeutralAtaPlane != nullptr)
    delete m_ImpactPoint3dNeutralAtaPlane;
  m_ImpactPoint3dNeutralAtaPlane = myIP3dNeutralAtaPlane;
}

void
VxTrackAtVertex::setPerigeeAtVertex(TrackParameters* perigee)
{
  if (m_perigeeAtVertex != nullptr)
    delete m_perigeeAtVertex;
  m_perigeeAtVertex = perigee;
}

void
VxTrackAtVertex::setPerigeeAtVertex(NeutralParameters* perigee)
{
  if (m_neutralPerigeeAtVertex != nullptr)
    delete m_neutralPerigeeAtVertex;
  m_neutralPerigeeAtVertex = perigee;
}

void
VxTrackAtVertex::setLinTrack(LinearizedTrack* myLinTrack)
{
  if (m_linState != nullptr)
    delete m_linState;
  m_linState = myLinTrack;
}

void
VxTrackAtVertex::setOrigTrack(ITrackLink* trk)
{
  delete m_trackOrParticleLink;
  // setting first the actual pointer:
  m_trackOrParticleLink = trk;

  // setting the initial perigee to be consistent with this
  // new track
  if (trk != nullptr) {
    // delete initialPerigee before, otherwise memleak!!
    //      if (m_initialPerigee!=0) { delete m_initialPerigee;
    //      m_initialPerigee=0; } const TrackParameters * tmpInPer =
    //      trk->parameters();
    m_initialPerigee = trk->parameters();
    m_initialNeutralPerigee = trk->neutralParameters();
  } else {
    m_initialPerigee = nullptr;
    m_initialNeutralPerigee = nullptr;
  }
} // end of setOrigTrack method

void
VxTrackAtVertex::setOrigTrackNoCache(ITrackLink* trk)
{
  delete m_trackOrParticleLink;
  m_trackOrParticleLink = trk;

  m_initialPerigee = nullptr;
  m_initialNeutralPerigee = nullptr;
} // end of setOrigTrackNocache method


MsgStream&
VxTrackAtVertex::dump(MsgStream& sl) const
{
  sl << "Printing VxTrackAtVertex:" << endmsg;
  const Trk::TrackParameters* inPerigee = this->initialPerigee();
  if (inPerigee) {
    sl << "Original Perigee: " << *(inPerigee) << endmsg;
  } else {
    sl << "This VxTrackAtVertex was not created using initial perigee"
       << endmsg;
  }
  if (this->perigeeAtVertex() != nullptr) {
    sl << "Charged Refitted Perigee: " << *(this->perigeeAtVertex()) << endmsg;
  } else {
    sl << "Charged Refitted Perigee was not created/stored in pool file." << endmsg;
  }
  if (this->neutralPerigeeAtVertex() != nullptr) {
    sl << "Neutral Refitted Perigee: " << *(this->neutralPerigeeAtVertex()) << std::endl;
  } else {
    sl << "Neutral Refitted Perigee was not created/stored in pool file." << std::endl;
  }

  sl << m_fitQuality << "\tWeight: " << m_trkWeight << endmsg;
  return sl;
}

std::ostream&
VxTrackAtVertex::dump(std::ostream& sl) const
{
  sl << "Printing VxTrackAtVertex:" << std::endl;
  const Trk::TrackParameters* inPerigee = this->initialPerigee();
  if (inPerigee) {
    sl << "Original Perigee: " << *(inPerigee) << std::endl;
  } else {
    sl << "This VxTrackAtVertex was not created using initial perigee"
       << std::endl;
  }
  if (this->perigeeAtVertex() != nullptr) {
    sl << "Charged Refitted Perigee: " << *(this->perigeeAtVertex()) << std::endl;
  } else {
    sl << "Charged Refitted Perigee was not created/stored in pool file." << std::endl;
  }
  if (this->neutralPerigeeAtVertex() != nullptr) {
    sl << "Neutral Refitted Perigee: " << *(this->neutralPerigeeAtVertex()) << std::endl;
  } else {
    sl << "Neutral Refitted Perigee was not created/stored in pool file." << std::endl;
  }
  sl << m_fitQuality << "\tWeight: " << m_trkWeight << std::endl;
  return sl;
}

MsgStream&
operator<<(MsgStream& sl, const VxTrackAtVertex& sf)
{
  return sf.dump(sl);
}

std::ostream&
operator<<(std::ostream& sl, const VxTrackAtVertex& sf)
{
  return sf.dump(sl);
}

} // end of namespace
