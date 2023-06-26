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

namespace MC
{
namespace PID
{
#include "AtlasPID.h"
}
using namespace PID;

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

/* The functions below should be unified */
  template <class T> inline bool FastCaloSimIsGenSimulStable(const T&  p) {
    const int status=p->status();
    const auto vertex = p->end_vertex();
    return (status == 1) ||
          (status==2 && !vertex) ||
          (status==2 && HepMC::is_simulation_vertex(vertex))
          ;
  }

  template <class T> inline bool jettruthparticleselectortool_isStable( const T& p) {
    if (HepMC::is_simulation_particle(p)) return false; // This particle is from G4
    if (p->pdg_id() == 21 && p->p4().E() == 0) return false; //< Workaround for a gen bug?
    const int status=p->status();
    const auto vertex = p->end_vertex();
    return ((status == 1) || //< Fully stable, even if marked that way by G4
            (status == 2 && vertex && HepMC::is_simulation_vertex(vertex))); //< Gen-stable with G4 decay
  }
  

  template <class T> inline bool jettruthparticleselectortool_isInteracting( const T& p) {
      if (! jettruthparticleselectortool_isStable(p)) return false;
      const int apid = std::abs(p->pdg_id() );
      if (apid == 12 || apid == 14 || apid == 16) return false;
      if (p->status() == 1 &&
          (apid == 1000022 || apid == 1000024 || apid == 5100022 ||
           apid == 39 || apid == 1000039 || apid == 5000039)) return false;
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

  template <class T> inline bool egammaTruthAlg_isGenStable (const T& p) {
    const int apid = std::abs(p->pdg_id());
    const auto vertex = p->end_vertex();
    const int status = p->status();
  // we want to keep primary particle with status==2 but without vertex in HepMC
    return (
          ( ( status == 1) ||  
            (status==2 && (!vertex || HepMC::is_simulation_vertex(vertex))) 
            ) && (!HepMC::is_simulation_particle(p)) 
          && !(apid == 21 && p->e()==0)
          );    
  }


  template <class T> inline bool egammaTruthAlg_isGenInteracting (const T& p){
    const int status = p->status();
    const int apid = abs(p->pdg_id());
    const auto vertex = p->end_vertex();
  // we want to keep primary particle with status==2 but without vertex in HepMC
    return
    (
     (((status == 1) ||
       (status==2 && (!vertex || HepMC::is_simulation_vertex(vertex)))) && (!HepMC::is_simulation_particle(p)) ) &&
     !(apid==12 || apid==14 || apid==16 ||
       (apid==1000022 &&  status==1 ) ||
       (apid==5100022 &&  status==1 ) ||
       (apid==1000024 &&  status==1 ) ||
       (apid==39 &&  status==1 ) ||
       (apid==1000039 &&  status==1 ) ||
       (apid==5000039 &&  status==1 ))
     );
  }

}
#endif
#endif
