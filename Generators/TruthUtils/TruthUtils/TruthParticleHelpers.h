/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRUTHUTILS_TRUTHPARTICLEHELPERS_H
#define TRUTHUTILS_TRUTHPARTICLEHELPERS_H
/// @file
///
/// Provides the HepMC tools from the external MCUtils header package,
/// ATLAS-specific HepMC functions not suitable for MCUtils.
#include "AtlasHepMC/MagicNumbers.h"
#include "TruthUtils/PIDHelpers.h"

namespace MC {

  // Use the MCUtils and HEPUtils functions as if they were defined in the ATLAS MC and MC::PID namespaces
  using namespace MCUtils;
  using namespace HEPUtils;


  /// @name Extra ATLAS-specific particle classifier functions
  //@{

  /// @brief Determine if the particle is stable at the generator (not det-sim) level,
  ///
  /// The receipe for this is barcode < 200k and status = 1. Gen-stable particles decayed by
  /// G4 are not set to have status = 2 in ATLAS, but simply have more status = 1 children,
  /// with barcodes > 200k.
  inline bool isGenStable(int status, int barcode) {
    if (status != 1) return false;
    return !HepMC::is_simulation_particle(barcode);
  }

  //@}

}
#endif
