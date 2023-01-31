/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// Navigator.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKEXTOOLS_NAVIGATOR_H
#define TRKEXTOOLS_NAVIGATOR_H

// Gaudi
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
// Trk
#include "TrkExInterfaces/INavigator.h"
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkVolumes/BoundarySurface.h"
#include "TrkGeometry/MagneticFieldProperties.h"
#include "TrkParameters/TrackParameters.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkDetDescrInterfaces/ITrackingGeometrySvc.h"

#include <cstring>
#include <Gaudi/Accumulators.h>

namespace Trk {

  class IGeometryBuilder;
  class IPropagator;
  class Surface;
  class Track;
  class TrackingVolume;
  typedef std::pair<const NavigationCell*,const NavigationCell*> NavigationPair;

 /**
     @class Navigator

     Main AlgTool for Navigation in the TrkExtrapolation realm :
     It retrieves the TrackingGeometry from the DetectorStore
     as the reference Geometry.

     There's an experimental possibility to use a straightLineApproximation for the
     Navigation. This is unstable due to wrong cylinder intersections.

     @author Andreas.Salzburger@cern.ch
     */

  class Navigator : public AthAlgTool,
                    virtual public INavigator {
    public:
      /** Constructor */
      Navigator(const std::string&, const std::string&, const IInterface*);
      /** Destructor */
      virtual ~Navigator() = default;

      /** AlgTool initialize method.*/
      virtual StatusCode initialize() override;

      /** INavigator interface method - returns the TrackingGeometry used for
       * navigation */
      virtual const TrackingGeometry* trackingGeometry(
        const EventContext& ctx) const override final;

      /** INavigator interface methods - global search for the Volume one is in
       */
      virtual const TrackingVolume* volume(
        const EventContext& ctx, const Amg::Vector3D& gp) const override final;

      /** INavigator interface method - forward hightes TrackingVolume */
      virtual const TrackingVolume* highestVolume(
        const EventContext& ctx) const override final;

      /** INavigator interface method - getting the closest TrackParameters from
       * a Track to a Surface*/
      virtual const TrackParameters* closestParameters(
        const EventContext& ctx,
        const Track& trk,
        const Surface& sf,
        const IPropagator* prop = nullptr) const override final;

      /** INavigator method to resolve navigation at boundary */
      virtual bool atVolumeBoundary(const Trk::TrackParameters* parms,
                                    const Trk::TrackingVolume* vol,
                                    Trk::PropDirection dir,
                                    const Trk::TrackingVolume*& nextVol,
                                    double tol) const override final;

      /** INavigator interface methods - getting the next BoundarySurface not
       * knowing the Volume*/
      virtual const BoundarySurface<TrackingVolume>* nextBoundarySurface(
        const EventContext& ctx,
        const IPropagator& prop,
        const TrackParameters& parms,
        PropDirection dir) const override final;

      /** INavigator interface methods - getting the next BoundarySurface when
       * knowing the Volume*/
      virtual const BoundarySurface<TrackingVolume>* nextBoundarySurface(
        const EventContext& ctx,
        const IPropagator& prop,
        const TrackParameters& parms,
        PropDirection dir,
        const TrackingVolume& vol) const override final;

      /** INavigator interface method - getting the next Volume and the
       * parameter for the next Navigation*/
      virtual NavigationCell nextTrackingVolume(
        const EventContext& ctx,
        const IPropagator& prop,
        const TrackParameters& parms,
        PropDirection dir,
        const TrackingVolume& vol) const override final;

    private:


      SG::ReadCondHandleKey<TrackingGeometry> m_trackingGeometryReadKey{
        this,
        "TrackingGeometryKey",
        "AtlasTrackingGeometry",
        "Key of output of TrackingGeometry for ID"
      };

      /// ToolHandle to the TrackingGeometrySvc
      ServiceHandle<Trk::ITrackingGeometrySvc> m_trackingGeometrySvc{
        this,
        "TrackingGeometrySvc",
        ""
      };
      /// Name of the TrackingGeometry as given in Detector Store
      std::string m_trackingGeometryName;
      /******************************************************************/
      /// Tolerance for inside() method of Volumes
      double m_insideVolumeTolerance;
      /// Tolerance for isOnSurface() method of BoundarySurfaces
      double m_isOnSurfaceTolerance;
      bool m_useConditions{};
      Trk::MagneticFieldProperties m_fieldProperties;
      /// use the straight line approximation for the next boundary sf
      bool m_useStraightLineApproximation;
      /// search with new distanceToSurface() method
      bool m_searchWithDistance;
      //------------ Magnetic field properties
      bool m_fastField;
    };

} // end of namespace


#endif // TRKEXTOOLS_NAVIGATOR_H

