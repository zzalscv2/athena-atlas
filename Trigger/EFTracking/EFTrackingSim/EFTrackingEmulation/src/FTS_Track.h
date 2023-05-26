// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#ifndef EFTRACKINGEMULATION_FTSTRACK_H
#define EFTRACKINGEMULATION_FTSTRACK_H

// Easy represetnation of a smeared track, allows fast manipulation

#include <iostream>
#include "TMath.h"

namespace EFTrackingSmearing{
class FTS_Track {

 public:
 FTS_Track():  m_pt(0), m_eta(0), m_phi(0), m_d0(0), m_z0(0),
    m_sigma_pt(0), m_sigma_eta(0), m_sigma_phi(0), m_sigma_d0(0), m_sigma_z0(0) {};

 FTS_Track(double pt, double eta, double phi, double d0, double z0):
  m_pt(pt), m_eta(eta), m_phi(phi), m_d0(d0), m_z0(z0) {};

 FTS_Track(double pt, double eta, double phi, double d0, double z0,double sigma_pt, double sigma_eta, double sigma_phi, double sigma_d0, double sigma_z0,int FakeFlags):
  m_pt(pt), m_eta(eta), m_phi(phi), m_d0(d0), m_z0(z0),
    m_sigma_pt(sigma_pt), m_sigma_eta(sigma_eta), m_sigma_phi(sigma_phi), m_sigma_d0(sigma_d0), m_sigma_z0(sigma_z0), m_FakeFlags(FakeFlags) {};

  double pt() const {return m_pt;};
  double curv() const {return 1.0/m_pt;}; // curv pT
  double eta() const {return m_eta;};
  double theta() const {return 2.0*TMath::ATan(TMath::Exp(-m_eta));};
  double phi() const {return m_phi;};
  double d0() const {return m_d0;};
  double z0() const {return m_z0;};

  double sigma_pt() const {return  m_sigma_pt;};
  double sigma_curv() const {return  m_sigma_pt/m_pt/m_pt;};
  double sigma_eta() const {return m_sigma_eta;};
  double sigma_theta() const {return m_sigma_eta/TMath::CosH(m_eta); };
  double sigma_phi() const {return m_sigma_phi;};
  double sigma_d0() const {return  m_sigma_d0;};
  double sigma_z0() const {return  m_sigma_z0;};
  int FakeFlags() const {return m_FakeFlags;};

 protected:
  double m_pt;
  double m_eta;
  double m_phi;
  double m_d0;
  double m_z0;

  double m_sigma_pt;
  double m_sigma_eta;
  double m_sigma_phi;
  double m_sigma_d0;
  double m_sigma_z0;
  int m_FakeFlags;
};
}

inline std::ostream& operator<<(std::ostream& s, const EFTrackingSmearing::FTS_Track& t){
  s << "trackCandidate:  pt="       << t.pt() << " eta="<< t.eta() <<" phi="<< t.phi() 
    << " d0="<<t.d0() << " z0="<< t.z0()
    << "\n";
  return s;
}


#endif
