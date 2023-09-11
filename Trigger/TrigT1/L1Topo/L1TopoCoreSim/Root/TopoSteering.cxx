/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "L1TopoInterfaces/AlgFactory.h" 
#include "L1TopoInterfaces/IL1TopoHistSvc.h"


#include "L1TopoInterfaces/ConfigurableAlg.h"
#include "L1TopoInterfaces/ParameterSpace.h"
#include "L1TopoInterfaces/SortingAlg.h"
#include "L1TopoInterfaces/DecisionAlg.h"
#include "L1TopoInterfaces/CountingAlg.h"
#include "L1TopoCommon/Exception.h"
#include "L1TopoEvent/GenericTOB.h"

#include "L1TopoConfig/L1TopoMenu.h"
#include "L1TopoConfig/L1TopoConfigGlobal.h"
#include "L1TopoConfig/LayoutConstraints.h"

#include "L1TopoCoreSim/TopoSteering.h"
#include "L1TopoCoreSim/Connector.h"
#include "L1TopoCoreSim/InputConnector.h"
#include "L1TopoCoreSim/SortingConnector.h"
#include "L1TopoCoreSim/DecisionConnector.h"
#include "L1TopoCoreSim/CountingConnector.h"

// c++ libraries
#include <iomanip>
#include <string>

using namespace std;
using namespace TCS;
using namespace TrigConf;


TopoSteering::TopoSteering() :
   TrigConfMessaging("TopoSteering")
{}

TCS::StatusCode
TopoSteering::setupFromConfiguration ATLAS_NOT_THREAD_SAFE (const TrigConf::L1Menu& l1menu){

  TCS::StatusCode sc = m_structure.setupFromMenu( l1menu, m_isLegacyTopo );
  
  // configure layout of the simulation result
  sc &= m_simulationResult.setupFromMenu( m_structure.outputConnectors(), m_structure.countConnectors() );

  return sc;

}

TCS::StatusCode
TopoSteering::reset() {

   ClusterTOB::clearHeap();
   eEmTOB::clearHeap();
   eTauTOB::clearHeap();
   jEmTOB::clearHeap();
   jTauTOB::clearHeap();
   cTauTOB::clearHeap();
   JetTOB::clearHeap();
   jJetTOB::clearHeap();
   jLJetTOB::clearHeap();
   gJetTOB::clearHeap();
   gLJetTOB::clearHeap();
   MuonTOB::clearHeap();
   LateMuonTOB::clearHeap();
   MuonNextBCTOB::clearHeap();
   MetTOB::clearHeap();
   jXETOB::clearHeap();
   jTETOB::clearHeap();
   gXETOB::clearHeap();
   gTETOB::clearHeap();
   GenericTOB::clearHeap();
   CompositeTOB::clearHeap();

   inputEvent().clear();

   m_structure.reset();

   m_simulationResult.reset();
   
   return TCS::StatusCode::SUCCESS;
}


TCS::StatusCode
TopoSteering::initializeAlgorithms() {
   TRG_MSG_INFO("initializing algorithms");
   if( ! structure().isConfigured() ) {
      TCS_EXCEPTION("L1Topo Steering has not been configured, can't run")
   }

   for(auto conn: m_structure.connectors()) {
      TCS::ConfigurableAlg * alg = conn->algorithm();
      if(alg) {
         TRG_MSG_INFO("initializing algorithm " << alg->name());
         if(m_histSvc) {
            alg->setL1TopoHistSvc(m_histSvc);
         }
	 alg->setLegacyMode(m_isLegacyTopo);
	 alg->setIsolationFW_CTAU(structure().isolationFW_CTAU());
	 alg->setIsolationFW_JTAU(structure().isolationFW_JTAU());
         alg->initialize();
      }

   }

   return TCS::StatusCode::SUCCESS;
}


TCS::StatusCode
TopoSteering::setHistSvc(std::shared_ptr<IL1TopoHistSvc> histSvc) {
   TRG_MSG_INFO("setting L1TopoHistSvc ");
   m_histSvc = histSvc;
   return TCS::StatusCode::SUCCESS;
}


TCS::StatusCode
TopoSteering::saveHist() {
   if(m_histSvc) {
      m_histSvc->save();
   } else {
      TRG_MSG_WARNING("saveHist called without an L1TopoHistSvc being available");
   }
   return TCS::StatusCode::SUCCESS;
}


TCS::StatusCode
TopoSteering::executeEvent() {


   TRG_MSG_INFO("L1 TopoSteering: start executing event " << m_evtCounter << "-----------------------------------");


   if( ! structure().isConfigured() ) {
      TRG_MSG_INFO("L1Topo Steering has not been configured, can't run");
      TCS_EXCEPTION("L1Topo Steering has not been configured, can't run");
   }

   inputEvent().print();

   // execute all connectors
   TCS::StatusCode sc = TCS::StatusCode::SUCCESS;
   TRG_MSG_INFO("Going to execute " << m_structure.outputConnectors().size() << " decision connectors and " << m_structure.countConnectors().size() << " multiplicity connectors.");
   for(auto outConn: m_structure.outputConnectors()) {
      TRG_MSG_INFO("executing trigger line " << outConn.first);
      sc |= executeConnector(outConn.second);
      TRG_MSG_INFO("result of trigger line " << outConn.first << " : " << outConn.second->decision().decision());
   }   

   for(auto multConn: m_structure.countConnectors()) {
      TRG_MSG_INFO("executing trigger line " << multConn.first);
      sc |= executeConnector(multConn.second);
   } 

   sc |= m_simulationResult.collectResult(); 

   m_simulationResult.globalOutput().print();

   TRG_MSG_INFO("finished executing event " << m_evtCounter++);
   return TCS::StatusCode::SUCCESS;
}



TCS::StatusCode
TopoSteering::executeTrigger(const std::string & TrigName) {
   if( ! structure().isConfigured() )
      TCS_EXCEPTION("TopoSteering has not been configured, can't run");
   
   DecisionConnector * outConn = m_structure.outputConnector(TrigName);

   TCS::StatusCode sc = executeConnector(outConn);

   m_simulationResult.collectResult(outConn);

   return sc;
}






TCS::StatusCode
TopoSteering::executeConnector(TCS::Connector *conn) {

   if (conn == NULL) {
     return TCS::StatusCode::FAILURE;
   }

   // caching
   if(conn->isExecuted())
      return conn->executionStatusCode();
  
   TCS::StatusCode sc(TCS::StatusCode::SUCCESS);

   if(conn->isInputConnector()) {
      //TRG_MSG_DEBUG("  ... executing input connector '" << conn->name() << "'");
      sc = executeInputConnector(dynamic_cast<InputConnector*>(conn));
   } else if(conn->isSortingConnector()) {
      //TRG_MSG_DEBUG("  ... executing sorting connector '" << conn->name() << "'");
      sc = executeSortingConnector(dynamic_cast<SortingConnector*>(conn));
   } else if(conn->isDecisionConnector()){
      //TRG_MSG_DEBUG("  ... executing decision connector '" << conn->name() << "'");
      sc = executeDecisionConnector(dynamic_cast<DecisionConnector*>(conn));
   } else if(conn->isCountingConnector()){
      sc = executeCountingConnector(dynamic_cast<CountingConnector*>(conn));
   }

   conn->setIsExecuted(true);
   conn->setExecutionStatusCode(sc);
  
   return sc;
}



TCS::StatusCode
TopoSteering::executeInputConnector(TCS::InputConnector *conn) {

   if (conn == NULL) {
     return TCS::StatusCode::FAILURE;
   }

   TCS::StatusCode sc(TCS::StatusCode::SUCCESS);

   // attaching data from inputEvent to input connector, depending on the configured input type

   const InputTOBArray * inputData = inputEvent().inputTOBs( conn->inputTOBType() );
   const bool hasInputOverflow = inputEvent().hasInputOverflow(conn->inputTOBType());
   conn->attachOutputData( inputData );
   conn->toggleInputOverflow(hasInputOverflow);

   TRG_MSG_DEBUG("  ... executing input connector '" << conn->name() << "' -> attaching '" << inputData->name() << "' of size " << inputData->size());

   return sc;
}



TCS::StatusCode
TopoSteering::executeSortingConnector(TCS::SortingConnector *conn) {

   if (conn == NULL) {
     return StatusCode::FAILURE;
   }

   TCS::StatusCode sc = TCS::StatusCode::SUCCESS;
  
   // execute all the prior connectors
   for( TCS::Connector* inputConn: conn->inputConnectors() ){
      sc &= executeConnector(inputConn);
      conn->toggleInputOverflow(conn->hasInputOverflow() ||
                                inputConn->hasInputOverflow());
   }
   TCS::SortingAlg* alg = conn->sortingAlgorithm();

   TOBArray * sortedOutput = new TOBArray(conn->outputName());

   sc &= executeSortingAlgorithm(alg, conn->inputConnector(), sortedOutput);

   conn->toggleAmbiguity(sortedOutput->ambiguityFlag());

   TRG_MSG_DEBUG("  ... executing sorting connector '" << conn->name() << "' -> attaching '" << sortedOutput->name() << "' of size " << sortedOutput->size());

   conn->attachOutputData(sortedOutput);

   return sc;
}



TCS::StatusCode
TopoSteering::executeDecisionConnector(TCS::DecisionConnector *conn) {

   if (conn == NULL) {
     return TCS::StatusCode::FAILURE;
   }

   TCS::StatusCode sc = TCS::StatusCode::SUCCESS;
  
   // execute all the prior connectors
   for( TCS::Connector* inputConn: conn->inputConnectors() ){
      sc &= executeConnector(inputConn);
      conn->toggleInputOverflow(conn->hasInputOverflow() ||
                                inputConn->hasInputOverflow());
      conn->toggleAmbiguity(conn->hasAmbiguity() ||
                                inputConn->hasAmbiguity());
   }
   // execute
   TCS::DecisionAlg* alg = conn->decisionAlgorithm();

   // TRG_MSG_DEBUG("  ... executing decision connector '" << conn->name() << "' with " << conn->triggers().size() << " active trigger lines. The algorithm has " << alg->numberOutputBits() << " output bits.");

   // the output is one TOBArray per output line
   vector<TOBArray *> output( alg->numberOutputBits() );

   for(unsigned int i=0; i<alg->numberOutputBits(); ++i) {
      output[i] = new TOBArray(conn->triggers()[i].name());
      output[i]->setAmbiguityFlag(conn->hasAmbiguity());
   }

   sc &= executeDecisionAlgorithm(alg, conn->inputConnectors(), output, conn->m_decision);

   TRG_MSG_DEBUG("  ... executing decision connector '" << conn->name() << "' -> attaching output data:");
   for(TOBArray const * outarr : output) {
      TRG_MSG_DEBUG("           data '" << outarr->name() << "' of size " << outarr->size());
      conn->toggleAmbiguity(outarr->ambiguityFlag());
   }

   conn->attachOutputData(output);

   conn->setIsExecuted(true);
   conn->setExecutionStatusCode(sc);
   bool sortOverflow = false;
   for(TCS::Connector* inputConnector: conn->inputConnectors()) {
       // TODO DG-2017-04-18 the sort overflow (>10 TOBs) in the SortAlg is not implemented yet
       if(inputConnector->isSortingConnector()) {
         if (auto sortConn = dynamic_cast<SortingConnector*>(inputConnector)) {
           sortOverflow = sortConn->sortingAlgorithm()->overflow();
           if (sortOverflow) break;
         }
       }
   }
   conn->m_decision.setOverflow(conn->hasInputOverflow() || sortOverflow);
   conn->m_decision.setAmbiguity(conn->hasAmbiguity());
   return sc;
}


TCS::StatusCode
TopoSteering::executeCountingConnector(TCS::CountingConnector *conn) {

   if (conn == NULL) {
      return TCS::StatusCode::FAILURE;
   }

   TCS::StatusCode sc = TCS::StatusCode::SUCCESS;

   // execute all the prior connectors
   for( TCS::Connector* inputConn: conn->inputConnectors() ){
      sc &= executeConnector(inputConn);
      conn->toggleInputOverflow(conn->hasInputOverflow() ||
                                inputConn->hasInputOverflow());
   }

   // execute 
   TCS::CountingAlg* alg = conn->countingAlgorithm();
   
   // Execute algorithm with no output data - not needed for now
   sc &= executeCountingAlgorithm(alg, conn->inputConnector(), conn->m_count);

   conn->setIsExecuted(true);
   conn->setExecutionStatusCode(sc);

   // TO-DO overflows in multiplicity algorithms

   return sc;
}



TCS::StatusCode
TopoSteering::executeSortingAlgorithm(TCS::SortingAlg *alg,
                                      TCS::InputConnector* inputConnector,
                                      TCS::TOBArray * & sortedOutput) {
                                           
    TRG_MSG_DEBUG("  ... executing sorting alg '" << alg->fullname() << "'"
                  <<(m_useBitwise?" (bitwise)":""));

   const InputTOBArray * input = inputConnector->outputData();

   if(m_useBitwise) alg->sortBitCorrect(*input, *sortedOutput);
   else alg->sort(*input, *sortedOutput);

   return TCS::StatusCode::SUCCESS;
}



TCS::StatusCode
TopoSteering::executeDecisionAlgorithm(TCS::DecisionAlg *alg,
                                       const std::vector<Connector*> & inputConnectors,
                                       const std::vector<TCS::TOBArray *> & output,
                                       TCS::Decision & decision) {

    TRG_MSG_DEBUG("  ... executing decision alg '" << alg->fullname() << "'"
                  <<(m_useBitwise?" (bitwise)":""));

   if(inputConnectors.size()<1) {
      TCS_EXCEPTION("L1Topo Steering: Decision algorithm expects at least 1 input array but got 0");
   }

   std::vector<TCS::TOBArray const *> input;  // needs to be optimized
   for(const Connector* inConn: inputConnectors)
   {
      if (inConn == nullptr) continue;
      const SortingConnector * sc = dynamic_cast<const SortingConnector *>(inConn);
      if (sc==nullptr) {
	 TCS_EXCEPTION("L1Topo Steering: Decision algorithm " << alg->name() << " could not cast as SortingConnector* the input connector " << inConn->name());
      }
      else {
        const TOBArray * tobA = dynamic_cast<const TOBArray *>( sc->outputData());
        if(tobA==nullptr) {
          TCS_EXCEPTION("L1Topo Steering: Decision algorithm " << alg->name() << " expects TOBArray(s) as input, but did not get it from connector " << (inConn ? inConn->name() : ""));
        }
        input.push_back( tobA );
      }
   }

   alg->reset();


   if(m_useBitwise) alg->processBitCorrect( input, output, decision);   
   else alg->process( input, output, decision );
   //TRG_MSG_ALWAYS("[XS1234sz]L1Topo Steering alg " << alg->name() << " has decision " << decision.decision());
     
   return TCS::StatusCode::SUCCESS;
}


TCS::StatusCode
TopoSteering::executeCountingAlgorithm(TCS::CountingAlg *alg,
                                      TCS::InputConnector* inputConnector,
                                      TCS::Count & count) {

   TRG_MSG_DEBUG("  ... executing multiplicity alg '" <<alg->fullname() << "'");

   const InputTOBArray * input = inputConnector->outputData();

   alg->process( *input, count);

   return TCS::StatusCode::SUCCESS;
}


void
TopoSteering::printDebugInfo() {
   TRG_MSG_INFO("Number of ClusterTOB  : " << ClusterTOB::heap().size());
   TRG_MSG_INFO("Number of eEmTOB      : " << eEmTOB::heap().size());
   TRG_MSG_INFO("Number of JetTOB      : " << JetTOB::heap().size());
   TRG_MSG_INFO("Number of jJetTOB     : " << jJetTOB::heap().size());
   TRG_MSG_INFO("Number of GenericTOB  : " << GenericTOB::heap().size());
   TRG_MSG_INFO("Number of CompositeTOB: " << CompositeTOB::heap().size());
   TRG_MSG_INFO("Number of MuonTOB     : " << MuonTOB::heap().size());
   TRG_MSG_INFO("Number of LateMuonTOB : " << LateMuonTOB::heap().size());
}



void
TopoSteering::printConfiguration(std::ostream & o) const {
   o << "==========================" << endl
     << "TopoSteering configuration" << endl
     << "--------------------------" << endl;
   structure().print(o);
   o << "==========================" << endl;
}



void
TopoSteering::setMsgLevel( TrigConf::MSGTC::Level lvl ) {

   //const char* levelNames[TrigConf::MSGTC::NUM_LEVELS] = {"NIL","VERBOSE","DEBUG","INFO",
   //                                                       "WARNING","ERROR","FATAL","ALWAYS"};
   msg().setLevel( lvl );

   inputEvent().msg().setLevel(lvl);

   m_simulationResult.setMsgLevel( lvl );
}

void
TopoSteering::setAlgMsgLevel( TrigConf::MSGTC::Level lvl ) {

   m_AlgMsgLvl = lvl;

   for( Connector * conn : m_structure.connectors() ) {
      const ConfigurableAlg * alg = conn->algorithm();
      if(alg==nullptr) continue;
      alg->msg().setLevel(lvl);
   }
}
//----------------------------------------------------------
void TopoSteering::setHardwareBits(const std::bitset<numberOfL1TopoBits> &triggerBits,
                                   const std::bitset<numberOfL1TopoBits> &ovrflowBits)
{
    m_triggerHdwBits = triggerBits;
    m_ovrflowHdwBits = ovrflowBits;
}
//----------------------------------------------------------
void TopoSteering::propagateHardwareBitsToAlgos()
{
   for(auto connector : m_structure.outputConnectors()) {
       const string &connectorName = connector.first;
       TCS::DecisionConnector *outCon = connector.second;
        outCon->decisionAlgorithm()->resetHardwareBits();
        unsigned int pos = 0; // for multi-output algorithms pos is the output index
        for(const TrigConf::TriggerLine &trigger : outCon->triggers()){
	    unsigned int bitNumber = trigger.flatindex();
            outCon->decisionAlgorithm()->setHardwareBits(pos,
                                                         m_triggerHdwBits[bitNumber],
                                                         m_ovrflowHdwBits[bitNumber]);
            pos++;
            TRG_MSG_DEBUG("propagating hardware bit (dec/ovr) "<<bitNumber
                          <<" to algo "<<connectorName<<"["<<pos<<"]"
                          <<" "<<m_triggerHdwBits[bitNumber]<<" /"
                          <<" "<<m_ovrflowHdwBits[bitNumber]);

        }
   }
}
//----------------------------------------------------------
void TopoSteering::setOutputAlgosFillBasedOnHardware(const bool &value)
{
   for(auto connector : m_structure.outputConnectors()) {
       const string &connectorName = connector.first;
       TCS::DecisionConnector *outCon = connector.second;
       if(outCon) {
           outCon->decisionAlgorithm()->setFillHistosBasedOnHardware(value);
       } else {
           TRG_MSG_DEBUG("skipping invalid DecisionConnector '"<<connectorName<<"'");
       }
   }
}
//----------------------------------------------------------
void TopoSteering::setOutputAlgosSkipHistograms(const bool &value)
{
   for(auto connector : m_structure.outputConnectors()) {
       const string &connectorName = connector.first;
       TCS::DecisionConnector *outCon = connector.second;
       if(outCon) {
           outCon->decisionAlgorithm()->setSkipHistos(value);
       } else {
           TRG_MSG_DEBUG("skipping invalid DecisionConnector '"<<connectorName<<"'");
       }
   }
}
//----------------------------------------------------------
