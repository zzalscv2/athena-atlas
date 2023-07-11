/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


#ifndef JPSIUPSILONCOMMON
#define JPSIUPSILONCOMMON

#include <vector>
#include "xAODTracking/TrackParticleFwd.h"
#include "xAODMuon/MuonContainer.h"
#include <algorithm>
#include "xAODTracking/VertexContainerFwd.h"
#include "xAODTracking/Vertex.h"
#include <array>
namespace xAOD{
   class BPhysHelper;
}

namespace Analysis {
   class PrimaryVertexRefitter;
   class CleanUpVertex{
       const xAOD::Vertex* m_vtx;
       bool m_cleanup;
    public:
       const xAOD::Vertex* get() const { return m_vtx; }
       ~CleanUpVertex(){ if (m_cleanup) delete  m_vtx; }
       CleanUpVertex(const xAOD::Vertex* vtx, bool cleanup) : m_vtx(vtx), m_cleanup(cleanup) {}
       CleanUpVertex(const CleanUpVertex&) = delete;
       CleanUpVertex(CleanUpVertex&& vtx) noexcept {
           m_vtx = vtx.m_vtx;
           m_cleanup = vtx.m_cleanup;
           vtx.m_cleanup = false;
           vtx.m_vtx = nullptr;
       }
       CleanUpVertex & operator=(const CleanUpVertex&) = delete;
   };

   class JpsiUpsilonCommon{
    public:

        static double getPt(const xAOD::TrackParticle*, const xAOD::TrackParticle*);
        static double getPt(const xAOD::TrackParticle* trk1, const xAOD::TrackParticle* trk2, const xAOD::TrackParticle* trk3);
        static double getPt(const xAOD::TrackParticle*, const xAOD::TrackParticle*, const xAOD::TrackParticle*, const xAOD::TrackParticle*);
        static bool   isContainedIn(const xAOD::TrackParticle*, const std::vector<const xAOD::TrackParticle*>&);
        static bool   isContainedIn(const xAOD::TrackParticle*, const xAOD::MuonContainer*);
        static bool   cutRangeOR(const std::vector<double> &values, double min, double max);
        static bool   cutRange(double value, double min, double max);
        static bool   cutAcceptGreaterOR(const std::vector<double> &values, double min);
        static bool   cutAcceptGreater(double value, double min);
        static Analysis::CleanUpVertex ClosestRefPV(xAOD::BPhysHelper&, const xAOD::VertexContainer*,const Analysis::PrimaryVertexRefitter*);
        template< size_t N>
        static bool isContainedIn(const xAOD::TrackParticle*, const std::array<const xAOD::TrackParticle*, N>& );
        static void RelinkVertexTracks(const std::vector<const xAOD::TrackParticleContainer*> &trkcols, xAOD::Vertex* vtx);
        static void RelinkVertexMuons(const std::vector<const xAOD::MuonContainer*>& muoncols, xAOD::Vertex* vtx);

   };

template< size_t N>
bool JpsiUpsilonCommon::isContainedIn(const xAOD::TrackParticle* t, const std::array<const xAOD::TrackParticle*, N>& cont )
 {
    return std::find(cont.begin(), cont.end(), t) != cont.end();
 }
}

#endif

