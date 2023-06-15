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
/// @file
///
/// Provides the HepMC tools from the external MCUtils header package,
/// ATLAS-specific HepMC functions not suitable for MCUtils.

namespace MCUtils
{
namespace PID
{
#include "AtlasPID.h"
}
}

namespace MC
{
using namespace MCUtils;
using namespace MCUtils::PID;

  /// @brief Identify if the particle with given PDG ID would not interact with the detector, i.e. not a neutrino or WIMP
  inline bool isNonInteracting(int pid) { return !(PID::isStrongInteracting(pid) || PID::isEMInteracting(pid)); }

  /// @brief Identify if the particle with given PDG ID would produce ID tracks but not shower in the detector if stable
  inline bool isChargedNonShowering(int pid) { return (PID::isMuon(pid) || PID::isSUSY(pid)); }
}

#if !defined(XAOD_STANDALONE)
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/Relatives.h"
#include "AtlasHepMC/MagicNumbers.h"
namespace MC {

  using namespace MCUtils::PID;

  template <class T> inline bool isDecayed(const T& p)  { return p->status() == 2;}
  template <class T> inline bool isStable(const T& p)   { return p->status() == 1;}
  template <class T> inline bool isPhysical(const T& p) { return isStable<T>(p) || isDecayed<T>(p); }
  template <class T> inline bool isPhysicalHadron(const T& p) { return PID::isHadron(p->pdg_id()) && isPhysical<T>(p);}

  /// @brief Determine if the particle is stable at the generator (not det-sim) level,
  template <class T> inline bool isGenStable(const T& p) {
    /// Retrieving the barcode is relatively expensive with HepMC3, so test status first.
    if (p->status() != 1) return false;
    return !HepMC::is_simulation_particle<T>(p);
  }

  /// @brief Identify if the particle is considered stable at the post-detector-sim stage
  template <class T> inline bool isSimStable(const T& p) {
    if (p->status() != 1) return false;
    if (isGenStable<T>(p)) return p->end_vertex() == nullptr;
    return true;
  }

  /// @brief Identify if the particle could interact with the detector during the simulation, e.g. not a neutrino or WIMP
  template <class T> inline bool isSimInteracting(const T& p) {
    if (!MC::isGenStable<T>(p)) return false;
    return !MC::isNonInteracting(p->pdg_id());
  }


  template <class T> inline bool FastCaloSimIsGenSimulStable(const T&  p) {
  int status=p->status();
  const auto& vertex = p->end_vertex();
  return (status%1000 == 1) ||
          (status%1000 == 2 && status > 1000) ||
          (status==2 && !vertex) ||
          (status==2 && HepMC::is_simulation_vertex(vertex))
          ;
}


}
#endif
#endif
