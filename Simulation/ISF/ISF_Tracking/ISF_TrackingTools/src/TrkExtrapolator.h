/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrkExtrapolator.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ISF_TRACKINGTOOLS_TRKEXTRAPOLATOR_H
#define ISF_TRACKINGTOOLS_TRKEXTRAPOLATOR_H 1

// class header
#include "ISF_TrackingInterfaces/ITrkExtrapolator.h"

// STL includes
#include <string>

// FrameWork includes
#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "CxxUtils/CachedPointer.h"
#include "CxxUtils/checker_macros.h"

// ISF includes
#include "ISF_Event/ISFParticle.h"
#include "TrkGeometry/TrackingGeometry.h"

namespace Trk {
  class IExtrapolator;
  class TrackingGeometry;
  class TrackingVolume;
  class PdgToParticleHypothesis;
}

namespace ISF {

   class ISFParticle;


  /** @class TrkExtrapolator

      An Athena AlgTool wrapper for the Tracking Extrapolator engine.

      @author Elmar.Ritsch@cern.ch (refactored original implementation by Robert Harrington <roberth@cern.ch>)
     */
  class TrkExtrapolator : public extends<AthAlgTool, ISF::ITrkExtrapolator> {

    public:
      /** Constructor with parameters */
      TrkExtrapolator( const std::string& t, const std::string& n, const IInterface* p );

      /** Destructor */
      ~TrkExtrapolator();

      /** Athena AlgTool initialization **/
      virtual StatusCode  initialize() override;
      /** Athena AlgTool finalization **/
      virtual StatusCode  finalize() override;

      /** Extrapolate the given ISFParticle */
      virtual ISF::ISFParticle* extrapolate( const ISF::ISFParticle &particle ) const override;

    private:

      /** tracking geometry for geometry signature */
      SG::ReadCondHandleKey<Trk::TrackingGeometry>      m_trackingGeometryReadKey{this, "TrackingGeometryReadKey", "AtlasTrackingGeometry", "Key of input TrackingGeometry"};  

      /** extrapolation to calo entry */
      ToolHandle<Trk::IExtrapolator>       m_extrapolator;              //!< ToolHandle for track extrapolator

      std::string                               m_trackingVolumeName;   //!< name of the volume within the extrapolation is carried out
      mutable CxxUtils::CachedPointer<const Trk::TrackingVolume> m_trackingVolume ATLAS_THREAD_SAFE;       //!< volume within the extrapolation is carried out

      Trk::PdgToParticleHypothesis*             m_pdgToParticleHypothesis; //!< converts PDG ID to hypothesis for TrackParameters


  };

}


#endif //> !ISF_TRACKINGTOOLS_TRKEXTRAPOLATOR_H
