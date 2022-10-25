/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// DiTauProngDecorator.h
// Author: Nadav  Tamir (nadavtamir@mail.tau.ac.il)
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_DITAUPRONGDECORATOR_H
#define DERIVATIONFRAMEWORK_DITAUPRONGDECORATOR_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODTau/DiTauJetContainer.h"

//Tool to decorate boosted ditaus with subjet prongness

namespace DerivationFramework {

  class DiTauProngDecorator : public AthAlgTool, public IAugmentationTool {
    public: 
      DiTauProngDecorator(const std::string& t, const std::string& n, const IInterface* p);

      virtual StatusCode addBranches() const;
      std::vector<int> getNtracks(const xAOD::DiTauJet* xDiTau, int iSubjet) const;           

    private:

      std::string m_ditauContainerName;

  }; 
}

#endif // DERIVATIONFRAMEWORK_DITAUPRONGDECORATOR_H
