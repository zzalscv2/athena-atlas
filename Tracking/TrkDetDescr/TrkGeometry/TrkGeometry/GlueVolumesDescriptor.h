/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// GlueVolumesDescriptor.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKGEOMETRY_GLUEVOLUMESDESCRIPTOR_H
#define TRKGEOMETRY_GLUEVOLUMESDESCRIPTOR_H

// Trk include
#include "TrkVolumes/BoundarySurfaceFace.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
// STL
#include <map>
#include <vector>


namespace Trk {

class TrackingVolume;

typedef std::map<BoundarySurfaceFace,
                 std::vector<TrackingVolume*> >::iterator
    GlueVolumeIterator;
typedef std::map<BoundarySurfaceFace,
                 std::vector<TrackingVolume*> >::const_iterator
    GlueVolumeConstIterator;
/** @class GlueVolumesDescriptor

   Descriptor class to hold GlueVolumes of a TrackingGeometry object.
   Should ease the wrapping of a TrackingGeometry object describing one Detector
   by another one.

   @author Andreas.Salzburger@cern.ch
   */

class GlueVolumesDescriptor {
 public:
  /**Constructor */
  GlueVolumesDescriptor() {}

  /**Constructor - with arguments*/
  GlueVolumesDescriptor(
      const std::map<BoundarySurfaceFace, std::vector<TrackingVolume*> >&
          glv);

  /**Desctructor */
  ~GlueVolumesDescriptor() {}

  /** register the volumes */
  void registerGlueVolumes(BoundarySurfaceFace,
                           std::vector<TrackingVolume*>&);

  /** retrieve them again */
  const std::vector<TrackingVolume*>& glueVolumes(
      BoundarySurfaceFace);

  /** retrieve the available Glue Faces */
  const std::vector<BoundarySurfaceFace>& glueFaces() const;

 private:
  std::map<BoundarySurfaceFace, std::vector<TrackingVolume*> >
      m_glueVolumes;
  std::vector<BoundarySurfaceFace> m_glueFaces;
  static const std::vector<TrackingVolume*> s_emptyVector;
};

inline const std::vector<BoundarySurfaceFace>&
GlueVolumesDescriptor::glueFaces() const {
  return m_glueFaces;
}

/**Overload of << operator for both, MsgStream and std::ostream for debug
 * output*/
MsgStream& operator<<(MsgStream& sl, GlueVolumesDescriptor& mprop);
std::ostream& operator<<(std::ostream& sl, GlueVolumesDescriptor& mprop);

}  // namespace Trk

#endif
