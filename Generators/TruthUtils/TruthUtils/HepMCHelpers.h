/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRUTHUTILS_HEPMCHELPERS_H
#define TRUTHUTILS_HEPMCHELPERS_H
/// @file
///
/// Provides the HepMC tools from the external MCUtils header package,
/// ATLAS-specific HepMC functions not suitable for MCUtils.

#include "MCUtils/PIDUtils.h"
namespace MC {

  // Use the MCUtils and HEPUtils functions as if they were defined in the ATLAS MC and MC::PID namespaces
  using namespace MCUtils;
  using namespace HEPUtils;


  /// @brief Identify if the particle with given PDG ID would not interact with the detector, i.e. not a neutrino or WIMP
  inline bool isNonInteracting(int pid) {
    return !(PID::isStrongInteracting(pid) || PID::isEMInteracting(pid));
  }

  /// @brief Identify if the particle with given PDG ID would produce ID tracks but not shower in the detector if stable
  inline bool isChargedNonShowering(int pid) {
    if (PID::isMuon(pid)) return true;
    if (PID::isSUSY(pid)) return true; //(meta)stable charginos, R-hadrons etc
     return false;
   }

}
#if !defined(XAOD_STANDALONE)
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/Relatives.h"
#include "AtlasHepMC/MagicNumbers.h"
namespace MC
{
using namespace MCUtils::PID;
template <class T> inline bool isDecayed(const T& p)  { return p->status() == 2;}
template <class T> inline bool isStable(const T& p)   { return p->status() == 1;}
template <class T> inline bool isPhysical(const T& p) { return isStable<T>(p) || isDecayed<T>(p); }
template <class T> inline bool isPhysicalHadron(const T& p) { return PID::isHadron(p->pdg_id()) && isPhysical<T>(p);}
}


namespace MC {

  // Use the MCUtils and HEPUtils functions as if they were defined in the ATLAS MC and MC::PID namespaces
  using namespace MCUtils;
  using namespace HEPUtils;


  /// @name Extra ATLAS-specific particle classifier functions
  //@{

  /// @brief Determine if the particle is stable at the generator (not det-sim) level,
  inline bool isGenStable(const HepMC::ConstGenParticlePtr& p) {
    // Retrieving the barcode is relatively expensive with HepMC3,
    // so test status first.
    if (p->status() != 1) return false;
    return !HepMC::is_simulation_particle(p);
  }


  /// @todo There are many kinds of stable: stable from generator, stable at intermediate stage of det sim transport, or stable after all det sim. Need fns for each?


  /// @brief Identify if the particle is considered stable at the post-detector-sim stage
  inline bool isSimStable(const HepMC::ConstGenParticlePtr& p) {
    if (p->status() != 1) return false;
    if (isGenStable(p)) return p->end_vertex() == nullptr;
    return true;
  }

  /// @brief Identify if the particle would not interact with the detector, i.e. not a neutrino or WIMP
  inline bool isNonInteracting(const HepMC::ConstGenParticlePtr& p) {
    return MC::isNonInteracting(p->pdg_id()); //< From TruthUtils/PIDHelpers.h
  }

  /// @brief Identify if the particle could interact with the detector during the simulation, e.g. not a neutrino or WIMP
  inline bool isSimInteracting(const HepMC::ConstGenParticlePtr& p) {
    if (! MC::isGenStable(p)) return false; //skip particles which the simulation would not see
    return !MC::isNonInteracting(p);
  }


  //@}

}
#endif
#endif
