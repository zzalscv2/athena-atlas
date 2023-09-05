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
  template <class T> inline bool isInteracting(const T& p) { return isStrongInteracting<T>(p) || isEMInteracting<T>(p) || isGeantino<T>(p); }

  /// @brief Identify if the particle with given PDG ID would produce ID tracks but not shower in the detector if stable
  template <class T> inline  bool isChargedNonShowering(const T& p) { return (isMuon<T>(p) || isSUSY<T>(p)); }

  template <class T> inline bool isDecayed(const T& p)  { return p->status()%HepMC::SIM_STATUS_THRESHOLD == 2;}
  template <class T> inline bool isStable(const T& p)   { return p->status()%HepMC::SIM_STATUS_THRESHOLD == 1;}
  template <class T> inline bool isFinalState(const T& p)   { return p->status()%HepMC::SIM_STATUS_THRESHOLD == 1 && !p->end_vertex();}
  template <class T> inline bool isPhysical(const T& p) { return isStable<T>(p) || isDecayed<T>(p); }
  template <class T> inline bool isPhysicalHadron(const T& p) { return isHadron<T>(p) && isPhysical<T>(p);}

  /// @brief Determine if the particle is stable at the generator (not det-sim) level,
  template <class T> inline bool isGenStable(const T& p) { return isStable<T>(p) && !HepMC::is_simulation_particle<T>(p);}

  /// @brief Identify if the particle is considered stable at the post-detector-sim stage
  template <class T> inline bool isSimStable(const T& p) { return  isStable<T>(p) &&  !p->end_vertex() && HepMC::is_simulation_particle<T>(p);}

  /// @brief Identify if the particle could interact with the detector during the simulation, e.g. not a neutrino or WIMP
  template <class T> inline bool isSimInteracting(const T& p) { return isGenStable<T>(p) && isInteracting<T>(p);}

  /// @brief Identify if particle is satble or decayed in simulation. + a pathological case of decayed particle w/o end vertex.
  template <class T> inline bool isStableOrSimDecayed(const T& p) {
    const auto vertex = p->end_vertex();
    return ( isStable<T>(p) || (isDecayed<T>(p) && (!vertex || HepMC::is_simulation_vertex(vertex))));
  }

  /// @brief Identify a photon with zero energy. Probably a workaround for a generator bug.
  template <class T> inline bool isZeroEnergyPhoton(const T&  p) { return isPhoton<T>(p) && p->e() == 0;}

/* The functions below should be unified */
  template <class T> inline bool jettruthparticleselectortool_isStable( const T& p) {
    if (HepMC::is_simulation_particle(p)) return false; // This particle is from G4
    if (isZeroEnergyPhoton<T>(p)) return false;
    const auto vertex = p->end_vertex();
    return ( isStable<T>(p) || //< Fully stable, even if marked that way by G4
            ( isDecayed<T>(p) && vertex && HepMC::is_simulation_vertex(vertex))); //< Gen-stable with G4 decay
  }

  template <class T> inline bool jettruthparticleselectortool_isInteracting(const T& p) {
      if (! jettruthparticleselectortool_isStable(p)) return false;
      const int apid = std::abs(p->pdg_id() );
      if ( isStable<T>(p) && (apid == 12 || apid == 14 || apid == 16)) return false; //< neutrinos
      if ( isStable<T>(p) && (apid == 1000022 || apid == 1000024 || apid == 5100022)) return false;
      if ( isStable<T>(p) && (apid == 39 || apid == 1000039 || apid == 5000039)) return false;
      if ( isStable<T>(p) && (apid == 9000001 || apid == 9000002 || apid == 9000003 || apid == 9000004 || apid == 9000005 || apid == 9000006)) return false; //< exotic particles from monotop model
      return true;      
    }

  template <class T> inline bool DerivationFramework_isInteracting(const T& p) {
    const int apid = std::abs(p->pdg_id());
    if (apid == 12 || apid == 14 || apid == 16) return false; //< neutrinos
    if (apid == 1000022 || apid == 1000024 || apid == 5100022) return false; // SUSY & KK photon and Z partners
    if (apid == 39 || apid == 1000039 || apid == 5000039) return false; //< gravitons: standard, SUSY and KK
    if (apid == 9000001 || apid == 9000002 || apid == 9000003 || apid == 9000004 || apid == 9000005 || apid == 9000006) return false; //< exotic particles from monotop model
    return true;
  }

  template <class T> inline bool egammaTruthAlg_isGenStable_and_isGenInteracting (const T& p) {
    if (HepMC::is_simulation_particle(p)) return false;
    if (isZeroEnergyPhoton<T>(p)) return false;
    const int apid = std::abs(p->pdg_id());
    if ( isStable<T>(p) && (apid == 12 || apid == 14 || apid == 16)) return false; //< neutrinos
    if ( isStable<T>(p) && (apid == 1000022 || apid == 1000024 || apid == 5100022)) return false;
    if ( isStable<T>(p) && (apid == 39 || apid == 1000039 || apid == 5000039)) return false;
    if ( isStable<T>(p) && (apid == 9000001 || apid == 9000002 || apid == 9000003 || apid == 9000004 || apid == 9000005 || apid == 9000006)) return false; //< exotic particles from monotop model
    return isStableOrSimDecayed<T>(p);
  }

  template <class T> inline bool ThinGeantTruthAlg_isStatus1BSMParticle(const T& p)  {
    const int apid = std::abs(p->pdg_id());
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
      ) && isStable<T>(p);
  }

  template <class T> inline bool MenuTruthThinning_isBSM(const T& p) {
    const int apid = std::abs(p->pdg_id());
    return (isBSM(apid) ||
      /// Legacy Athena BSM. Not clear if those are still in use.
      (600 < apid && apid < 607) || // scalar leptoquarks
      apid == 6000005 || // X5/3
      apid == 6000006 || // T2/3
      apid == 6000007 || // B-1/3
      apid == 6000008 || // Y-4/3
      ((apid >= 10000100) && (apid <= 10001000) ) // multi-charged
      );
  }

}
#endif
