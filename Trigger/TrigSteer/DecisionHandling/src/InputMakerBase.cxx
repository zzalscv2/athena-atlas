#include "DecisionHandling/InputMakerBase.h"

const SG::ReadHandleKeyArray<TrigCompositeUtils::DecisionContainer>& InputMakerBase::decisionInputs() const{
  return m_inputs;
}

const SG::WriteHandleKeyArray<TrigCompositeUtils::DecisionContainer>& InputMakerBase::decisionOutputs() const{
  return m_outputs;
}

StatusCode InputMakerBase::initialize() {
  CHECK( m_inputs.initialize() );
  renounceArray(m_inputs); // make inputs implicit, not required by scheduler

  ATH_MSG_DEBUG("Will consume implicit decisions:" );
  for (auto& input: m_inputs){  
    ATH_MSG_DEBUG(" "<<input.key());
  }
  ATH_MSG_DEBUG(" and produce decisions: " << m_outputs );
  for (auto& output: m_outputs){  
    ATH_MSG_DEBUG(" "<<output.key());
  }
  // initialise sub class
  CHECK ( subInitialize() );
}
