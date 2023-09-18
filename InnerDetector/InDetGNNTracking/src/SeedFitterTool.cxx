/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "SeedFitterTool.h"
#include "TrkPrepRawData/PrepRawData.h"

#include <cmath>


InDet::SeedFitterTool::SeedFitterTool(
  const std::string& type, const std::string& name, const IInterface* parent):
    base_class(type, name, parent)
{
  declareInterface<ISeedFitter>(this);
}

std::unique_ptr<const Trk::TrackParameters> InDet::SeedFitterTool::fit(
  const std::vector<const Trk::SpacePoint*>& spacePoints) const
{
  //// @todo maybe use a even simplier version to estimate track parameters.
  //// Taken from the following link:
  //// https://gitlab.cern.ch/xju/athena/-/blob/master/InnerDetector/InDetRecTools/SiTrackMakerTool_xk/src/SiTrackMaker_xk.cxx#L851-993

  //// Only the first 3 spacepoints are used.
  //// the fitting was not stable. Now require at least 5 SPs.

  if (spacePoints.size() < 3) {
    return nullptr;
  }

  double track_paras[9];

  const Trk::PrepRawData*       cl  = spacePoints[0]->clusterList().first;
  if(!cl) return nullptr;
  const Trk::PlaneSurface*      pla = 
    static_cast<const Trk::PlaneSurface*>(&cl->detectorElement()->surface());
  if(!pla) return nullptr;

  // translate second and third SP w.r.t first one
  double x0 = spacePoints[0]->globalPosition().x();
  double y0 = spacePoints[0]->globalPosition().y();
  double z0 = spacePoints[0]->globalPosition().z();

  double x1 = spacePoints[1]->globalPosition().x() - x0;
  double y1 = spacePoints[1]->globalPosition().y() - y0;

  double x2 = spacePoints[2]->globalPosition().x() - x0;
  double y2 = spacePoints[2]->globalPosition().y() - y0;
  double z2 = spacePoints[2]->globalPosition().z() - z0; 

  // distance of second SP to first in transverse plane
  // Also happens to be u-coordinate of second SP in conformal mapping
  double u1 = 1./std::sqrt(x1*x1+y1*y1);
  // denominator for conformal mapping
  double rn = x2*x2+y2*y2;
  double r2 = 1./rn;
  // coordinate system for conformal mapping - this is local x
  double a  = x1*u1;
  double b  = y1*u1;
  // u/v-coordinate of third SP in conformal mapping
  double u2 = (a*x2+b*y2)*r2;
  double v2 = (a*y2-b*x2)*r2;
  // A,B are slope and intercept of the straight line in the u,v plane
  // connecting the three points.
  double A  = v2/(u2-u1);
  double B  = 2.*(v2-A*u2);
  double C  = B/std::sqrt(1.+A*A);  // curvature estimate. (2R)²=(1+A²)/b² => 1/2R = b/sqrt(1+A²) = B / sqrt(1+A²).
  double T;  // estimate of the track dz/dr (1/tanTheta)
  std::abs(C) > 1.e-6 ? T = (z2*C)/std::asin(C*std::sqrt(rn)) : T = z2/std::sqrt(rn);

  const Amg::Transform3D& Tp = pla->transform();

  double Ax[3] = {Tp(0,0),Tp(1,0),Tp(2,0)}; 
  double Ay[3] = {Tp(0,1),Tp(1,1),Tp(2,1)}; 
  double D [3] = {Tp(0,3),Tp(1,3),Tp(2,3)}; 
  
  double   d[3] = {x0-D[0],y0-D[1],z0-D[2]};

  track_paras[0] = d[0]*Ax[0]+d[1]*Ax[1]+d[2]*Ax[2];
  track_paras[1] = d[0]*Ay[0]+d[1]*Ay[1]+d[2]*Ay[2];

  // use constant magnetic field to estimate theta and phi
  double magnetic_field = 0.002; // kT
  track_paras[2] = std::atan2(y2,x2);
  track_paras[3] = std::atan2(1.,T) ;
  track_paras[5] = -C / (0.3 * magnetic_field); // inverse momentum in GeV^-1

  track_paras[4] = track_paras[5]/std::sqrt(1.+T*T);  // qoverp from qoverpt and theta
  track_paras[6] = x0;
  track_paras[7] = y0;
  track_paras[8] = z0;

  ATH_MSG_DEBUG(
      "linearConformalMapping: \n" << \
      "\nlocal x = " << track_paras[0] << \
      "\nlocal y = " << track_paras[1] << \
      "\nphi     = " << track_paras[2] << \
      "\ntheta   = " << track_paras[3] << \
      "\nqoverp  = " << track_paras[4]);

  bool any_nan = false;
  for (int i = 0; i < 5; i++) {
    if (std::isnan(track_paras[i])) {
      any_nan = true;
      break;
    }
  }
  if (any_nan){
      ATH_MSG_WARNING("Seed parameters contain NaN elements - skipping this track ");
      return nullptr; 
  }

  std::unique_ptr<const Trk::TrackParameters> trkParameters(
    pla->createUniqueTrackParameters(track_paras[0],track_paras[1],track_paras[2],track_paras[3],track_paras[4],std::nullopt));
  
  if (!trkParameters) {
    ATH_MSG_WARNING("Failed to create track parameters");
    return nullptr;
  }

  return trkParameters;
}

StatusCode InDet::SeedFitterTool::initialize() {
  return StatusCode::SUCCESS;
}

StatusCode InDet::SeedFitterTool::finalize() {
  StatusCode sc = AlgTool::finalize();
  return sc;
}

MsgStream&  InDet::SeedFitterTool::dump( MsgStream& out ) const
{
  out<<std::endl;
  return dumpevent(out);
}

std::ostream& InDet::SeedFitterTool::dump( std::ostream& out ) const
{
  return out;
}

MsgStream& InDet::SeedFitterTool::dumpevent( MsgStream& out ) const
{
  out<<"|Nothing to dump-|"
       <<std::endl;
  return out;
}
