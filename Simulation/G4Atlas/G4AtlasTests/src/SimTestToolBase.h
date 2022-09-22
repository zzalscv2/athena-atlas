/* -*- C++ -*- */

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASTESTS_SIMTESTTOOLBASE_H
#define G4ATLASTESTS_SIMTESTTOOLBASE_H
/** @file SimTestToolBase.h
 * @author John Chapman - ATLAS Collaboration
 */
#include "AthenaBaseComps/AthAlgTool.h"
#include "G4AtlasTests/ISimTestTool.h"

#include "SimTestHisto.h"

#include "AtlasHepMC/GenParticle_fwd.h"


class SimTestToolBase : public SimTestHisto, public extends<AthAlgTool, ISimTestTool> {
public:
 /// \name structors and AlgTool implementation
 //@{
  SimTestToolBase(const std::string& type, const std::string& name, const IInterface* parent);
  virtual StatusCode initialize() {
    return StatusCode::SUCCESS;
  }
  //@}

 protected:
  HepMC::ConstGenParticlePtr   getPrimary();

  /// The MC truth key
  std::string m_key;
};

#endif //G4ATLASTESTS_SIMTESTTOOLBASE_H
