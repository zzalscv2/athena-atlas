/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETTRACKFITTER_TRIGINDETTRACKFOLLOWINGTOOL_H
#define TRIGINDETTRACKFITTER_TRIGINDETTRACKFOLLOWINGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

#include "TrigInDetToolInterfaces/ITrigInDetTrackFollowingTool.h"

// MagField cache
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "MagFieldElements/AtlasFieldCache.h"

#include "InDetPrepRawData/PixelClusterContainer.h"
#include "InDetPrepRawData/SCT_ClusterContainer.h"

namespace Trk {	
  class PrepRawData;
  class PlaneSurface;
}

struct TrigFTF_HitAssignment {
  TrigFTF_HitAssignment() = delete;
  ~TrigFTF_HitAssignment() {};
  TrigFTF_HitAssignment(const TrigFTF_HitAssignment&) = delete;
  TrigFTF_HitAssignment& operator=(const TrigFTF_HitAssignment&) = delete;
  
TrigFTF_HitAssignment(const Trk::PrepRawData* p, double const* X, double const* C, double dchi2, int ndof) : m_pPRD(p), m_dchi2(dchi2), m_ndof(ndof) {
  if(m_pPRD!=nullptr) {
    memcpy(&m_Xk[0], X, sizeof(m_Xk));
    memcpy(&m_Ck[0], C, sizeof(m_Ck));
  }
  else {
    memset(&m_Xk[0], 0, sizeof(m_Xk));
    memset(&m_Ck[0], 0, sizeof(m_Ck));
  }
}

  const Trk::PrepRawData* m_pPRD;
  double m_Xk[5];
  double m_Ck[15];
  double m_dchi2;
  int m_ndof;
};

struct TrigFTF_ExtendedTrackState {

  TrigFTF_ExtendedTrackState() = delete;
  ~TrigFTF_ExtendedTrackState() {};
  TrigFTF_ExtendedTrackState& operator=(const TrigFTF_ExtendedTrackState&) = delete;

  TrigFTF_ExtendedTrackState(double const*, const Trk::PlaneSurface*);

  void AddHole();
  void AddHit(const Trk::PrepRawData*, double, int);
  void SwapTheEnds();
  
  void report() const;

  double m_Xk[10];
  double m_Gk[10][10];
  double m_chi2;
  double m_ndof;

  const Trk::PlaneSurface* m_pS;//moving plane
  const Trk::PlaneSurface* m_pO;//initial plane

  std::list<TrigFTF_HitAssignment> m_track;
  
  int m_nClusters;
  int m_nHoles;

  bool m_isSwapped;
  
};

class TrigInDetTrackFollowingTool: public AthAlgTool, virtual public ITrigInDetTrackFollowingTool
{
 public:
  TrigInDetTrackFollowingTool( const std::string&, const std::string&, const IInterface* );
  virtual StatusCode initialize();
  virtual StatusCode finalize();

  virtual Trk::Track* getTrack(const std::vector<const Trk::SpacePoint*>&, const std::vector<const InDetDD::SiDetectorElement*>&, const EventContext&) const;

private:

  std::unique_ptr<TrigFTF_ExtendedTrackState> fitTheSeed(const std::vector<const Trk::SpacePoint*>&, MagField::AtlasFieldCache&) const;

  const InDet::PixelCluster* findBestPixelHit(const InDet::PixelClusterCollection*, const TrigFTF_ExtendedTrackState&) const;
  const InDet::SCT_Cluster* findBestStripHit(const InDet::SCT_ClusterCollection*, const TrigFTF_ExtendedTrackState&, int) const;

  void update(const InDet::PixelCluster*, TrigFTF_ExtendedTrackState&) const;
  void update(const InDet::SCT_Cluster*, int, TrigFTF_ExtendedTrackState&) const;
  
  int extrapolateTrackState(TrigFTF_ExtendedTrackState&, const Trk::PlaneSurface*, MagField::AtlasFieldCache&) const;

  int RungeKutta34(double*, double*, const Trk::PlaneSurface*, MagField::AtlasFieldCache&, bool) const;

  bool checkIntersection(double const*, const Trk::PlaneSurface*, const Trk::PlaneSurface*, MagField::AtlasFieldCache&) const;

  inline void crossProduct(double const *, double const *, double*) const;

  inline void rotateToLocal(const double (&GL)[3][3], const double*, double (&Y)[3]) const;

  void transformJacobianToLocal(const Trk::PlaneSurface*, double const *, double const *, double*) const;

  double estimateRK_Step(const Trk::PlaneSurface*, double const *) const;
  
  Gaudi::Property<int> m_nClustersMin {this, "nClustersMin", 7, "Minimum number of clusters on track"};
  Gaudi::Property<int> m_nHolesMax {this, "nHolesMax", 4, "Maximum number of holes on track"};
  Gaudi::Property<double> m_maxChi2Dist_Pixels {this, "Chi2MaxPixels", 50.0, "the Pixel hit chi2 cut"};
  Gaudi::Property<double> m_maxChi2Dist_Strips {this, "Chi2MaxStrips", 50.0, "the Strip hit chi2 cut"};
  Gaudi::Property<bool> m_useHitErrors {this, "UseHitErrors", false, "use PrepRawData errors"};
  Gaudi::Property<bool> m_useDetectorThickness {this, "UseDetectorThickness", false, "get Si-modules thickness from InDet Geometry"};
  Gaudi::Property<double> m_nominalRadLength {this, "ModuleRadLength", 0.024, "fixed radiation thickness of the detector modules"};

  SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCondObjInputKey {this, "AtlasFieldCacheCondObj", "fieldCondObj", "Name of the Magnetic Field conditions object key"};
  SG::ReadHandleKey<InDet::PixelClusterContainer> m_pixcontainerkey{this, "PixelClusterContainer","ITkPixelClusters"};
  SG::ReadHandleKey<InDet::SCT_ClusterContainer> m_sctcontainerkey{this, "SCT_ClusterContainer", "ITkStripClusters"};
  
};

#endif
