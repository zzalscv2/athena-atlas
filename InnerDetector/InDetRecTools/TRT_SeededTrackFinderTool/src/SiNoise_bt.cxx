/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TRT_SeededTrackFinderTool/SiNoise_bt.h"
#include <cmath>

///////////////////////////////////////////////////////////////////
// Noise production
// Di r  = +1 along momentum , -1 opposite momentum 
// Model = 1 - muon, 2 - electron 
///////////////////////////////////////////////////////////////////

void 
InDet::SiNoise_bt::reset(){
      m_model          = 0 ;
      m_covarianceAzim = 0.;
      m_covariancePola = 0.;
      m_covarianceIMom = 0.;
      m_correctionIMom = 1.;
    }


void InDet::SiNoise_bt::production
(int Dir,int Model,const Trk::TrackParameters& Tp)
{
  reset(); 
  if(Model < 1 || Model > 2) return; 
  m_model = Model;
  double radlength = 0.03;
  double energylose = 0.4;

  const Amg::Transform3D& T  = Tp.associatedSurface().transform();
  const AmgVector(5)&     Vp = Tp.parameters(); 

  double q     = std::abs(Vp[4]);
  double cosp  = std::cos(Vp[3]) ;
  double sinp2 = (1.-cosp)*(1.+cosp)   ;
  if(sinp2==0) sinp2 = 0.000001;
  double s     = 
    std::abs(std::sqrt(sinp2)*(std::cos(Vp[2])*T(0,2)+std::sin(Vp[2]*T(1,2)))+cosp*T(2,2));
  s  < .05 ? s = 20. : s = 1./s; 
  
  m_covariancePola = 134.*s*radlength*q*q;
  m_covarianceAzim = m_covariancePola/sinp2;

  if(m_model==1) {
    double        dp = energylose*q*s;
    m_covarianceIMom = .2*dp*dp*q*q;
    m_correctionIMom = 1.-dp;
  }
  else {
    m_correctionIMom = .5;
    m_covarianceIMom = (m_correctionIMom-1.)*(m_correctionIMom-1.)*q*q;
  }
  if(Dir>0) m_correctionIMom = 1./m_correctionIMom;
}
