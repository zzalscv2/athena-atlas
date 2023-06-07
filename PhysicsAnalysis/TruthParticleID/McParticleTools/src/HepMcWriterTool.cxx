/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////// 
// HepMcWriterTool.cxx 
// Implementation file for class HepMcWriterTool
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 

// STL includes
#include <algorithm>
#include <cctype>

// HepMC includes
#include "GeneratorObjects/McEventCollection.h"
#include "AtlasHepMC/IO_GenEvent.h"

// McParticleTools includes
#include "HepMcWriterTool.h"

static const char * const s_protocolSep = ":";

/////////////////////////////////////////////////////////////////// 
/// Constructors
////////////////
HepMcWriterTool::HepMcWriterTool( const std::string& type,   const std::string& name,  const IInterface* parent ) : 
  AthAlgTool( type, name, parent ),
  m_ioBackend( nullptr )
{
  //
  // Property declaration
  // 

  declareProperty( "Output", 
		   m_ioBackendURL = "ascii:hepmc.genevent.txt", 
		   "Name of the back-end we'll use to write out the HepMC::GenEvent."
		   "\nEx: ascii:hepmc.genevent.txt" );
  m_ioBackendURL.declareUpdateHandler( &HepMcWriterTool::setupBackend,
				       this );

  declareProperty( "McEvents",
		   m_mcEventsName = "GEN_EVENT",
		   "Input location of the McEventCollection to write out" );

  declareInterface<IIOHepMcTool>(this);
}

/// Destructor
///////////////
HepMcWriterTool::~HepMcWriterTool()
{ 
  ATH_MSG_DEBUG("Calling destructor");

  if ( m_ioBackend ) {
    delete m_ioBackend;
    m_ioBackend = nullptr;
  }
}

/// Athena Algorithm's Hooks
////////////////////////////
StatusCode HepMcWriterTool::initialize()
{
  ATH_MSG_INFO("Initializing " << name() << "...");

  // Get pointer to StoreGateSvc and cache it :
  if ( !evtStore().retrieve().isSuccess() ) {
    ATH_MSG_ERROR("Unable to retrieve pointer to StoreGateSvc");
    return StatusCode::FAILURE;
  }

  // setup backend
  if ( nullptr == m_ioBackend ) {
    setupBackend(m_ioBackendURL);
  }

  return StatusCode::SUCCESS;
}

StatusCode HepMcWriterTool::finalize()
{
  ATH_MSG_INFO("Finalizing " << name() << "...");
  return StatusCode::SUCCESS;
}

StatusCode HepMcWriterTool::execute()
{
  // retrieve the McEventCollection
  const McEventCollection * mcEvts = nullptr;
  if ( evtStore()->retrieve( mcEvts, m_mcEventsName ).isFailure() || nullptr == mcEvts ) {
    ATH_MSG_ERROR("Could not retrieve a McEventCollection at ["  << m_mcEventsName << "] !!");
    return StatusCode::FAILURE;
  }

  if ( mcEvts->empty() ) {
    ATH_MSG_WARNING("McEventCollection at ["<<m_mcEventsName<<"] is EMPTY !!");
    return StatusCode::FAILURE;
  }

  const HepMC::GenEvent * evt = mcEvts->front();
  if ( !evt ) {
    ATH_MSG_ERROR("Retrieved NULL pointer to HepMC::GenEvent !!");
    return StatusCode::FAILURE;
  }

  return write(evt);
}

/////////////////////////////////////////////////////////////////// 
/// Non-const methods: 
/////////////////////////////////////////////////////////////////// 

StatusCode HepMcWriterTool::write( const HepMC::GenEvent* evt )
{
#ifdef HEPMC3
  m_ioBackend->write_event(*evt);
#else
  m_ioBackend->write_event(evt);
#endif
  return StatusCode::SUCCESS;
}

void HepMcWriterTool::setupBackend( Gaudi::Details::PropertyBase& /*prop*/ )
{
  // defaults
  std::string protocol = "ascii";
  std::string fileName = "hepmc.genevent.txt";

  // reset internal state
  if ( m_ioBackend ) {
    delete m_ioBackend;
    m_ioBackend = nullptr;
  }

  // caching URL
  const std::string& url = m_ioBackendURL.value();
  
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
    m_ioBackend = new HepMC3::WriterAsciiHepMC2( fileName.c_str());

  } else {
    ATH_MSG_WARNING("UNKNOWN protocol [" << protocol << "] !!" << endmsg  << "Will use [ascii] instead...");
    protocol = "ascii";
    m_ioBackend = new HepMC3::WriterAsciiHepMC2( fileName.c_str());
  }    
#else
  if ( "ascii" == protocol ) {
    m_ioBackend = new HepMC::IO_GenEvent( fileName.c_str(), std::ios::out | std::ios::trunc );

  } else {
    ATH_MSG_WARNING("UNKNOWN protocol [" << protocol << "] !!" << endmsg  << "Will use [ascii] instead...");
    protocol = "ascii";
    m_ioBackend = new HepMC::IO_GenEvent( fileName.c_str(), std::ios::out | std::ios::trunc );
  }    
#endif
  ATH_MSG_DEBUG("Using protocol [" << protocol << "] and write to ["<< fileName << "]");
}
