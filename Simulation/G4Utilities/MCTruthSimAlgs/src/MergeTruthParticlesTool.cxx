/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MergeTruthParticlesTool.h"

#include "AthenaKernel/errorcheck.h"
#include "xAODTruth/TruthParticleAuxContainer.h"

#include "GaudiKernel/SystemOfUnits.h"

MergeTruthParticlesTool::MergeTruthParticlesTool(const std::string& type,
                                                 const std::string& name,
                                                 const IInterface* parent)
  : PileUpToolBase(type, name, parent)
{
}

StatusCode MergeTruthParticlesTool::initialize()
{
  ATH_MSG_DEBUG ( "Initializing " << name());
  ATH_CHECK(m_pMergeSvc.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode MergeTruthParticlesTool::prepareEvent(const EventContext& /*ctx*/, unsigned int nInputEvents)
{
  ATH_MSG_VERBOSE ( "prepareEvent()" );
  ATH_MSG_DEBUG ( "prepareEvent: there are " << nInputEvents << " subevents in this event." );
  m_first_event = true;
  m_inTimeOutputTruthParticleContainer = new xAOD::TruthParticleContainer();
  m_inTimeOutputTruthParticleContainer->setStore(new xAOD::TruthParticleAuxContainer);
  return StatusCode::SUCCESS;
}

StatusCode MergeTruthParticlesTool::processBunchXing(int bunchXing,
                                                     SubEventIterator bSubEvents,
                                                     SubEventIterator eSubEvents)
{
  ATH_MSG_VERBOSE ( "processBunchXing()" );
  SubEventIterator iEvt(bSubEvents);
  while (iEvt != eSubEvents) {
    const xAOD::TruthParticleContainer* inputTruthParticleContainer{};
    if (m_pMergeSvc->retrieveSingleSubEvtData(m_inputTruthParticleCollKey.value(), inputTruthParticleContainer,
                                              bunchXing, iEvt).isSuccess()) {
      ATH_MSG_VERBOSE("Found an xAOD::TruthParticleContainer in storeGate.");
      if ( !inputTruthParticleContainer ) {
        ATH_MSG_ERROR("Unable to retrieve input xAOD::TruthParticleContainer: " << m_inputTruthParticleCollKey);
        return StatusCode::FAILURE;
      }
      ATH_MSG_DEBUG ( "processBunchXing: bunch Crossing = " << bunchXing << " xAOD::TruthParticleContainer size = " << inputTruthParticleContainer->size());
      const int eventNumber{iEvt->index()};
      // const float timeOfBCID{static_cast<float>(iEvt->time())};
      if ( !m_first_event || m_includeSignalTruthParticles ) {
        ATH_CHECK(this->processTruthParticleContainer(&(*inputTruthParticleContainer), m_inTimeOutputTruthParticleContainer, eventNumber));
      }
    }
    else {
      ATH_MSG_DEBUG ( "processBunchXing: No TruthParticleContainer found." );
    }
    m_first_event=false;
    ++iEvt;
  }
  //signal is always the first event, so even if we didn't see
  //anything should set this to false here.
  if(m_first_event) {m_first_event=false;}
  return StatusCode::SUCCESS;
}

StatusCode MergeTruthParticlesTool::mergeEvent(const EventContext& /*ctx*/)
{
  ATH_MSG_VERBOSE ( "mergeEvent" );

  if(this->record(m_inTimeOutputTruthParticleContainer, m_inTimeOutputTruthParticleCollKey).isFailure()) { // This call also records the xAOD::TruthParticleAuxContainer.
    ATH_MSG_ERROR("mergeEvent: Failed to record InTimeOutputTruthParticleContainer");
    return StatusCode::FAILURE;
  }
  else {
    ATH_MSG_DEBUG ( "mergeEvent: Recorded " << m_inTimeOutputTruthParticleCollKey << " TruthParticleContainer with "
                    << m_inTimeOutputTruthParticleContainer->size() <<" entries." );
  }
  return StatusCode::SUCCESS;
}

StatusCode MergeTruthParticlesTool::record(const xAOD::TruthParticleContainer* pTruthParticles, const std::string& truthParticleContainerName) const
{
  const xAOD::TruthParticleAuxContainer* pTruthParticlesAux =
    dynamic_cast<const xAOD::TruthParticleAuxContainer*>(pTruthParticles->getConstStore());
  ATH_MSG_DEBUG("Check Aux store: " << pTruthParticles << " ... " << &pTruthParticles->auxbase() << " ... " << pTruthParticlesAux );
  if ( !pTruthParticlesAux ) {
    ATH_MSG_ERROR("Unable to retrieve xAOD::TruthParticleAuxContainer");
    return StatusCode::FAILURE;
  }
  ATH_MSG_VERBOSE("Recording new xAOD::TruthParticleAuxContainer.");
  if ( evtStore()->record(pTruthParticlesAux, truthParticleContainerName+"Aux.").isFailure() ) {
    ATH_MSG_ERROR("Unable to write new xAOD::TruthParticleAuxContainer to event store: " << truthParticleContainerName);
    return StatusCode::FAILURE;
  }
  ATH_MSG_VERBOSE("Recording new xAOD::TruthParticleContainer.");
  if ( evtStore()->record(pTruthParticles, truthParticleContainerName).isFailure() ) {
    ATH_MSG_ERROR("Unable to write new xAOD::TruthParticleContainer to event store: " << truthParticleContainerName);
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Created new xAOD::TruthParticleContainer in event store: " << truthParticleContainerName);
  return StatusCode::SUCCESS;
}


StatusCode MergeTruthParticlesTool::processAllSubEvents(const EventContext& /*ctx*/)
{
  ATH_MSG_VERBOSE ( "processAllSubEvents()" );

  m_first_event = true;
  m_inTimeOutputTruthParticleContainer = new xAOD::TruthParticleContainer();
  m_inTimeOutputTruthParticleContainer->setStore(new xAOD::TruthParticleAuxContainer);

  typedef PileUpMergeSvc::TimedList<xAOD::TruthParticleContainer>::type TruthParticleList;
  TruthParticleList truthList;
  if ( (m_pMergeSvc->retrieveSubEvtsData(m_inputTruthParticleCollKey.value(), truthList)).isSuccess() ) {
    if (!truthList.empty()) {
      //now merge all collections into one
      TruthParticleList::const_iterator truthParticleColl_iter(truthList.begin());
      const TruthParticleList::const_iterator endOfTruthParticleColls(truthList.end());
      while (truthParticleColl_iter!=endOfTruthParticleColls) {
        const int eventNumber{static_cast<int>((truthParticleColl_iter)->first.index())}; // casting from long unsigned int to int
        //const float timeOfBCID(static_cast<float>((truthParticleColl_iter)->first.time()));
        if( static_cast<int>((truthParticleColl_iter)->first.time())!=0 || !m_first_event || m_includeSignalTruthParticles ) {
          ATH_CHECK(this->processTruthParticleContainer(&(*((truthParticleColl_iter)->second)), m_inTimeOutputTruthParticleContainer, eventNumber));
        }
        //signal is always the first event, so if the first event
        //wasn't in-time, then the signal collection was missing and
        //we should skip further checks.
        if(m_first_event) {m_first_event=false;}
        ++truthParticleColl_iter;
      }
    }
    else {
      ATH_MSG_DEBUG ( "processAllSubEvents: TruthParticleList is empty" );
    }
  }
  else {
    ATH_MSG_ERROR ( "processAllSubEvents: Can not find TruthParticleList" );
  }

  if( this->record(m_inTimeOutputTruthParticleContainer, m_inTimeOutputTruthParticleCollKey).isFailure() ) { // This call also records the xAOD::TruthParticleAuxContainer.
    ATH_MSG_ERROR ( "processAllSubEvents: Failed to record InTimeOutputTruthParticleContainer" );
    return StatusCode::FAILURE;
  }
  else {
    ATH_MSG_DEBUG ( "processAllSubEvents: Recorded " << m_inTimeOutputTruthParticleCollKey << " xAOD::TruthParticleContainer with "
                    << m_inTimeOutputTruthParticleContainer->size() <<" entries." );
  }

  return StatusCode::SUCCESS;
}

//use a float for timeOfBCID as TruthParticle moments are stored as floats.
StatusCode MergeTruthParticlesTool::processTruthParticleContainer(const xAOD::TruthParticleContainer* inputTruthParticleContainer, xAOD::TruthParticleContainer *outputTruthParticleContainer, int eventNumber)
{
  if (!inputTruthParticleContainer || !outputTruthParticleContainer) { return StatusCode::FAILURE; }

  // Set up decorators
  const static SG::AuxElement::Accessor< int > eventNumberAccessor("pileupEventNumber");

  for (const xAOD::TruthParticle *theParticle : *inputTruthParticleContainer) {
    xAOD::TruthParticle* xTruthParticle = new xAOD::TruthParticle();
    outputTruthParticleContainer->push_back(xTruthParticle);
    *xTruthParticle = *theParticle; // deep-copy
    // add the pile-up event number
    eventNumberAccessor(*xTruthParticle) = eventNumber;
  }
  return StatusCode::SUCCESS;
}
