/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "NewMergeMcEventCollTool.h"
#include <fstream>

NewMergeMcEventCollTool::NewMergeMcEventCollTool(const std::string& type,
                                                 const std::string& name,
                                                 const IInterface *parent) :
  PileUpToolBase(type, name, parent)
{
}

StatusCode NewMergeMcEventCollTool::initialize()
{
  ATH_CHECK(m_pMergeSvc.retrieve());

  ATH_CHECK( m_truthCollOutputKey.initialize() );
  return StatusCode::SUCCESS;
}

/// PileUpTools Approach
StatusCode NewMergeMcEventCollTool::prepareEvent(const EventContext& ctx, unsigned int nInputEvents)
{
  ATH_MSG_VERBOSE( this->name()<<"::prepareEvent()" );

  //Check we are getting at least one event
  if (0 == nInputEvents) {
    ATH_MSG_ERROR("prepareEvent: TimedTruthList with key "
                  << m_truthCollInputKey.value() << " is empty");
    return StatusCode::RECOVERABLE;
  }
  ATH_MSG_VERBOSE(  this->name()<<"::prepareEvent: there are " << nInputEvents << " subevents in this event.");

  if (!m_outputMcEventCollection.isValid()) {
    // Would be nice to avoid having the WriteHandle as a member
    // variable, but this is the only way to allow multiple function
    // calls to add information to the version of the
    // McEventCollection in the output StoreGate
    m_outputMcEventCollection = SG::makeHandle(m_truthCollOutputKey, ctx);
    ATH_CHECK(m_outputMcEventCollection.record(std::make_unique<McEventCollection>()));
  }
  else {
    ATH_MSG_ERROR("WriteHandle already valid??");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode NewMergeMcEventCollTool::processBunchXing(int /*bunchXing*/,
                                                     SubEventIterator bSubEvents,
                                                     SubEventIterator eSubEvents)
{
  SubEventIterator iEvt(bSubEvents);
  //loop over the McEventCollections (each one assumed to containing exactly one GenEvent) of the various input events
  while (iEvt != eSubEvents){
    StoreGateSvc& seStore(*iEvt->ptr()->evtStore());
    const McEventCollection *pMEC(nullptr);
    ATH_CHECK(seStore.retrieve(pMEC, m_truthCollInputKey.value()));
    ATH_MSG_VERBOSE( this->name()<<"::processBunchXing: SubEvt McEventCollection from StoreGate " << seStore.name() << " of PileUpType " << iEvt->type() );
    if(m_pileUpType==iEvt->type()) {
      ATH_CHECK(this->processEvent(pMEC, m_outputMcEventCollection.ptr(),
                                   static_cast<int>(iEvt->type()), iEvt->time()));
    }
    ++iEvt;
  }
  return StatusCode::SUCCESS;
}

StatusCode NewMergeMcEventCollTool::mergeEvent(const EventContext& /*ctx*/)
{
  ATH_MSG_VERBOSE(  this->name()<<"::mergeEvent()" );
  if(msgLvl(MSG::VERBOSE)) { this->printDetailsOfMergedMcEventCollection(m_outputMcEventCollection.ptr()); }
  return StatusCode::SUCCESS;
}

/// Algorithm Approach
StatusCode NewMergeMcEventCollTool::processAllSubEvents(const EventContext& ctx)
{
  ATH_MSG_VERBOSE ( this->name()<<"::processAllSubEvents()" );
  SG::WriteHandle<McEventCollection> outputMcEventCollection(m_truthCollOutputKey, ctx);
  ATH_CHECK(outputMcEventCollection.record(std::make_unique<McEventCollection>()));

  //first get the list of McEventCollections
  typedef PileUpMergeSvc::TimedList<McEventCollection>::type TimedTruthList;
  TimedTruthList truthList;
  ATH_CHECK(m_pMergeSvc->retrieveSubEvtsData(m_truthCollInputKey.value(), truthList));

  //Check we are getting at least one event
  const unsigned int nInputMcEventColls=truthList.size();
  if (0 == nInputMcEventColls) {
    ATH_MSG_WARNING("TimedTruthList with key " << m_truthCollInputKey.value() << " is empty.");
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG( "execute: there are " << nInputMcEventColls << " subevents in this event.");
  //TODO can we make this into an auto for loop?
  TimedTruthList::iterator timedTruthListIter(truthList.begin()), endOfTimedTruthList(truthList.end());
  //loop over the McEventCollections (each one assumed to containing exactly one GenEvent) of the various input events
  while (timedTruthListIter != endOfTimedTruthList) {
    ATH_MSG_VERBOSE( this->name()<<"::processBunchXing: SubEvt McEventCollection of PileUpType " << timedTruthListIter->first.type() );
    if(m_pileUpType==timedTruthListIter->first.type()) {
      const McEventCollection *pBackgroundMcEvtColl(&*(timedTruthListIter->second));
      ATH_CHECK(this->processEvent(pBackgroundMcEvtColl, outputMcEventCollection.ptr(), static_cast<int>(timedTruthListIter->first.type()), timedTruthListIter->first.time()));
    }
    ++timedTruthListIter;
  } //timed colls

  if(msgLvl(MSG::VERBOSE)) { this->printDetailsOfMergedMcEventCollection(outputMcEventCollection.ptr()); }
  return StatusCode::SUCCESS;
}

StatusCode NewMergeMcEventCollTool::processEvent(const McEventCollection *pMcEvtColl, McEventCollection *outputMcEventCollection, int pileupType, long timeOffset)
{
  ATH_MSG_VERBOSE(  this->name()<<"::processEvent() Event Type: " << pileupType );
  if (!outputMcEventCollection) {
    ATH_MSG_ERROR( this->name()<<"::processEvent() was passed an null output McEventCollection pointer." );
    return StatusCode::FAILURE;
  }
  if (!pMcEvtColl) {
    ATH_MSG_ERROR( this->name()<<"::processEvent() was passed an null input McEventCollection pointer." );
    return StatusCode::FAILURE;
  }
  if (!pMcEvtColl->empty()) {
    for (unsigned int iEv=0; iEv<pMcEvtColl->size(); iEv++) {
      const HepMC::GenEvent& c_evt(*((*pMcEvtColl)[iEv]));
#ifdef HEPMC3
      HepMC::GenEvent * evt = new HepMC::GenEvent(c_evt);
      HepMC::fillBarcodesAttribute(evt);
      const int bunchCrossingTime=static_cast<int>(timeOffset);
      evt->add_attribute("BunchCrossingTime",std::make_shared<HepMC3::IntAttribute>(bunchCrossingTime));
      evt->add_attribute("PileUpType",std::make_shared<HepMC3::IntAttribute>(pileupType));
      for (const auto&  itVer:  evt->vertices()) {
        HepMC::FourVector newPos(itVer->position().x(),itVer->position().y(),itVer->position().z(),itVer->position().t()+timeOffset);
        itVer->set_position(newPos);
      }
      outputMcEventCollection->push_back(evt);
#else
      HepMC::GenEvent * evt = new HepMC::GenEvent(c_evt);
      for (HepMC::GenEvent::vertex_iterator itVer=evt->vertices_begin(); itVer!=evt->vertices_end(); ++itVer) {
        HepMC::FourVector newPos((*itVer)->position().x(),(*itVer)->position().y(),(*itVer)->position().z(),(*itVer)->position().t()+timeOffset);
        (*itVer)->set_position(newPos);
      }
      outputMcEventCollection->push_back(evt);
#endif
    }
  }
  return StatusCode::SUCCESS;
}

void NewMergeMcEventCollTool::printDetailsOfMergedMcEventCollection(McEventCollection* outputMcEventCollection) const
{
  DataVector<HepMC::GenEvent>::const_iterator outputEventItr(outputMcEventCollection->begin());
  const DataVector<HepMC::GenEvent>::const_iterator endOfEvents(outputMcEventCollection->end());
  ATH_MSG_INFO ( "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" );
  ATH_MSG_INFO ( "pileUpType: " << m_pileUpType);
  ATH_MSG_INFO ( "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" );
  ATH_MSG_INFO ( "Current OUTPUT GenEvent: " );
  while(outputEventItr!=endOfEvents) {
    const int signal_process_id(HepMC::signal_process_id((*outputEventItr)));
    const int event_number((*outputEventItr)->event_number());
#ifdef HEPMC3
    ATH_MSG_INFO ( "GenEvent #"<<event_number<<", signal_process_id="<<signal_process_id<</*", category="<<event->second<<*/", number of Vertices="<<(*outputEventItr)->vertices().size() );
#else
    ATH_MSG_INFO ( "GenEvent #"<<event_number<<", signal_process_id="<<signal_process_id<</*", category="<<event->second<<*/", number of Vertices="<<(*outputEventItr)->vertices_size() );
#endif
    char fname[80];
    sprintf(fname,"%s.event%d.txt",m_truthCollInputKey.value().c_str(),event_number);
    std::ofstream of(fname);
    const HepMC::GenEvent *evt=(*outputEventItr);
    HepMC::Print::line(of,*evt); // verbose output
    of.close();
    ++outputEventItr;
  }
  ATH_MSG_INFO ( "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" );
  return;
}
