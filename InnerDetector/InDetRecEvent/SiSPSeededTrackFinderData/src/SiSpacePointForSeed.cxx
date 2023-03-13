/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include <cmath>



#include "SiSPSeededTrackFinderData/SiSpacePointForSeed.h"

#include "InDetPrepRawData/SiCluster.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "TrkSpacePoint/SpacePoint.h"
#include "TrkSurfaces/Surface.h"

namespace InDet {

  SiSpacePointForSeed::SiSpacePointForSeed
  (const Trk::SpacePoint*const& sp,const float* r) 
  {
    set(sp,r); m_param = 0.;  
  }

  SiSpacePointForSeed::SiSpacePointForSeed
  (const Trk::SpacePoint*const& sp,const float* r,const float* sc) 
  {
    set(sp,r,sc); m_param = 0.; 
  }


  /////////////////////////////////////////////////////////////////////////////////
  // Set
  /////////////////////////////////////////////////////////////////////////////////

  void SiSpacePointForSeed::set
  (const Trk::SpacePoint*const& sp,const float* r)
  {
    spacepoint = sp  ;
    m_x        = r[0];
    m_y        = r[1];
    m_z        = r[2];
    m_r        =std::sqrt(m_x*m_x+m_y*m_y);
    m_q        = 100000.;

    const InDet::SiCluster*           c  = static_cast<const InDet::SiCluster*>(sp->clusterList().first);
    const InDetDD::SiDetectorElement* de = c ->detectorElement();

    if( de->isPixel() ) {
      
      const Amg::MatrixX& v =  c->localCovariance();
      float f22 = float(v(1,1) );
      float wid = float(c->width().z());
      float cov = wid*wid*.08333; if(cov < f22) cov = f22;
      if(de->isBarrel()) {m_covz = 9.*cov; m_covr = .06;}
      else               {m_covr = 9.*cov; m_covz = .06;}
      m_sn = nullptr;
    }
    else                {

      const Amg::MatrixX& v = sp->localCovariance();
      float f22 = float(v(1,1));
      if(de->isBarrel()) {m_covz = 8.*f22; m_covr = .1;} 
      else               {m_covr = 8.*f22; m_covz = .1;} 
      m_sn =  &sp->clusterList().second->detectorElement()->surface();
    }
    m_su = &sp->clusterList().first->detectorElement()->surface();
  } 

  /////////////////////////////////////////////////////////////////////////////////
  // Set with error correction 
  // sc[0] - barrel pixels error correction
  // sc[1] - endcap pixels 
  // sc[2] - barrel sct
  // sc[3] - endcap sct 
  /////////////////////////////////////////////////////////////////////////////////

  void SiSpacePointForSeed::set
  (const Trk::SpacePoint*const& sp,const float* r,const float* sc)
  {
    spacepoint = sp  ;
    m_x        = r[0];
    m_y        = r[1];
    m_z        = r[2];
    m_r        =std::sqrt(m_x*m_x+m_y*m_y);
    m_q        = 100000.;

    const InDet::SiCluster*           c  = static_cast<const InDet::SiCluster*>(sp->clusterList().first);
    const InDetDD::SiDetectorElement* de = c ->detectorElement();

    if( de->isPixel() ) {
      
      const Amg::MatrixX& v =  c->localCovariance();
      float f22 = float(v(1,1));
      float wid = float(c->width().z());
      float cov = wid*wid*.08333; if(cov < f22) cov = f22;
      if(de->isBarrel()) {m_covz = 9.*cov*sc[0]; m_covr = .06;}
      else               {m_covr = 9.*cov*sc[1]; m_covz = .06;}
      m_sn = nullptr;
    }
    else                {

      const Amg::MatrixX& v = sp->localCovariance();
      float f22 = float(v(1,1));
      if(de->isBarrel()) {m_covz = 8.*f22*sc[2]; m_covr = .1;} 
      else               {m_covr = 8.*f22*sc[3]; m_covz = .1;} 
      m_sn =  &sp->clusterList().second->detectorElement()->surface();
    }
    m_su = &sp->clusterList().first->detectorElement()->surface();
  }

  void SiSpacePointForSeed::setParam(const float& p)
  {
    m_param = p;
  }

  void SiSpacePointForSeed::setD0(const float& d0)
  {
    m_d0 = d0;
  } 

  void SiSpacePointForSeed::setEta(const float& eta)
  {
    m_eta = eta;
  }

  void  SiSpacePointForSeed::setQuality(float q)
  {
    if(q <= m_q) m_q = q;
  }

  void  SiSpacePointForSeed::setDZDR(const float& dzdr)
  {
    m_dzdr = dzdr;
  }

  void  SiSpacePointForSeed::setPt(const float& pt)
  {
    m_pt = pt;
  }
 
} // end of name space
