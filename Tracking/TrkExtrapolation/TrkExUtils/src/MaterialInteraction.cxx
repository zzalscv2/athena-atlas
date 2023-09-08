/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MaterialInteraction.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
#include "TrkExUtils/MaterialInteraction.h"
#include <cmath>


/** dE/dl ionization energy loss per path unit */
double
Trk::MaterialInteraction::dEdl_ionization(double p,
                                          const Trk::Material* mat,
                                          Trk::ParticleHypothesis particle,
                                          double& sigma,
                                          double& kazL)
{

  //
  // calculate mean ionization that is pathlentgh INDEPENDENT
  //
  // sigma = sigmaL = landau sigma is pathlentgh DEPENDENT (here calculated for
  // 1 mm) sigma(length) =  sigma - kazL*log(pathlength) kazL is calculated in
  // this function for later use.
  //
  // For a Landau: MOP value = Mean + 3.59524*sigmaL
  //
  constexpr double factor = (1. / 3.59524); // the compiler will evaulate this

  constexpr double path = 1.; // this is a scaling factor for the landau convolution

  sigma = 0.;
  if (mat->averageZ() < 1)
    return 0.;

  double Ionization = 0.;

  // kinetic variables
  // and the electron mass in MeV
  double me = Trk::ParticleMasses::mass[Trk::electron];
  double m = Trk::ParticleMasses::mass[particle];
  double mfrac = me / m;
  double E = std::sqrt(p * p + m * m);
  double beta = p / E;
  double gamma = E / m;

  // Ionization - Bethe-Bloch
  double I =
    16.e-6 * std::pow(mat->averageZ(), 0.9); // 16 eV * Z**0.9 - bring to MeV

  // K/A*Z = 0.5 * 30.7075MeV/(g/mm2) * Z/A * rho[g/mm3]  / scale to mm by this
  double kaz = 0.5 * 30.7075 * mat->zOverAtimesRho();

  //  sigmaL of Landau
  sigma = 4 * kaz * beta / beta; // dsigma/dl
  kazL = 0.;

  if (particle == Trk::electron) {
    // for electrons use slightly different BetheBloch adaption
    // see Stampfer, et al, "Track Fitting With Energy Loss", Comp. Pyhs. Comm.
    // 79 (1994), 157-164
    Ionization = -kaz * (2. * log(2. * me / I) + 3. * log(gamma) - 1.95);
  } else {

    double eta2 = beta * gamma;
    eta2 *= eta2;
    // density effect, only valid for high energies (gamma > 10 -> p > 1GeV for
    // muons)
    double delta = 0.;
    if (gamma > 10.) {
      double eplasma = 28.816e-6 * std::sqrt(1000. * mat->zOverAtimesRho());
      delta = 2. * std::log(eplasma / I) + std::log(eta2) - 1.;
    }
    // tmax - cut off energy
    double tMax = 2. * eta2 * me / (1. + 2. * gamma * mfrac + mfrac * mfrac);
    // divide by beta^2 for non-electrons
    kaz /= beta * beta;
    Ionization = -kaz * (std::log(2. * me * eta2 * tMax / (I * I)) -
                         2. * (beta * beta) - delta);
    //
    // the landau sigmaL is path length dependent - take what's given
    //
    //    PDG formula 27.11 for MOP value from
    //    http://pdg.lbl.gov/2011/reviews/rpp2011-rev-passage-particles-matter.pdf
    //
    const double MOP = -kaz * (std::log(2. * me * eta2 / I) + std::log(path * kaz / I) + 0.2 -
                  (beta * beta) - delta);
    sigma = -(Ionization - MOP) * factor;
    kazL = kaz * factor;
  }
  // return mean or mop
  return Ionization;
}

/** dE/dl radiation energy loss per path unit */
double
Trk::MaterialInteraction::dEdl_radiation(double p,
                                         const Trk::Material* mat,
                                         Trk::ParticleHypothesis particle,
                                         double& sigma)
{
  sigma = 0.;
  if (mat->x0() == 0.)
    return 0.;

  // preparation of kinetic constants
  const double m = Trk::ParticleMasses::mass[particle];
  const double me = Trk::ParticleMasses::mass[Trk::electron];
  const double mfrac = me / m;
  const double E = sqrt(p * p + m * m);

  // Bremsstrahlung - Bethe-Heitler
  double Radiation = -E * mfrac * mfrac;
  // sigma is rms of steep exponential part of radiation
  sigma = -Radiation;

  // Add e+e- pair production and photonuclear effect for muons at energies
  // above 8 GeV
  //    Radiation gives mean Eloss including the long tail from 'catastrophic'
  //    Eloss sigma the rms of steep exponential
  if ((particle == Trk::muon) && (E > 8000.)) {
    if (E < 1.e6) {
      Radiation += 0.5345 - 6.803e-5 * E - 2.278e-11 * E * E +
                   9.899e-18 * E * E * E;                    // E below 1 TeV
      sigma += (0.1828 - 3.966e-3 * std::sqrt(E) + 2.151e-5 * E); // idem
    } else {
      Radiation += 2.986 - 9.253e-5 * E;          // E above 1 TeV
      sigma += 17.73 + 2.409e-5 * (E - 1000000.); // idem
    }
  }

  sigma = sigma / mat->x0();

  return Radiation / mat->x0();
}

/** multiple scattering as function of dInX0 */
double
Trk::MaterialInteraction::sigmaMS(double dInX0, double p, double beta)
{

  if (dInX0 == 0. || p == 0. || beta == 0.)
    return 0.;

  // Highland formula - projected sigma_s
  double sig_ms =
    13.6 * std::sqrt(dInX0) / (beta * p) * (1. + 0.038 * std::log(dInX0 / (beta * beta)));
  return sig_ms;
}

/** Ionization energy loss from the PDG */
/** PDG formula 32.11 for MOP value from
 * http://http://pdg.lbl.gov/2014/reviews/rpp2014-rev-passage-particles-matter.pdf
 */
double
Trk::MaterialInteraction::PDG_energyLoss_ionization(
  double p,
  const Trk::Material* mat,
  Trk::ParticleHypothesis particle,
  double& sigma,
  double& kazL,
  double path)
{
  // kinetic variables
  // and the electron mass in MeV

  double m = Trk::ParticleMasses::mass[particle];
  double E = std::sqrt(p * p + m * m);
  double me = Trk::ParticleMasses::mass[Trk::electron];
  double beta = p / E;
  double gamma = E / m;

  // Ionization - Bethe-Bloch
  double I =
    16.e-6 * std::pow(mat->averageZ(), 0.9); // 16 eV * Z**0.9 - bring to MeV

  // K/A*Z = 0.5 * 30.7075MeV/(g/mm2) * Z/A * rho[g/mm3]  / scale to mm by this
  double kaz = 0.5 * 30.7075 * mat->zOverAtimesRho();
  double eta2 = beta * gamma;
  eta2 *= eta2;
  // density effect, only valid for high energies (gamma > 10 -> p > 1GeV for
  // muons)
  double delta = 0.;
  if (gamma > 10.) {
    double eplasma = 28.816e-6 * std::sqrt(1000. * mat->zOverAtimesRho());
    delta = 2. * std::log(eplasma / I) + std::log(eta2) - 1.;
  }

  // divide by beta^2 for non-electrons
  kaz /= beta * beta;
  kazL = kaz * path;

  //
  // the landau sigmaL is path length dependent
  //    PDG formula 32.11 for MOP value from
  //    http://http://pdg.lbl.gov/2014/reviews/rpp2014-rev-passage-particles-matter.pdf
  //
  const double MOP = -kazL * (std::log(2. * me * eta2 / I) + std::log(kazL / I) + 0.2 -
                        (beta * beta) - delta);
  sigma = 0.424 * 4. * kazL; // 0.424: scale factor from sigma to FWHM

  return MOP;
}

