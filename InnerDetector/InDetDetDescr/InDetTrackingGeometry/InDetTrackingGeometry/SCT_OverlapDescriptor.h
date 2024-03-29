/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SCT_OverlapDescriptor.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef INDETTRACKINGGEOMETRY_SCT_OVERLAPDESCRIPTOR_H
#define INDETTRACKINGGEOMETRY_SCT_OVERLAPDESCRIPTOR_H

// Amg
#include "GeoPrimitives/GeoPrimitives.h"
// Trk
#include "TrkGeometry/OverlapDescriptor.h"
#include "TrkDetDescrUtils/Intersection.h"
// STL include
#include <atomic>

#ifndef TRKDETDESCR_SIDETADDNEXTPHIETA
#define TRKDETDESCR_SIDETADDNEXTPHIETA

#define addSurface(cur, surfaces)                             \
  if (cur)                                                    \
  surfaces.emplace_back(                                      \
      Trk::Intersection(Amg::Vector3D(0., 0., 0.), 0., true), \
      &(cur->surface(cur->identify())))

#define addOtherSide(cur, surfaces)                           \
  if (cur && cur->otherSide())                                \
  surfaces.emplace_back(                                      \
      Trk::Intersection(Amg::Vector3D(0., 0., 0.), 0., true), \
      &(cur->otherSide()->surface(cur->otherSide()->identify())))

#define addNextInPhiOS(cur, surfaces)     \
  addSurface(cur->nextInPhi(), surfaces); \
  addOtherSide(cur->nextInPhi(), surfaces)

#define addPrevInPhiOS(cur, surfaces)     \
  addSurface(cur->prevInPhi(), surfaces); \
  addOtherSide(cur->prevInPhi(), surfaces)

#define addNextInEtaOS(cur, surfaces)     \
  addSurface(cur->nextInEta(), surfaces); \
  addOtherSide(cur->nextInEta(), surfaces)

#define addPrevInEtaOS(cur, surfaces)     \
  addSurface(cur->prevInEta(), surfaces); \
  addOtherSide(cur->prevInEta(), surfaces)
#endif

namespace Trk {
  class Surface;
}


namespace InDetDD {   
     class SiDetectorElement;
}

class SCT_ID;
          
namespace InDet {

    /** @class SCT_OverlapDescriptor
         
        Class to describe overlaps in the SCT detector,
        it extends the Trk::OverlapDescriptor base class.
                
        @author Andreas.Salzburger@cern.ch        
        */
        
     class SCT_OverlapDescriptor : public Trk::OverlapDescriptor {
       public:

         /** Constructor */
         SCT_OverlapDescriptor(bool addMoreSurfaces = false, int eta_slices = 4);
         
         /** Destructor */
         virtual ~SCT_OverlapDescriptor() = default;
         
         /**Pseudo-Constructor*/
         SCT_OverlapDescriptor* clone() const;
     
         /** get the compatible surfaces */
         bool reachableSurfaces(std::vector<Trk::SurfaceIntersection>& cSurfaces, 
                                const Trk::Surface& sf,
                                const Amg::Vector3D& pos,
                                const Amg::Vector3D& dir) const;

        private:
          bool dumpSurfaces(std::vector<Trk::SurfaceIntersection>& surfaces) const;
          bool                                 m_robustMode;
          bool                                 m_addMoreSurfaces;
          int                                  m_etaSlices;         
          mutable std::atomic<const SCT_ID*>   m_sctIdHelper{nullptr};
    };

  inline SCT_OverlapDescriptor* SCT_OverlapDescriptor::clone() const { return new SCT_OverlapDescriptor(); }     
          
}

#endif
