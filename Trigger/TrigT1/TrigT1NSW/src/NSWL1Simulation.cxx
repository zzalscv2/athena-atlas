/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// Athena/Gaudi includes
#include "GaudiKernel/ITHistSvc.h"

// root includes
#include "TTree.h"
// Local includes
#include "NSWL1Simulation.h"
#include "MuonRDO/NSW_TrigRawDataContainer.h"
#include <vector>

namespace NSWL1 {

  NSWL1Simulation::NSWL1Simulation( const std::string& name, ISvcLocator* pSvcLocator )
    : AthAlgorithm( name, pSvcLocator ),
      m_monitors(this),
      m_pad_tds("NSWL1::PadTdsOfflineTool",this),
      m_pad_trigger("NSWL1::PadTriggerLogicOfflineTool",this),
      m_pad_trigger_lookup("NSWL1::PadTriggerLookupTool",this),
      m_strip_tds("NSWL1::StripTdsOfflineTool",this),
      //m_strip_cluster("NSWL1::StripClusterTool",this),
      //m_strip_segment("NSWL1::StripSegmentTool",this), TODO: this line makes the code crash in initialization... please, sTGC friends, fix it!!!
      m_mmstrip_tds("NSWL1::MMStripTdsOfflineTool",this),
      m_mmtrigger("NSWL1::MMTriggerTool",this),
      m_tree(nullptr),
      m_current_run(-1),
      m_current_evt(-1)
  {

    // Property setting general behaviour:
    declareProperty( "DoOffline",    m_doOffline    = false, "Steers the offline emulation of the LVL1 logic" );
    declareProperty( "UseLookup",    m_useLookup    = false, "Toggle Lookup mode on and off default is the otf(old) mode" );
    declareProperty( "DoNtuple",     m_doNtuple     = true,  "Create an ntuple for data analysis" );
    declareProperty( "DoMM",         m_doMM         = true,  "Run data analysis for MM" );
    declareProperty( "DoMMDiamonds", m_doMMDiamonds = false, "Run data analysis for MM using Diamond Roads algorithm" );
    declareProperty( "DosTGC",       m_dosTGC       = false, "Run data analysis for sTGCs" );

    // declare monitoring tools
    declareProperty( "AthenaMonTools",          m_monitors,           "List of monitoring tools to be run with this instance, if incorrect then tool is silently skipped.");
    declareProperty( "PadTdsTool",              m_pad_tds,            "Tool that simulates the functionalities of the PAD TDS");
    //PadTriggerTool : in principle can be totally wuiped out. necesary for ntuples currently. Once you isolate ntuple making code and the trigger code you can abandon this method. Things ae still tangled a bit somewhow so keep it just in case
    declareProperty( "PadTriggerTool",          m_pad_trigger,        "Tool that simulates the pad trigger logic");
    declareProperty( "PadTriggerLookupTool",    m_pad_trigger_lookup, "Tool that is used to lookup pad trigger patterns per execute against the same LUT as in trigger FPGA");
    declareProperty( "StripTdsTool",            m_strip_tds,          "Tool that simulates the functionalities of the Strip TDS");
    declareProperty( "StripClusterTool",        m_strip_cluster,      "Tool that simulates the Strip Clustering");
    declareProperty( "StripSegmentTool",        m_strip_segment,      "Tool that simulates the Segment finding");
    declareProperty( "MMStripTdsTool",          m_mmstrip_tds,        "Tool that simulates the functionalities of the MM STRIP TDS");
    declareProperty( "MMTriggerTool",           m_mmtrigger,          "Tool that simulates the MM Trigger");
    declareProperty( "NSWTrigRDOContainerName", m_trigRdoContainer = "NSWTRGRDO"," Give a name to NSW trigger rdo container");
    declareProperty( "PadTriggerRDOName",       m_padTriggerRdoKey = "NSWPADTRGRDO", "Name of the pad trigger RDO");
  }


  StatusCode NSWL1Simulation::initialize() {
    ATH_MSG_DEBUG( "initialize " << name() );
    ATH_CHECK( m_trigRdoContainer.initialize() );
    ATH_CHECK( m_xaodevtKey.initialize() );
    ATH_CHECK( m_eventInfoKey.initialize() );
    // Create an register the ntuple if requested, add branch for event and run number
    if ( m_doNtuple ) {
      ITHistSvc* tHistSvc;
      ATH_CHECK(service("THistSvc", tHistSvc));
      char ntuple_name[40];
      memset(ntuple_name,'\0',40*sizeof(char));
      sprintf(ntuple_name,"%sTree",name().c_str());
      m_current_evt = 0, m_current_run = 0;

      // create Ntuple and the branches
      m_tree = new TTree(ntuple_name, "Ntuple of NSWL1Simulation");
      m_tree->Branch("runNumber",   &m_current_run, "runNumber/i");
      m_tree->Branch("eventNumber", &m_current_evt, "eventNumber/i");

      char tdir_name[80];
      memset(tdir_name,'\0',80*sizeof(char));
      sprintf(tdir_name,"/%s/%s",name().c_str(),ntuple_name);
      ATH_CHECK(tHistSvc->regTree(tdir_name,m_tree));
    }

    // retrieving the private tools implementing the simulation
    if(m_dosTGC){
      ATH_CHECK(m_pad_tds.retrieve());
      if(m_useLookup){
        ATH_CHECK(m_pad_trigger_lookup.retrieve());
      }
      else{
        ATH_CHECK(m_pad_trigger.retrieve());
      }
      ATH_CHECK(m_strip_tds.retrieve());
      //ATH_CHECK(m_strip_cluster.retrieve());
      //ATH_CHECK(m_strip_segment.retrieve());
      ATH_CHECK(m_padTriggerRdoKey.initialize());
    }

    if(m_doMM ){
      ATH_CHECK(m_mmtrigger.retrieve());
    }

    // Connect to Monitoring Service
    if(m_doNtuple){
      ATH_CHECK(m_monitors.retrieve());
    }
    return StatusCode::SUCCESS;
  }


  StatusCode NSWL1Simulation::start() {
    ATH_MSG_DEBUG("start " << name() );
    if(m_doNtuple){
      for ( auto& mon : m_monitors ) {
        ATH_CHECK(mon->bookHists());
      }
    }
    return StatusCode::SUCCESS;
  }


  StatusCode NSWL1Simulation::execute() {
    SG::ReadHandle<xAOD::EventInfo> evt(m_xaodevtKey);
    if (evt.isValid()) {
      m_current_run = evt->runNumber();
      m_current_evt = evt->eventNumber();
    } else {
      SG::ReadHandle<EventInfo> eventInfo (m_eventInfoKey);
      if(eventInfo->event_ID()->event_number() == 0 && m_current_evt == 0) ATH_MSG_DEBUG("Start " << name());
      else ++m_current_evt;
      m_current_run = eventInfo->event_ID()->run_number();
    }

    std::vector<std::shared_ptr<PadData>> pads;
    std::vector<std::unique_ptr<PadTrigger>> padTriggers;
    std::vector<std::unique_ptr<StripData>> strips;
    std::vector< std::unique_ptr<StripClusterData> > clusters;
    auto trgContainer=std::make_unique<Muon::NSW_TrigRawDataContainer>();

    if(m_dosTGC){
      ATH_CHECK( m_pad_tds->gather_pad_data(pads) );
      if(m_useLookup){
        ATH_CHECK( m_pad_trigger_lookup->lookup_pad_triggers(pads, padTriggers) );
      }
      else{
        ATH_CHECK( m_pad_trigger->compute_pad_triggers(pads, padTriggers) );
      }

      ATH_CHECK( m_strip_tds->gather_strip_data(strips,padTriggers) );
      //ATH_CHECK( m_strip_cluster->cluster_strip_data(strips,clusters) );
      //ATH_CHECK( m_strip_segment->find_segments(clusters,trgContainer) );

      auto padTriggerRdoHandle = SG::makeHandle(m_padTriggerRdoKey);
      auto padTriggerContainer = std::make_unique<Muon::NSW_PadTriggerDataContainer>();
      ATH_CHECK(PadTriggerAdapter::fillContainer(padTriggerContainer, padTriggers, m_current_evt));
      ATH_CHECK(padTriggerRdoHandle.record(std::move(padTriggerContainer)));

      auto rdohandle = SG::makeHandle( m_trigRdoContainer );
      ATH_CHECK( rdohandle.record( std::move(trgContainer)));
    }

    //retrive the MM Strip hit data
    if(m_doMM){
      ATH_CHECK( m_mmtrigger->runTrigger(m_doMMDiamonds) );
    }
    if(m_doNtuple){
      for ( auto& mon : m_monitors) {
        ATH_CHECK(mon->fillHists());
      }
      if (m_tree) m_tree->Fill();
    }

    // Dump content of the pad trigger collection
    if (m_dosTGC)
    {
      const Muon::NSW_PadTriggerDataContainer* padTriggerContainer;
      ATH_CHECK(evtStore()->retrieve(padTriggerContainer, m_padTriggerRdoKey.key()));
      ATH_MSG_DEBUG("Pad Trigger Container size: " << padTriggerContainer->size());
      for (const auto &padTriggerData : *padTriggerContainer)
      {
        ATH_MSG_DEBUG("  " << *padTriggerData);
        for (const auto & padTriggerSegment : *padTriggerData)
        {
          ATH_MSG_DEBUG("    " << *padTriggerSegment);
        }
      }
    }
    return StatusCode::SUCCESS;
  }


  StatusCode NSWL1Simulation::finalize() {
    ATH_MSG_DEBUG( "finalize" << name() );
    if(m_doNtuple){
      for ( auto& mon :  m_monitors ) {
        ATH_CHECK(mon->finalHists());
      }
    }
    return StatusCode::SUCCESS;
  }

  int NSWL1Simulation::resultBuilder() const {
    return 0;
  }

}
