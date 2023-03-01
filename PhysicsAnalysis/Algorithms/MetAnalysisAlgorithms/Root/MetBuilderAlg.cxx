/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <MetAnalysisAlgorithms/MetBuilderAlg.h>

#include <METUtilities/METHelpers.h>
#include <xAODMissingET/MissingETAuxContainer.h>

//
// method implementations
//

namespace CP
{
  MetBuilderAlg ::
  MetBuilderAlg (const std::string& name, 
                     ISvcLocator* pSvcLocator)
    : AnaAlgorithm (name, pSvcLocator)
  {
    declareProperty ("finalKey", m_finalKey, "the key for the final met term");
    declareProperty ("softTerm", m_softTerm, "the key for the soft term");
  }



  StatusCode MetBuilderAlg ::
  initialize ()
  {
    ANA_CHECK (m_metHandle.initialize (m_systematicsList));
    ANA_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode MetBuilderAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      xAOD::MissingETContainer *met {};
      ANA_CHECK (m_metHandle.getCopy (met, sys));

      xAOD::MissingET *softTerm = (*met)[m_softTerm];
      if (softTerm == nullptr)
      {
        ANA_MSG_ERROR ("could not find MET soft-term: " << m_softTerm);
        return StatusCode::FAILURE;
      }
      ANA_CHECK (met::buildMETSum (m_finalKey, met, softTerm->source()));

      const static SG::AuxElement::Decorator<float> met_met_dec("met");
      const static SG::AuxElement::Decorator<float> met_phi_dec("phi");
      for (const xAOD::MissingET *metTerm : (*met))
      {
        if (!metTerm)
        {
          ANA_MSG_WARNING("failed to retrieve MET term to decorate met and phi aux vars!");
          continue;
        }
        met_met_dec(*metTerm) = metTerm->met();
        met_phi_dec(*metTerm) = metTerm->phi();
      }
    }

    return StatusCode::SUCCESS;
  }
}
