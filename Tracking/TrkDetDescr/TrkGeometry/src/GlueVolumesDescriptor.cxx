/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// GlueVolumesDescriptor.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkGeometry/GlueVolumesDescriptor.h"

#include "TrkGeometry/TrackingVolume.h"

const std::vector<Trk::TrackingVolume*>
    Trk::GlueVolumesDescriptor::s_emptyVector;

Trk::GlueVolumesDescriptor::GlueVolumesDescriptor(
    const std::map<Trk::BoundarySurfaceFace,
                   std::vector<Trk::TrackingVolume*> >& glv)
    : m_glueVolumes(glv) {
  Trk::GlueVolumeIterator searchIter = m_glueVolumes.begin();
  Trk::GlueVolumeIterator endIter = m_glueVolumes.end();
  // fill the available faces
  for (; searchIter != endIter; ++searchIter)
    m_glueFaces.push_back(searchIter->first);
}

void Trk::GlueVolumesDescriptor::registerGlueVolumes(
    Trk::BoundarySurfaceFace bsf,
    std::vector<Trk::TrackingVolume*>& gvs) {
  // register the face
  Trk::GlueVolumeIterator searchIter = m_glueVolumes.begin();
  searchIter = m_glueVolumes.find(bsf);
  if (searchIter == m_glueVolumes.end()) m_glueFaces.push_back(bsf);
  // simple assignment overwrites already existing entries
  m_glueVolumes[bsf] = gvs;  //!< @todo change to addGlueVolumes principle
}

const std::vector<Trk::TrackingVolume*>&
Trk::GlueVolumesDescriptor::glueVolumes(Trk::BoundarySurfaceFace bsf){
  Trk::GlueVolumeConstIterator searchIter = m_glueVolumes.begin();
  Trk::GlueVolumeConstIterator endIter = m_glueVolumes.end();

  searchIter = m_glueVolumes.find(bsf);
  if (searchIter != endIter) return searchIter->second;
  return s_emptyVector;
}

MsgStream& Trk::operator<<(MsgStream& sl,
                           Trk::GlueVolumesDescriptor& gvd) {
  sl << "Trk::GlueVolumesDescriptor: " << std::endl;
  const std::vector<Trk::BoundarySurfaceFace>& glueFaceVector = gvd.glueFaces();
  sl << "     has Tracking Volumes registered for : " << glueFaceVector.size()
     << " Volume faces." << std::endl;
  std::vector<Trk::BoundarySurfaceFace>::const_iterator glueFaceIter =
      glueFaceVector.begin();
  std::vector<Trk::BoundarySurfaceFace>::const_iterator glueFaceIterEnd =
      glueFaceVector.end();
  // loop over the faces
  for (; glueFaceIter != glueFaceIterEnd; ++glueFaceIter) {
    const std::vector<Trk::TrackingVolume*>& glueVolumesVector =
      gvd.glueVolumes(*glueFaceIter);
    auto glueVolumeIter = glueVolumesVector.begin();
    auto glueVolumeIterEnd = glueVolumesVector.end();
    // loop over the TrackingVolumes
    sl << "        -----> Processing Face: " << int(*glueFaceIter) << " - has ";
    sl << glueVolumesVector.size()
       << " TrackingVolumes marked as 'GlueVolumes' " << std::endl;
    for (; glueVolumeIter != glueVolumeIterEnd; ++glueVolumeIter)
      sl << "             - TrackingVolume: " << (*glueVolumeIter)->volumeName()
         << std::endl;
  }
  return sl;
}

std::ostream& Trk::operator<<(std::ostream& sl,
                              GlueVolumesDescriptor& gvd) {
  sl << "Trk::GlueVolumesDescriptor: " << std::endl;
  const std::vector<Trk::BoundarySurfaceFace>& glueFaceVector = gvd.glueFaces();
  sl << "     has Tracking Volumes registered for : " << glueFaceVector.size()
     << " Volume faces." << std::endl;
  std::vector<Trk::BoundarySurfaceFace>::const_iterator glueFaceIter =
      glueFaceVector.begin();
  std::vector<Trk::BoundarySurfaceFace>::const_iterator glueFaceIterEnd =
      glueFaceVector.end();
  // loop over the faces
  for (; glueFaceIter != glueFaceIterEnd; ++glueFaceIter) {
    const std::vector<Trk::TrackingVolume*>& glueVolumesVector =
        gvd.glueVolumes(*glueFaceIter);
    auto glueVolumeIter = glueVolumesVector.begin();
    auto glueVolumeIterEnd = glueVolumesVector.end();
    // loop over the TrackingVolumes
    sl << "        -----> Processing Face: " << int(*glueFaceIter) << " - has ";
    sl << glueVolumesVector.size()
       << " TrackingVolumes marked as 'GlueVolumes' " << std::endl;
    for (; glueVolumeIter != glueVolumeIterEnd; ++glueVolumeIter)
      sl << "             - TrackingVolume: " << (*glueVolumeIter)->volumeName()
         << std::endl;
  }
  return sl;
}
