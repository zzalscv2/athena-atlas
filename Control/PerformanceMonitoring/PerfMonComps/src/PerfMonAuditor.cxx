///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// PerfMonAuditor.cxx 
// Implementation file for class PerfMonAuditor
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 


// STL includes

// FrameWork includes
#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/MsgStream.h" 

// PerfMonKernel includes
#include "PerfMonKernel/IPerfMonSvc.h"

// PerfMonComps includes
#include "PerfMonAuditor.h"

using PerfMon::State;
using PerfMon::Steps;
using namespace Athena;

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////
PerfMonAuditor::PerfMonAuditor( const std::string& name, 
                                ISvcLocator* pSvcLocator ) : 
  Auditor    ( name,            pSvcLocator  ),
  m_monSvc   ( "PerfMonSvc/PerfMonSvc", name )
{
  //
  // Property declaration
  // 
  //declareProperty( "Property", m_nProperty, "descr" );

  // for AuditorHandle ? someday ?
  //declareInterface<IAuditor>(this);
}

// Destructor
///////////////
PerfMonAuditor::~PerfMonAuditor()
{ 
  //m_msg << MSG::DEBUG << "Calling destructor" << endmsg;
}

StatusCode PerfMonAuditor::initialize()
{
  MsgStream msg( msgSvc(), name() );

  if ( !m_monSvc.retrieve().isSuccess() ) {
    msg << MSG::ERROR
        << "Could not retrieve [" << m_monSvc.typeAndName() << "] !!"
        << endmsg;
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}


/////////////////////////////////////////////////////////////////// 
// Non-const methods: 
/////////////////////////////////////////////////////////////////// 

void PerfMonAuditor::beforeInitialize( INamedInterface* component )
{
  return m_monSvc->startAud( PerfMon::Steps[State::ini], component->name() );
}

void PerfMonAuditor::afterInitialize( INamedInterface* component )
{
  return m_monSvc->stopAud( PerfMon::Steps[State::ini], component->name() );
}

void PerfMonAuditor::beforeReinitialize( INamedInterface* /*component*/ )
{
  // don't poll for re-init...
  return;
}

void PerfMonAuditor::afterReinitialize( INamedInterface* /*component*/ )
{
  // don't poll for re-init
  return;
}

void PerfMonAuditor::beforeExecute( INamedInterface* component )
{
  return m_monSvc->startAud( PerfMon::Steps[State::evt], component->name() );
}

void PerfMonAuditor::afterExecute( INamedInterface* component, 
				   const StatusCode& ) 
{
  return m_monSvc->stopAud( PerfMon::Steps[State::evt], component->name() );
}

void PerfMonAuditor::beforeFinalize( INamedInterface* component )
{
  return m_monSvc->startAud( PerfMon::Steps[State::fin], component->name() );
}

void PerfMonAuditor::afterFinalize( INamedInterface* component )
{
  return m_monSvc->stopAud( PerfMon::Steps[State::fin], component->name() );
}

void PerfMonAuditor::before(CustomEventTypeRef evt, const std::string& caller)
{
  return m_monSvc->startAud (evt=="Callback" ? "cbk" : evt, caller);
}

void PerfMonAuditor::after(CustomEventTypeRef evt, const std::string& caller,
			   const StatusCode&)
{
  return m_monSvc->stopAud (evt=="Callback" ? "cbk" : evt, caller);
}


