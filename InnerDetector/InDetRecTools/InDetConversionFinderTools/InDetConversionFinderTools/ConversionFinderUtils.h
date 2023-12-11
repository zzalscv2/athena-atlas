/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETCONVERSIONFINDERTOOLS_CONVERSIONFINDERUTILS_H
#define INDETCONVERSIONFINDERTOOLS_CONVERSIONFINDERUTILS_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "Particle/TrackParticle.h"
#include "xAODTracking/Vertex.h"

class MsgStream;

namespace Trk {
  class Track;
}

namespace InDet {

  /**
     @class ConversionFinderUtils
     Some helper tools like:
     * hits counter
     @author Tatjana Lenz , Thomas Koffas
  */

  class ConversionFinderUtils final : public AthAlgTool {

  public:
    ConversionFinderUtils (const std::string& type,const std::string& name, const IInterface* parent);

    virtual ~ConversionFinderUtils();

    static const InterfaceID& interfaceID();

    virtual StatusCode initialize() override;

    virtual StatusCode finalize() override;

    /** helper functions */
    /** Momentum fraction of tracks in pair. */
    static double momFraction(const Trk::TrackParameters* per1,
                       const Trk::TrackParameters* per2) ;
    /** Approximate distance of minimum approach between tracks in pair. */
    double distBetweenTracks(const Trk::Track* trk_pos,
                             const Trk::Track* trk_neg) const;
    /** Add new perigee to track. */
    static std::unique_ptr<Trk::Track> addNewPerigeeToTrack(
        const Trk::Track* track, const Trk::Perigee* mp);
    /** Correct VxCandidate with respect to a user defined vertex.  */
    static xAOD::Vertex* correctVxCandidate(xAOD::Vertex*, Amg::Vector3D) ;

  };

}
#endif // INDETCONVERSIONFINDERTOOLS_CONVERSIONFINDERUTILS_H

