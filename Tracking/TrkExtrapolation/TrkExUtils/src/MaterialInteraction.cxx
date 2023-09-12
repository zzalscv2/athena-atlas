/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MaterialInteraction.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
#include "TrkExUtils/MaterialInteraction.h"

#include <cmath>

/**
 * This code follows the notation from the PDG
 * https://pdg.lbl.gov/2022/reviews/rpp2022-rev-passage-particles-matter.pdf
 * equations will refer to this
 *
 *
 * - The mean ionization energy loss <-dE/dX> follow the bethe formula  Eq 34.5
 *
 * - The Most Propable ionization energy loss \Delta_p follows the
 * Landau-Vavilov-Bichsel Eq 34.12
 *
 * - In tracking we might want to employ so called "Landau" treatment.
 *   Espacially  for Muon tracking inside the calorimeter.
 *   In this case the in-out parameters  sigma and kazL
 *   need some explanations
 *
 * - Landau distribution :
 *   The normalized deviation from the most probable energy loss
 *   is \lamda = \Delta - Delta_mp / \xi
 *   \xi is what referred as kazL in the code below
 *   The FWHM of the landau is 4 * \xi or 4 * kazL
 *   We additonally can define a sigmaL ~ FWHM /2.355 ~ 0.424 * FWHM
 *   so sigmaL = 0.424 * 4 * kazL
 *
 * - The ATLAS tracking (since circa 2013) also employs
 *   an approximation to the Landau see as an example
 *   https://indico.cern.ch/event/261427/contributions/580495/attachments/461615/639740/ElossTracking.pdf
 *   This leads to these relations :
 *   sigma = (MPV - mean)/3.59524
 *   sigmaL =  sigma - kazL*log(pathlength)
 */

namespace {

// mean excitation energy --> I
inline double MeanExcitationEnergy(const Trk::Material& mat) {
  // 16 eV * Z**0.9 - bring to MeV
  return 16.e-6 * std::pow(mat.averageZ(), 0.9);
}

inline double DensityEffect(const double zOverAtimesRho, const double eta2,
                            const double gamma, const double I) {

  // density effect. Done for gamma > 10  ( p > 1GeV for muons)
  // see ATLASRECTS-3144 and ATLASRECTS-7586
  // for possible issues
  if (gamma > 10.) {
    // PDG 2022 Table 34.1
    double eplasma = 28.816e-6 * std::sqrt(1000. * zOverAtimesRho);
    // PDG 2022 Eq. 34.6
    return 2. * std::log(eplasma / I) + std::log(eta2) - 1.;
  }
  return 0;
}

inline double KAZ(const double zOverAtimesRho) {
  // K/A*Z = 0.5 * 30.7075MeV/(g/mm2) * Z/A * rho[g/mm3]
  return 0.5 * 30.7075 * zOverAtimesRho;
}
}  // namespace

/** dE/dl ionization energy loss per path unit
It returns de/dl in MeV/mm
For the meaning of sigma and kazL
look at comments on the  ATLAS Landau
approximation.
*/
double Trk::MaterialInteraction::dEdl_ionization(
    double p, const Trk::Material& mat, Trk::ParticleHypothesis particle,
    double& sigma, double& kazL) {

  sigma = 0.;
  kazL = 0.;
  if (mat.averageZ() < 1) {
    return 0.;
  }
  const double me = Trk::ParticleMasses::mass[Trk::electron];
  const double m = Trk::ParticleMasses::mass[particle];
  const double mfrac = me / m;
  const double E = std::sqrt(p * p + m * m);
  const double beta = p / E;
  const double gamma = E / m;
  const double I = MeanExcitationEnergy(mat);
  const double zOverAtimesRho = mat.zOverAtimesRho();
  double kaz = KAZ(zOverAtimesRho);
  //  sigmaL of Landau
  sigma = 4 * kaz * beta / beta;  // dsigma/dl
  //
  double Ionization = 0.;
  if (particle == Trk::electron) {
    // for electrons use slightly different BetheBloch adaption
    // see Stampfer, et al, "Track Fitting With Energy Loss", Comp. Pyhs. Comm.
    // 79 (1994), 157-164
    Ionization = -kaz * (2. * log(2. * me / I) + 3. * log(gamma) - 1.95);
  } else {
    double eta2 = beta * gamma;
    eta2 *= eta2;
    const double delta = DensityEffect(zOverAtimesRho, eta2, gamma, I);
    // tmax - cut off energy
    const double tMax =
        2. * eta2 * me / (1. + 2. * gamma * mfrac + mfrac * mfrac);
    // divide by beta^2 for non-electrons
    kaz /= beta * beta;

    // PDG 2022 Eq 34.5
    Ionization = -kaz * (std::log(2. * me * eta2 * tMax / (I * I)) -
                         2. * (beta * beta) - delta);
    // The MPV is path lenght dependent, here we set pathlenght = 1mm
    constexpr double path = 1.;
    // PDG 2022 Eq 34.12
    const double MOP =
        -kaz * (std::log(2. * me * eta2 / I) + std::log(path * kaz / I) + 0.2 -
                (beta * beta) - delta);
    constexpr double factor = (1. / 3.59524);
    sigma = -(Ionization - MOP) * factor;
    kazL = kaz * factor;
  }
  return Ionization;
}

/** Most Propable dE ionization energly loss
 * It returns dE in MeV.
 * For the meaning of sigma, kazL
 * look at comments on std Landau distribution
 */
double Trk::MaterialInteraction::dE_MPV_ionization(
    double p, const Trk::Material& mat, Trk::ParticleHypothesis particle,
    double& sigma, double& kazL, double path) {
  const double m = Trk::ParticleMasses::mass[particle];
  const double E = std::sqrt(p * p + m * m);
  const double me = Trk::ParticleMasses::mass[Trk::electron];
  const double beta = p / E;
  const double gamma = E / m;
  const double I = MeanExcitationEnergy(mat);
  const double zOverAtimesRho = mat.zOverAtimesRho();
  double kaz = KAZ(zOverAtimesRho);
  double eta2 = beta * gamma;
  eta2 *= eta2;
  const double delta = DensityEffect(zOverAtimesRho, eta2, gamma, I);
  // divide by beta^2 for non-electrons
  kaz /= beta * beta;
  kazL = kaz * path;
  // PDG 2022 Eq 34.12
  const double MPV = -kazL * (std::log(2. * me * eta2 / I) +
                              std::log(kazL / I) + 0.2 - (beta * beta) - delta);
  sigma = 0.424 * 4. * kazL;  // 0.424 FWHM to sigma

  return MPV;
}

/** dE/dl radiation energy loss per path unit */
double Trk::MaterialInteraction::dEdl_radiation(
    double p, const Trk::Material* mat, Trk::ParticleHypothesis particle,
    double& sigma) {
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
                   9.899e-18 * E * E * E;  // E below 1 TeV
      sigma += (0.1828 - 3.966e-3 * std::sqrt(E) + 2.151e-5 * E);  // idem
    } else {
      Radiation += 2.986 - 9.253e-5 * E;           // E above 1 TeV
      sigma += 17.73 + 2.409e-5 * (E - 1000000.);  // idem
    }
  }

  sigma = sigma / mat->x0();

  return Radiation / mat->x0();
}

/** multiple scattering as function of dInX0 */
double Trk::MaterialInteraction::sigmaMS(double dInX0, double p, double beta) {

  if (dInX0 == 0. || p == 0. || beta == 0.) {
    return 0.;
  }

  // Highland formula - projected sigma_s
  // PDG 2022 34.16
  return 13.6 * std::sqrt(dInX0) / (beta * p) *
         (1. + 0.038 * std::log(dInX0 / (beta * beta)));
}

