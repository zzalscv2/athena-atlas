/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRUTHUTILS_HEPMCHELPERS_H
#define TRUTHUTILS_HEPMCHELPERS_H
#include <vector>
#include <cmath>
#include <algorithm>
#include <array>
#include <cstdlib>
#include "TruthUtils/MagicNumbers.h"

/// @file
///
/// Provides the HepMC tools from the external MCUtils header package,
/// ATLAS-specific HepMC functions not suitable for MCUtils.

namespace MC
{
#include "AtlasPID.h"

  /// @brief Identify if the particle with given PDG ID would not interact with the detector, i.e. not a neutrino or WIMP
  template <class T> inline bool isNonInteracting(const T& p) { return !(isStrongInteracting<T>(p) || isEMInteracting<T>(p) || isGeantino<T>(p)); }

  /// @brief Identify if the particle with given PDG ID would produce ID tracks but not shower in the detector if stable
  template <class T> inline  bool isChargedNonShowering(const T& p) { return (isMuon<T>(p) || isSUSY<T>(p)); }

  template <class T> inline bool isDecayed(const T& p)  { return p->status() == 2;}
  template <class T> inline bool isStable(const T& p)   { return p->status() == 1;}
  template <class T> inline bool isFinalState(const T& p)   { return p->status() == 1 && !p->end_vertex();}
  template <class T> inline bool isPhysical(const T& p) { return isStable<T>(p) || isDecayed<T>(p); }
  template <class T> inline bool isPhysicalHadron(const T& p) { return isHadron<T>(p) && isPhysical<T>(p);}

  /// @brief Determine if the particle is stable at the generator (not det-sim) level,
  template <class T> inline bool isGenStable(const T& p) { return isStable<T>(p) && !HepMC::is_simulation_particle<T>(p);}

  /// @brief Identify if the particle is considered stable at the post-detector-sim stage
  template <class T> inline bool isSimStable(const T& p) { return  isStable<T>(p) &&  !p->end_vertex() && HepMC::is_simulation_particle<T>(p);}

  /// @brief Identify if the particle could interact with the detector during the simulation, e.g. not a neutrino or WIMP
  template <class T> inline bool isSimInteracting(const T& p) { return isGenStable<T>(p) && !isNonInteracting<T>(p);}

/* The functions below should be unified */
  template <class T> inline bool FastCaloSimIsGenSimulStable(const T&  p) {
    const int status = p->status();
    const auto vertex = p->end_vertex();
    return ((status == 1) ||
            (status == 2 && (!vertex || HepMC::is_simulation_vertex(vertex))));
  }

  template <class T> inline bool isZeroEnergyPhoton(const T&  p) { return isPhoton(p->pdg_id()) && p->e() == 0;}

  template <class T> inline bool jettruthparticleselectortool_isStable( const T& p) {
    if (HepMC::is_simulation_particle(p)) return false; // This particle is from G4
    if (p->pdg_id() == 21 && p->p4().E() == 0) return false; //< Workaround for a gen bug?
    const int status = p->status();
    const auto vertex = p->end_vertex();
    return ((status == 1) || //< Fully stable, even if marked that way by G4
            (status == 2 && vertex && HepMC::is_simulation_vertex(vertex))); //< Gen-stable with G4 decay
  }

  template <class T> inline bool jettruthparticleselectortool_isInteracting(const T& p) {
      if (! jettruthparticleselectortool_isStable(p)) return false;
      const int apid = std::abs(p->pdg_id() );
      const int status = p->status();
      if (apid == 12 || apid == 14 || apid == 16) return false; //< neutrinos
      if (status == 1 && (apid == 1000022 || apid == 1000024 || apid == 5100022)) return false;
      if (status == 1 && (apid == 39 || apid == 1000039 || apid == 5000039)) return false;
      return true;      
    }

  template <class T> inline bool DerivationFramework_isNonInteracting(const T& p) {
    const int apid = std::abs(p->pdg_id());
    if (apid == 12 || apid == 14 || apid == 16) return true; //< neutrinos
    if (apid == 1000022 || apid == 1000024 || apid == 5100022) return true; // SUSY & KK photon and Z partners
    if (apid == 39 || apid == 1000039 || apid == 5000039) return true; //< gravitons: standard, SUSY and KK
    if (apid == 9000001 || apid == 9000002 || apid == 9000003 || apid == 9000004 || apid == 9000005 || apid == 9000006) return true; //< exotic particles from monotop model
    return false;
  }

  template <class T> inline bool egammaTruthAlg_isGenStable_and_isGenInteracting (const T& p) {
    if (HepMC::is_simulation_particle(p)) return false;
    if (p->pdg_id() == 21 && p->e() == 0) return false;
    const int status = p->status();
    const int apid = std::abs(p->pdg_id());
    const auto vertex = p->end_vertex();
    if (apid == 12 || apid == 14 || apid == 16) return false; //< neutrinos
    if (status == 1 && (apid == 1000022 || apid == 1000024 || apid == 5100022)) return false;
    if (status == 1 && (apid == 39 || apid == 1000039 || apid == 5000039)) return false;
    return ((status == 1) ||
            (status == 2 && (!vertex || HepMC::is_simulation_vertex(vertex))));
  }

  template <class T> inline bool ThinGeantTruthAlg_isStatus1BSMParticle(const T& p)  {
    const int apid = std::abs(p->pdg_id());
    bool status1 = (p->status() == 1);
    return ((31 < apid && apid < 38) || // BSM Higgs / W' / Z' / etc
      apid == 39 || apid == 41 || apid == 42 ||
      apid == 7 || // 4th gen beauty
      apid == 8 || // 4th gen top
      (600 < apid && apid < 607) || // scalar leptoquarks
      (1000000 < apid && apid < 2000000) || // left-handed SUSY (including R-Hadrons)
      (2000000 < apid && apid < 3000000) || // right-handed SUSY (including R-Hadrons)
      apid == 6000005 ||  // X5/3
      apid == 6000006 ||  // T2/3
      apid == 6000007 ||  // B-1/3
      apid == 6000008 ||  // Y-4/3
      ((apid >= 10000100) && (apid <= 10001000)) // multi-charged
      ) && status1;
  }

  template <class T> inline bool MenuTruthThinning_isBSM(const T& p) {
    const int apid = std::abs(p->pdg_id());
    return ((31 < apid && apid < 38) || // BSM Higgs / W' / Z' / etc
      apid == 39 || apid == 41 || apid == 42 ||
      apid == 7 || // 4th gen beauty
      apid == 8 || // 4th gen top
      (600 < apid && apid < 607) || // scalar leptoquarks
      (1000000 < apid && apid < 1000040) || // left-handed SUSY
      (2000000 < apid && apid < 2000040) || // right-handed SUSY
      apid == 6000005 || // X5/3
      apid == 6000006 || // T2/3
      apid == 6000007 || // B-1/3
      apid == 6000008 || // Y-4/3
      ((apid >= 10000100) && (apid <= 10001000) ) // multi-charged
      );
  }

}
#endif
