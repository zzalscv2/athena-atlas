/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////// 
// HepMcReaderTool.cxx 
// Implementation file for class HepMcReaderTool
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 


// STL includes
#include <algorithm>
#include <cctype>


//HepMC includes
#include "GeneratorObjects/McEventCollection.h"
#include "AtlasHepMC/IO_GenEvent.h"

// McParticleTools includes
#include "HepMcReaderTool.h"

static const char * const s_protocolSep = ":";


/////////////////////////////////////////////////////////////////// 
/// Public methods: 
/////////////////////////////////////////////////////////////////// 

/// Constructors
////////////////
HepMcReaderTool::HepMcReaderTool( const std::string& type, 
				  const std::string& name, 
				  const IInterface* parent ) : 
  AthAlgTool( type, name, parent ),
  m_ioFrontend( nullptr )
{
  //
  // Property declaration
  // 

  declareProperty( "Input", 
		   m_ioFrontendURL = "ascii:hepmc.genevent.txt", 
		   "Name of the front-end we'll use to read in the HepMC::GenEvent."
		   "\nEx: ascii:hepmc.genevent.txt" );
  m_ioFrontendURL.declareUpdateHandler( &HepMcReaderTool::setupFrontend,
					this );

  declareProperty( "McEventsOutput",
		   m_mcEventsOutputName = "GEN_EVENT",
		   "Output location of the McEventCollection to read out" );

  declareInterface<IIOHepMcTool>(this);
}

/// Destructor
///////////////
HepMcReaderTool::~HepMcReaderTool()
{ 
  ATH_MSG_DEBUG("Calling destructor");

  delete m_ioFrontend;
  m_ioFrontend = nullptr;
}

/// Athena Algorithm's Hooks
////////////////////////////
StatusCode HepMcReaderTool::initialize()
{
  ATH_MSG_INFO("Initializing " << name() << "...");
  // Get pointer to StoreGateSvc and cache it :
  if ( !evtStore().retrieve().isSuccess() ) {
    ATH_MSG_ERROR("Unable to retrieve pointer to StoreGateSvc");
    return StatusCode::FAILURE;
  }

  // setup frontend
  if ( nullptr == m_ioFrontend ) {
    setupFrontend(m_ioFrontendURL);
  }

  return StatusCode::SUCCESS;
}

StatusCode HepMcReaderTool::finalize()
{
  ATH_MSG_INFO("Finalizing " << name() << "...");
  return StatusCode::SUCCESS;
}

StatusCode HepMcReaderTool::execute()
{
  // create a new McEventCollection and put it into StoreGate
  McEventCollection * mcEvts = new McEventCollection;
  if ( evtStore()->record( mcEvts, m_mcEventsOutputName ).isFailure() ) {
    ATH_MSG_ERROR("Could not record a McEventCollection at ["<< m_mcEventsOutputName << "] !!");
    return StatusCode::FAILURE;
  }
  
  if ( evtStore()->setConst( mcEvts ).isFailure() ) {
    ATH_MSG_WARNING("Could not setConst McEventCollection at ["<< m_mcEventsOutputName << "] !!");
  }

  HepMC::GenEvent * evt = new HepMC::GenEvent;
  mcEvts->push_back(evt);

  return read(evt);
}

/////////////////////////////////////////////////////////////////// 
/// Non-const methods: 
/////////////////////////////////////////////////////////////////// 

StatusCode HepMcReaderTool::read( HepMC::GenEvent* evt )
{
#ifdef HEPMC3
  m_ioFrontend->read_event(*evt);
#else
  m_ioFrontend->fill_next_event(evt);
#endif

  return StatusCode::SUCCESS;
}

void HepMcReaderTool::setupFrontend( Gaudi::Details::PropertyBase& /*prop*/ )
{
  // defaults
  std::string protocol = "ascii";
  std::string fileName = "hepmc.genevent.txt";

  // reset internal state
  if ( m_ioFrontend ) {
    delete m_ioFrontend;
    m_ioFrontend = nullptr;
  }

  // caching URL
  const std::string& url = m_ioFrontendURL.value();
  
  std::string::size_type protocolPos = url.find(s_protocolSep);

  if ( std::string::npos != protocolPos ) {
    protocol = url.substr( 0, protocolPos );
    fileName = url.substr( protocolPos+1, std::string::npos );
  } else {
    //protocol = "ascii";
    fileName = url;
  }

  // get the protocol name in lower cases
  std::transform( protocol.begin(), protocol.end(), protocol.begin(), [](unsigned char c){ return std::tolower(c); } );

#ifdef HEPMC3
  if ( "ascii" == protocol ) {
    m_ioFrontend = new HepMC3::ReaderAsciiHepMC2( fileName.c_str());

  } else {
    msg(MSG::WARNING) << "UNKNOWN protocol [" << protocol << "] !!" << endmsg<< "Will use [ascii] instead..."<< endmsg;
    protocol = "ascii";
    m_ioFrontend = new HepMC3::ReaderAsciiHepMC2( fileName.c_str());
  }    
#else 
  if ( "ascii" == protocol ) {
    m_ioFrontend = new HepMC::IO_GenEvent( fileName.c_str(), std::ios::in );

  } else {
    msg(MSG::WARNING) << "UNKNOWN protocol [" << protocol << "] !!" << endmsg << "Will use [ascii] instead..."<< endmsg;
    protocol = "ascii";
    m_ioFrontend = new HepMC::IO_GenEvent( fileName.c_str(), std::ios::in );
  }    
#endif

  ATH_MSG_DEBUG("Using protocol [" << protocol << "] and write to ["<< fileName << "]");
}
