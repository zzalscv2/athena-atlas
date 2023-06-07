/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BASETRACKVERTEXASSOCIATIONTOOL_H
#define BASETRACKVERTEXASSOCIATIONTOOL_H

// Includes from this package
#include "TrackVertexAssociationTool/ITrackVertexAssociationTool.h"
#include "TrackVertexAssociationTool/TrackVertexAssociationMap.h"

// FrameWork includes
#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"
#include "AthLinks/ElementLink.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"

// EDM includes
#include "xAODTracking/TrackParticleFwd.h"
#include "xAODTracking/TrackParticleContainerFwd.h"
#include "xAODTracking/VertexFwd.h"
#include "xAODTracking/VertexContainerFwd.h"
#include "xAODEventInfo/EventInfo.h"

// STL includes
#include <string>
#include <vector>

namespace CP {

/// General track-vertex association tool, can currently be used to match
/// electron or muon tracks.
///
/// Supported properties (set via setProperty):
/// - dzSinTheta_cut: Cut on \f$ |\Delta z_{0}^{BL} \sin \theta| \f$ (in mm).  Not applied if set to -1.
/// - d0sig_cut: Cut on \f$ |d_{0}^{BL} \mathrm{significance}| \f$.  Not applied if set to -1.
///
/// To apply the recommendations for lepton tracks from
/// https://twiki.cern.ch/twiki/bin/view/AtlasProtected/InDetTrackingPerformanceGuidelines#Track_to_Vertex_Association,
/// use the subclasses ElectronTrackVertexAssociationTool and MuonTrackVertexAssociationTool.
class BaseTrackVertexAssociationTool
    : public asg::AsgTool,
      virtual public ITrackVertexAssociationTool
{

  ASG_TOOL_CLASS(BaseTrackVertexAssociationTool, ITrackVertexAssociationTool)
public:
  BaseTrackVertexAssociationTool(const std::string& name);

  StatusCode initialize();

  void print() const;

  /// Returns true if the track trk is compatible with the vertex vx.
  ///
  /// If the vertex is not a good vertex (`vx->vertexType() != xAOD::VxType::NoVtx`), returns false.
  bool isCompatible(const xAOD::TrackParticle &trk,
                    const xAOD::Vertex &vx) const;
  bool isCompatible(const xAOD::TrackParticle &trk) const;

  xAOD::TrackVertexAssociationMap
  getMatchMap(std::vector<const xAOD::TrackParticle *> &trk_list,
              std::vector<const xAOD::Vertex *> &vx_list) const;

  xAOD::TrackVertexAssociationMap
  getMatchMap(const xAOD::TrackParticleContainer &trkCont,
              const xAOD::VertexContainer &vxCont) const;

  /// Compares the track trk with each track in vx_list using isCompatible,
  /// and returns the compatible track with the smallest Δz * sin θ.
  const xAOD::Vertex *
  getUniqueMatchVertex(const xAOD::TrackParticle &trk,
                       std::vector<const xAOD::Vertex *> &vx_list) const;

  ElementLink<xAOD::VertexContainer>
  getUniqueMatchVertexLink(const xAOD::TrackParticle &,
                           const xAOD::VertexContainer &vxCont) const;

  /// Returns the best matched tracks for each vertex, in a
  /// map<const xAOD::Vertex*, std::vector<const xAOD::TrackParticle*> >
  /// (defined as TrackVertexAssociationMap).
  ///
  /// Note: the dummy vertex will be also stored in the map, but the track
  /// vector assigned to this vertex will be empty
  xAOD::TrackVertexAssociationMap
  getUniqueMatchMap(std::vector<const xAOD::TrackParticle *> &trk_list,
                    std::vector<const xAOD::Vertex *> &vx_list) const;

  xAOD::TrackVertexAssociationMap
  getUniqueMatchMap(const xAOD::TrackParticleContainer &trkCont,
                    const xAOD::VertexContainer &vxCont) const;

private:
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo { this, "EventInfo", "EventInfo", "EventInfo key" };
  
  SG::ReadDecorHandleKey<xAOD::EventInfo> m_hardScatterDecoKey{this, "VertexScatterKey", "" , 
                              "Overwrriten in the configuration step by the <HardScatterLinkDeco> property"};
  /// Checks if a track-vertex pair passes the cuts.  Returns
  /// Δz * sin θ of the pair in `dzSinTheta` if successful.
  bool isMatch(const xAOD::TrackParticle &trk, const xAOD::Vertex &vx,
               float &dzSinTheta, const xAOD::EventInfo* evtInfo = nullptr) const;

  template <typename T, typename U>
  xAOD::TrackVertexAssociationMap getMatchMapImpl(T &trk_list,
                                                  U &vx_list) const;

  template <typename T, typename U>
  xAOD::TrackVertexAssociationMap getUniqueMatchMapImpl(T &trk_list,
                                                        U &vx_list) const;

  template <typename T>
  const xAOD::Vertex *getUniqueMatchVertexImpl(const xAOD::TrackParticle &trk,
                                               T &vx_list) const;

  
  /// Cut on d0 significance
  Gaudi::Property<float> m_d0sig_cut{this, "d0sig_cut", -1.};
  /// Cut on dz*sin theta
  Gaudi::Property<float> m_dzSinTheta_cut{this, "dzSinTheta_cut", 1.};
  /// The decoration name of the ElementLink to the hardscatter vertex (applied to xAOD::EventInfo)
  Gaudi::Property<std::string> m_hardScatterDeco{this, "HardScatterLinkDeco", "hardScatterVertexLink",
                              "The decoration name of the ElementLink to the hardscatter vertex (found on xAOD::EventInfo)"};

};

} // namespace CP

#endif // BASETRACKVERTEXASSOCIATIONTOOL_H

// vim: expandtab tabstop=8 shiftwidth=2 softtabstop=2
