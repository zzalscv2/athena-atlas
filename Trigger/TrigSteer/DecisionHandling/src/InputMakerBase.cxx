/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include "DecisionHandling/InputMakerBase.h"
#include "DecisionHandling/HLTIdentifier.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"



const std::vector<std::string> InputMakerBase::m_baseLinks = {"initialRoI", "roi", "view", "feature"};

InputMakerBase::InputMakerBase( const std::string& name, ISvcLocator* pSvcLocator )
  : ::AthReentrantAlgorithm( name, pSvcLocator ) {}

InputMakerBase::~InputMakerBase() {}

const SG::ReadHandleKeyArray<DecisionContainer>& InputMakerBase::decisionInputs() const{
  return m_inputs;
}

const SG::WriteHandleKeyArray<DecisionContainer>& InputMakerBase::decisionOutputs() const{
  return m_outputs;
}

StatusCode InputMakerBase::sysInitialize() {
  CHECK( AthReentrantAlgorithm::sysInitialize() ); // initialise base class
  CHECK( m_inputs.initialize() );
  renounceArray(m_inputs); // make inputs implicit, i.e. not required by scheduler
  ATH_MSG_DEBUG("Will consume implicit decisions:" );
  for (auto& input: m_inputs){  
    ATH_MSG_DEBUG( " "<<input.key() );
  }
  CHECK( m_outputs.initialize() );
  ATH_MSG_DEBUG(" and produce decisions: ");
  for (auto& output: m_outputs){  
    ATH_MSG_DEBUG( " "<<output.key() );
  }
  return StatusCode::SUCCESS;
}


StatusCode InputMakerBase::decisionInputToOutput(const EventContext& context, std::vector< SG::WriteHandle<DecisionContainer> > & outputHandles) const{

  outputHandles = decisionOutputs().makeHandles(context);

  // check inputs
  size_t validInput=0;
  for ( auto inputKey: decisionInputs() ) {
    auto inputHandle = SG::makeHandle( inputKey, context );
    ATH_MSG_DEBUG(" "<<inputKey.key()<<(inputHandle.isValid()? " is valid": " is not valid" ) );
    if (inputHandle.isValid()) validInput++;
  }
  ATH_MSG_DEBUG( "number of implicit ReadHandles is " << decisionInputs().size() <<", "<< validInput<<" are valid" );
  
  // Input is array of input decision containers
  // Loop over them. For each input decision container, create an output decision container
  // For each input Decision in the input container, create and output Decision in the corresponding output container and link them.
  size_t outputIndex = 0;
  for ( auto inputKey: decisionInputs() ) {
    auto inputHandle = SG::makeHandle( inputKey, context );
    if( not inputHandle.isValid() ) {
      ATH_MSG_DEBUG( "Got no decisions from input "<< inputKey.key() << " because handle not valid");
      outputIndex++;
      continue;
    }
    if( inputHandle->size() == 0){ // input filtered out
      ATH_MSG_WARNING( "Got no decisions from input "<< inputKey.key()<<": handle is valid but container is empty. Is this expected?");
      outputIndex++;
      continue;
    }
    ATH_MSG_DEBUG( "Got input "<< inputKey.key()<<" with " << inputHandle->size() << " elements" );
    // create the output container
    auto outDecisions = std::make_unique<DecisionContainer>();
    auto outDecAux = std::make_unique<DecisionAuxContainer>();
    outDecisions->setStore( outDecAux.get() );
       
    // loop over decisions retrieved from this input
    size_t input_counter =0;
    for ( auto decision : *inputHandle){
      // create new decision for each input	
      Decision*  newDec = newDecisionIn( outDecisions.get() );
      linkToPrevious( newDec, inputKey.key(), input_counter );
      insertDecisionIDs( decision, newDec );

      copyBaseLinks( decision, newDec);
      ATH_MSG_DEBUG("New decision has "<< newDec->hasObjectLink(m_roisLink.value() ) <<" "<< m_roisLink.value());      
      input_counter++;	
    } // loop over decisions

    ATH_MSG_DEBUG( "Recording output key " <<  decisionOutputs()[ outputIndex ].key() <<" of size "<<outDecisions->size()  <<" at index "<< outputIndex);
    CHECK( outputHandles[outputIndex].record( std::move( outDecisions ), std::move( outDecAux ) ) );
    outputIndex++;	       
  } // end of first loop over input keys

  return StatusCode::SUCCESS;
}


StatusCode InputMakerBase::copyBaseLinks(const Decision* src, Decision* dest) const  {
  for (auto link: m_baseLinks){
    if ( src->hasObjectLink(link ) ) dest->copyLinkFrom(src,link);
  }

  // do we need to filter the links to be copied? if not, we can copy all of them
  //    copyLinks(decision, newDec);
  if ( src->hasObjectLink("self" ) ) dest->copyLinkFrom(src,"self","seed"); // make use of self-link 


  return StatusCode::SUCCESS;
}


StatusCode InputMakerBase::debugPrintOut(const EventContext& context, const std::vector< SG::WriteHandle<DecisionContainer> >& outputHandles) const{
  size_t validInput=0;
  for ( auto inputKey: decisionInputs() ) {
    auto inputHandle = SG::makeHandle( inputKey, context );
    ATH_MSG_DEBUG(" " << inputKey.key() << " " << (inputHandle.isValid()? "valid": "not valid" ) );
    if (inputHandle.isValid()) validInput++;
  }
  size_t validOutput=0;
  ATH_MSG_DEBUG( "number of implicit ReadHandles for input decisions is " << decisionInputs().size() << ", " << validInput << " are valid" );
  for ( auto outHandle: outputHandles ) {
    if( not outHandle.isValid() ) continue;
    validOutput++;
  }
  ATH_MSG_DEBUG("Produced " << validOutput << " output decisions containers");
  if(validInput != validOutput) {
    ATH_MSG_ERROR("Found " << validInput << " inputs and " << validOutput << " outputs");
  }
  
  for ( auto outHandle: outputHandles ) {
    if( not outHandle.isValid() ) continue;
    ATH_MSG_DEBUG(outHandle.key() <<" with "<< outHandle->size() <<" decisions:");
    for (auto outdecision:  *outHandle){
      DecisionIDContainer objDecisions;      
      decisionIDs( outdecision, objDecisions );    
      ATH_MSG_DEBUG("Number of positive decisions for this output: " << objDecisions.size() );
      for ( DecisionID id : objDecisions ) {
        ATH_MSG_DEBUG( " ---  decision " << HLT::Identifier( id ) );
      }  
    }
  }
  return StatusCode::SUCCESS;
}



// StatusCode InputMakerBase::insertDecisions( const Decision* src, Decision* dest ) const  {

//   DecisionIDContainer ids;
//   decisionIDs( dest, ids );
//   decisionIDs( src, ids );
//   decisionIDs( dest ).clear(); 
//   decisionIDs(dest).insert( decisionIDs(dest).end(), ids.begin(), ids.end() );
//   return StatusCode::SUCCESS;
// }


size_t InputMakerBase::countInputHandles( const EventContext& context ) const {
  size_t validInputCount=0;
  for ( auto inputKey: decisionInputs() ) {
    auto inputHandle = SG::makeHandle( inputKey, context );
    if (inputHandle.isValid()) validInputCount++;
  }
  return validInputCount;
}
