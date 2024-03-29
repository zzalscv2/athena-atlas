///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// CallGraphAuditor.cxx 
// Implementation file for class PerfMon::CallGraphAuditor
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 


// STL includes

// FrameWork includes
#include "GaudiKernel/INamedInterface.h"

// PerfMonKernel includes
#include "PerfMonKernel/ICallGraphBuilderSvc.h"

// PerfMonComps includes
#include "CallGraphAuditor.h"

namespace PerfMon {

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////
CallGraphAuditor::CallGraphAuditor( const std::string& name, 
				    ISvcLocator* pSvcLocator ) : 
  Auditor       ( name, pSvcLocator ),
  m_callGraphSvc( "PerfMon::CallGraphBuilderSvc/CallGraphSvc", name )
{
  //
  // Property declaration
  // 
  //declareProperty( "Property", m_nProperty, "descr" );

  // for AuditorHandle ? someday ?
  //declareInterface<ICallGraphAuditor>(this);
}

// Destructor
///////////////
CallGraphAuditor::~CallGraphAuditor()
{ 
  //m_msg << MSG::DEBUG << "Calling destructor" << endmsg;
}


/////////////////////////////////////////////////////////////////// 
// Non-const methods: 
/////////////////////////////////////////////////////////////////// 

StatusCode
CallGraphAuditor::initialize()
{
  // retrieve the callgraph service
  if ( !m_callGraphSvc.retrieve().isSuccess() ) {
    // FIXME: use a datamember MsgStream !
    MsgStream msg( msgSvc(), name() );
    msg << MSG::ERROR
	<< "Could not retrieve [" << m_callGraphSvc.typeAndName() << "] !!"
	<< endmsg;
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

void CallGraphAuditor::beforeInitialize( INamedInterface* alg )
{
  m_callGraphSvc->openNode( alg->name() + ":initialize" );
  return;
}

void CallGraphAuditor::afterInitialize( INamedInterface* alg )
{
  m_callGraphSvc->closeNode( alg->name() + ":initialize" );
  return;
}

void CallGraphAuditor::beforeReinitialize( INamedInterface* alg )
{
  m_callGraphSvc->openNode( alg->name() + ":reinitialize" );
  return;
}

void CallGraphAuditor::afterReinitialize( INamedInterface* alg )
{
  m_callGraphSvc->closeNode( alg->name() + ":reinitialize" );
  return;
}

void CallGraphAuditor::beforeExecute( INamedInterface* alg )
{
  m_callGraphSvc->openNode( alg->name() + ":execute" );
  return;
}

void CallGraphAuditor::afterExecute( INamedInterface* alg, const StatusCode& ) 
{
  m_callGraphSvc->closeNode( alg->name() + ":execute" );
  return;
}

void CallGraphAuditor::beforeFinalize( INamedInterface* alg )
{
  m_callGraphSvc->openNode( alg->name() + ":finalize" );
  return;
}

void CallGraphAuditor::afterFinalize( INamedInterface* alg )
{
  m_callGraphSvc->closeNode( alg->name() + ":finalize" );
  return;
}


} // end namespace PerfMon
