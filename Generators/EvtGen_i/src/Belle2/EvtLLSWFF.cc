/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Florian Bernlochner                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "EvtGenBase/EvtReport.hh"
#include "EvtGenBase/EvtId.hh"
#include <string>
#include "EvtGenBase/EvtPDL.hh"
#include <math.h>
#include <stdlib.h>

#include "EvtGen_i/EvtGenExternal/Belle2//EvtLLSWFF.h"


EvtLLSWFF::EvtLLSWFF(double _tau_w1, double _tau_wp, double _zeta_1)
{

  m_tau_w1 = _tau_w1;
  m_tau_wp = _tau_wp;

  m_zeta_1 = _zeta_1;

  m_mb = 4.2; m_mc = 1.4; m_L = 0.40; m_Lp = 0.80; m_Ls = 0.76;

  m_eta_1 = m_eta_2 = m_eta_3 = m_eta_b = 0.;
  m_chi_1 = m_chi_2 = m_chi_b = 0.;

  return;
}

EvtLLSWFF::EvtLLSWFF(double _tau_w1, double _tau_wp, double _tau_1, double _tau_2)
{

  m_tau_w1 = _tau_w1;
  m_tau_wp = _tau_wp;

  m_tau_1 = _tau_1;
  m_tau_2 = _tau_2;

  m_mb = 4.2; m_mc = 1.4; m_L = 0.40; m_Lp = 0.80; m_Ls = 0.76;

  m_eta_1 = m_eta_2 = m_eta_3 = m_eta_b = 0.;
  m_chi_1 = m_chi_2 = m_chi_b = 0.;

  return;
}


void EvtLLSWFF::getscalarff(EvtId parent, EvtId,
                            double t, double mass, double* fp, double* f0)
{

  // std::cerr << "Called EvtLLSWFF::getscalarff" << std::endl;


  double m = EvtPDL::getMeanMass(parent);
  double w = ((m * m) + (mass * mass) - t) / (2.0 * m * mass);

  *fp = ((m + mass) * gpD0(w) - (m - mass) * gmD0(w)) / (2 * sqrt(mass * m));
  *f0 = ((m - mass) * (pow(m + mass, 2.) - t) * gpD0(w) - (mass + m) * (pow(m - mass,
         2.) - t) * gmD0(w)) / (2 * (m - mass) * sqrt(m * mass) * (mass + m));

  return;
}

void EvtLLSWFF::getvectorff(EvtId parent, EvtId daughter,
                            double t, double mass, double* a1f,
                            double* a2f, double* vf, double* a0f)
{

  double m = EvtPDL::getMeanMass(parent);
  double w = ((m * m) + (mass * mass) - t) / (2.0 * m * mass);

  static const EvtId D3P1P = EvtPDL::getId("D'_1+");
  static const EvtId D3P1N = EvtPDL::getId("D'_1-");
  static const EvtId D3P10 = EvtPDL::getId("D'_10");
  static const EvtId D3P1B = EvtPDL::getId("anti-D'_10");
  static const EvtId D3P1SP = EvtPDL::getId("D'_s1+");
  static const EvtId D3P1SN = EvtPDL::getId("D'_s1-");

  // Form factors have a general form, with parameters passed in
  // from the arguements.

  if (daughter == D3P1P || daughter == D3P1N || daughter == D3P10 || daughter == D3P1B || daughter == D3P1SP || daughter == D3P1SN) {

    *a1f = sqrt(mass * m) / (mass + m) * gV1D1p(w);
    *a2f = -(mass + m) * (gV3D1p(w) + mass / m * gV2D1p(w)) / (2 * sqrt(mass * m));
    *vf = 0.5 * (mass + m) * 1. / sqrt(mass * m) * gAD1p(w);

    double a3f = (m + mass) / (2 * mass) * (*a1f) - (m - mass) / (2 * mass) * (*a2f);
    *a0f = -t * (gV3D1p(w) - mass / m * gV2D1p(w)) / (4.*mass * sqrt(m * mass)) + a3f;


  } else {

    *a1f = sqrt(mass * m) / (mass + m) * fV1D1(w);
    *a2f = -(mass + m) * (fV3D1(w) + mass / m * fV2D1(w)) / (2 * sqrt(mass * m));
    *vf = 0.5 * (mass + m) * 1. / sqrt(mass * m) * fAD1(w);

    double a3f = (m + mass) / (2 * mass) * (*a1f) - (m - mass) / (2 * mass) * (*a2f);
    *a0f = -t * (fV3D1(w) - mass / m * fV2D1(w)) / (4.*mass * sqrt(m * mass)) + a3f;

  }


  return;
}

void EvtLLSWFF::gettensorff(EvtId parent, EvtId,
                            double t, double mass, double* hf,
                            double* kf, double* bpf, double* bmf)
{

  double m = EvtPDL::getMeanMass(parent);
  double w = ((m * m) + (mass * mass) - t) / (2.0 * m * mass);

  *hf = -sqrt(mass * m) / (mass * m * m) / 2.*kVD2(w);
  *kf = sqrt(mass * m) / m * kA1D2(w);

  *bpf = sqrt(mass * m) * (kA3D2(w) * m + kA2D2(w) * mass) / (2 * pow(m, 3.) * mass);
  *bmf = kA2D2(w) * sqrt(mass * m) / (2 * pow(m, 3.)) - kA3D2(w) * sqrt(mass * m) / (2 * pow(m, 2.) * mass);

  return;

}


void EvtLLSWFF::getbaryonff(EvtId, EvtId, double, double, double*,
                            double*, double*, double*)
{

  EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Not implemented :getbaryonff in EvtLLSWFF.\n";
  ::abort();

}

void EvtLLSWFF::getdiracff(EvtId, EvtId, double, double, double*, double*,
                           double*, double*, double*, double*)
{

  EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Not implemented :getdiracff in EvtLLSWFF.\n";
  ::abort();

}

void EvtLLSWFF::getraritaff(EvtId, EvtId, double, double, double*, double*,
                            double*, double*, double*, double*, double*, double*)
{

  EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Not implemented :getraritaff in EvtLLSWFF.\n";
  ::abort();

}

// Isgur-Wise Function
//-------------------------------------------------------------------------------------------

double EvtLLSWFF::IsgurWiseFunction(double w)
{

  double value = 0;

  value += m_tau_w1;
  value += m_tau_w1 * (w - 1.) * m_tau_wp;

  return value;

}

// Form Factors for D0
//-------------------------------------------------------------------------------------------

double EvtLLSWFF::gpD0(double w)
{

  double ec = 1. / (2.*m_mc), eb = 1. / (2.*m_mb);

  // Leading IW function values
  double z12 = IsgurWiseFunction(w);

  double gp = 0;

  gp += ec * (2.*(w - 1.) * m_zeta_1 * z12 - 3.*z12 * (w * m_Ls - m_L) / (w + 1.));
  gp += -eb * ((m_Ls * (2.*w + 1.) - m_L * (w + 2.)) / (w + 1.) * z12 - 2.*(w - 1.) * m_zeta_1 * z12);

  return gp;

}

double EvtLLSWFF::gmD0(double w)
{

  double ec = 1. / (2.*m_mc), eb = 1. / (2.*m_mb);

  // Leading IW function values
  double z12 = IsgurWiseFunction(w);

  double gm = 0;

  gm += z12;
  gm += ec * (6 * m_chi_1 - 2 * (w + 1) * m_chi_2);
  gm += eb * m_chi_b;

  return gm;
}

// Form Factors for D1*
//-------------------------------------------------------------------------------------------


double EvtLLSWFF::gV1D1p(double w)
{

  double ec = 1 / (2 * m_mc), eb = 1 / (2 * m_mb);

  // Leading IW function values
  double z12 = IsgurWiseFunction(w);

  double gv1 = 0;

  gv1 += (w - 1.) * z12;
  gv1 += ec * ((w * m_Ls - m_L) * z12 + (w - 1.) * (-2 * m_chi_1));
  gv1 -= eb * ((m_Ls * (2.*w + 1) - m_L * (w + 2.)) * z12 - 2 * (pow(w, 2.) - 1.) * m_zeta_1 * z12 - (w - 1.) * m_chi_b);

  return gv1;

}

double EvtLLSWFF::gV2D1p(double w)
{

  double ec = 1 / (2 * m_mc);

  // Leading IW function values
  double z12 = IsgurWiseFunction(w);

  double gv2 = 0;

  gv2 += 2 * ec * (m_zeta_1 * z12 - m_chi_2);

  return gv2;

}

double EvtLLSWFF::gV3D1p(double w)
{

  double ec = 1 / (2 * m_mc), eb = 1 / (2 * m_mb);

  // Leading IW function values
  double z12 = IsgurWiseFunction(w);

  double gv3 = 0;

  gv3 += eb * ((m_Ls * (2.*w + 1) - m_L * (w + 2.)) / (w + 1.) * z12 - 2 * (w - 1.) * m_zeta_1 * z12 - m_chi_b);
  gv3 -= z12;
  gv3 -= ec * ((w * m_Ls - m_L) / (w + 1) * z12 + 2.*m_zeta_1 * z12 - 2 * m_chi_1 + 2 * m_chi_2);

  return gv3;


}

double EvtLLSWFF::gAD1p(double w)
{

  double ec = 1 / (2 * m_mc), eb = 1 / (2 * m_mb);

  // Leading IW function values
  double z12 = IsgurWiseFunction(w);

  double ga = 0;

  ga += eb * (2.*(w - 1.) * m_zeta_1 * z12 - (m_Ls * (2.*w + 1.) - m_L * (w + 2.)) / (w + 1.) * z12 + m_chi_b);
  ga += z12;
  ga += ec * ((w * m_Ls - m_L) / (w + 1.) * z12 - 2 * m_chi_1);

  return ga;

}

// Form Factors for D1
//-------------------------------------------------------------------------------------------

double EvtLLSWFF::fV1D1(double w)
{

  double ec = 1. / (2.*m_mc), eb = 1. / (2.*m_mb);

  // Leading IW function values
  double t32 = IsgurWiseFunction(w);

  double fv1 = 0;

  fv1 += (1. - pow(w, 2.)) * t32;
  fv1 -= eb * (pow(w, 2.) - 1.) * ((m_Lp + m_L) * t32 - (2.*w + 1) * m_tau_1 * t32 - m_tau_2 * t32 + m_eta_b);
  fv1 -= ec * (4.*(w + 1.) * (w * m_Lp - m_L) * t32 - (pow(w, 2.) - 1) * (3.*m_tau_1 * t32 - 3.*m_tau_2 * t32 + 2 * m_eta_1 + 3 * m_eta_3));

  fv1 /= sqrt(6);

  return fv1;

}

double EvtLLSWFF::fV2D1(double w)
{

  double ec = 1. / (2.*m_mc), eb = 1. / (2.*m_mb);

  // Leading IW function values
  double t32 = IsgurWiseFunction(w);

  double fv2 = 0;

  fv2 -= 3.*t32;
  fv2 -= 3 * eb * ((m_Lp + m_L) * t32 - (2 * w + 1) * m_tau_1 * t32 - m_tau_2 * t32 + m_eta_b);
  fv2 -= ec * ((4.*w - 1) * m_tau_1 * t32 + 5 * m_tau_2 * t32 + 10 * m_eta_1 + 4 * (w - 1.) * m_eta_2 - 5 * m_eta_3);

  fv2 /= sqrt(6);

  return fv2;

}

double EvtLLSWFF::fV3D1(double w)
{

  double ec = 1. / (2.*m_mc), eb = 1. / (2.*m_mb);

  // Leading IW function values
  double t32 = IsgurWiseFunction(w);

  double fv3 = 0;

  fv3 += (w - 2.) * t32;
  fv3 += eb * ((2. + w) * ((m_Lp + m_L) * t32 - (2.*w + 1.) * m_tau_1 * t32 - m_tau_2 * t32) - (2. - w) * m_eta_b);
  fv3 += ec * (4.*(w * m_Lp - m_L) * t32 + (2. + w) * m_tau_1 * t32 + (2. + 3.*w) * m_tau_2 * t32 - 2.*(6. + w) * m_eta_1 - 4.*
               (w - 1) * m_eta_2 - (3.*w - 2.) * m_eta_3);

  fv3 /= sqrt(6);

  return fv3;


}

double EvtLLSWFF::fAD1(double w)
{

  double ec = 1. / (2.*m_mc), eb = 1. / (2.*m_mb);

  // Leading IW function values
  double t32 = IsgurWiseFunction(w);

  double fa = 0;

  fa += -(w + 1.) * t32;
  fa -= eb * ((w - 1.) * ((m_Lp + m_L) * t32 - (2.*w + 1.) * m_tau_1 * t32 - m_tau_2 * t32) + (w + 1.) * m_eta_b);
  fa -= ec * (4.*(w * m_Lp - m_L) 
* t32 - 3.*(w - 1.) * (m_tau_1 * t32 - m_tau_2 * t32) + (w + 1.) * (-2 * m_eta_1 - 3 * m_eta_3));

  fa /= sqrt(6);

  return fa;

}

// Form Factors for D2
//-------------------------------------------------------------------------------------------

double EvtLLSWFF::kA1D2(double w)
{

  double ec = 1. / (2.*m_mc), eb = 1. / (2.*m_mb);

  // Leading IW function values
  double t32 = IsgurWiseFunction(w);

  double ka1 = 0;

  ka1 -= (1. + w) * t32;
  ka1 -= eb * ((w - 1.) * ((m_Lp + m_L) * t32 - (2.*w + 1.) * m_tau_1 * t32 - m_tau_2 * t32) + (1. + w) * m_eta_b);
  ka1 -= ec * ((w - 1.) * (m_tau_1 * t32 - m_tau_2 * t32) + (w + 1.) * (-2 * m_eta_1 + m_eta_3));

  return ka1;

}

double EvtLLSWFF::kA2D2(double w)
{

  double ec = 1. / (2.*m_mc);

  // Leading IW function values
  double t32 = IsgurWiseFunction(w);

  double ka2 = 0;

  ka2 -=  2.*ec * (m_tau_1 * t32 + m_eta_2);

  return ka2;

}

double EvtLLSWFF::kA3D2(double w)
{

  double ec = 1. / (2.*m_mc), eb = 1. / (2.*m_mb);

  // Leading IW function values
  double t32 = IsgurWiseFunction(w);

  double ka3 = 0;

  ka3 += t32;
  ka3 += eb * ((m_Lp + m_L) * t32 - (2.*w + 1.) * m_tau_1 * t32 - m_tau_2 * t32 + m_eta_b);
  ka3 -= ec * (m_tau_1 * t32 + m_tau_2 * t32 + 2 * m_eta_1 - 2 * m_eta_2 - m_eta_3);

  return ka3;

}

double EvtLLSWFF::kVD2(double w)
{

  double ec = 1. / (2.*m_mc), eb = 1. / (2.*m_mb);

  // Leading IW function values
  double t32 = IsgurWiseFunction(w);

  double kv = 0;

  kv -= t32;
  kv -= eb * ((m_Lp + m_L) * t32 - (2.*w + 1) * m_tau_1 * t32 - m_tau_2 * t32 + m_eta_b);
  kv -= ec * (m_tau_1 * t32 - m_tau_2 * t32 - 2 * m_eta_1 + m_eta_3);

  return kv;

}
