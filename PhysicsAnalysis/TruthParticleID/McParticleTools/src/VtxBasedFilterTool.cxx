/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////// 
// VtxBasedFilterTool.cxx 
// Implementation file for class VtxBasedFilterTool
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 


// STL includes
#include <cmath>

// FrameWork includes

// CLHEP/HepMC includes
#include "CLHEP/Units/SystemOfUnits.h"

// McParticleKernel includes
#include "McParticleKernel/IMcVtxFilterTool.h"

// McParticleTools includes
#include "VtxBasedFilterTool.h"
#include "copyBeamParticles.h"
#include "AtlasHepMC/Flow.h"

/// Constructors
////////////////
VtxBasedFilterTool::VtxBasedFilterTool( const std::string& type, 
				  const std::string& name, 
				  const IInterface* parent ) : 
  TruthParticleFilterBaseTool( type, name, parent ){}

/// Destructor
///////////////
VtxBasedFilterTool::~VtxBasedFilterTool()
{ 
  ATH_MSG_DEBUG("Calling destructor");
}


/////////////////////////////////////////////////////////////////// 
/// Non-const methods: 
/////////////////////////////////////////////////////////////////// 

StatusCode VtxBasedFilterTool::buildMcAod( const McEventCollection* in,
					McEventCollection* out )
{
  if ( !in || ! out ) {
    msg(MSG::ERROR)
      << "Invalid pointer to McEventCollection !" << endmsg
      << "  in: " << in << endmsg
      << " out: " << out << endmsg;
    return StatusCode::FAILURE;
  }

  for ( unsigned int iEvt = 0; iEvt != in->size(); ++iEvt ) {
    const HepMC::GenEvent * inEvt = (*in)[iEvt];
    if ( !inEvt ) {
      msg(MSG::WARNING)
        << "Could not launch filtering procedure for GenEvent number ["
        << iEvt << "] from McEventCollection ["
        << m_mcEventsReadHandleKey.key() << " !!"
        << endmsg
        << "  inEvt: " << inEvt << endmsg;
      continue;
    }
    HepMC::GenEvent* outEvt =  HepMC::copyemptyGenEvent(inEvt);
    if ( buildGenEvent( inEvt, outEvt ).isFailure() ) {
      msg(MSG::ERROR)
	<< "Could filter GenEvent number [" << iEvt 
	<< "] from McEventCollection ["     << m_mcEventsReadHandleKey.key() 
	<< "] !!" << endmsg;
      delete outEvt;
      outEvt = nullptr;
      continue;
    }

    TruthHelper::copyBeamParticles (*inEvt, *outEvt);

    out->push_back( outEvt );
  }

  return StatusCode::SUCCESS;
}


StatusCode VtxBasedFilterTool::buildGenEvent( const HepMC::GenEvent* in,
					      HepMC::GenEvent* out )
{
  if ( !in || !out ) {
    msg(MSG::ERROR)
      << "Invalid pointer to GenEvent !!" << endmsg
      << "  in: " << in << endmsg
      << " out: " << out << endmsg;
    return StatusCode::FAILURE;
  }

#ifdef HEPMC3
  for (const auto& vtx: in->vertices()){
#else
  // loop over vertices
  for ( HepMC::GenEvent::vertex_const_iterator vtxit = in->vertices_begin(); vtxit != in->vertices_end(); ++vtxit ) {
    auto vtx=*vtxit;
#endif
    if ( !isAccepted(vtx) ) {
      // no in-going nor out-going particles at this vertex matches 
      // the requirements: ==> Skip it
      continue;
    }
    
    if ( addVertex( vtx, out ).isFailure() ) {
      msg(MSG::WARNING) << "Could not add vertex [" << vtx << "]" << endmsg;
    }
  } //> end loop over vertices
  
  return StatusCode::SUCCESS;
}

bool VtxBasedFilterTool::isAccepted( const HepMC::ConstGenVertexPtr& vtx ) const
{
  if ( !vtx ) {
    return false;
  }

  // check if this vertex looks like a vertex from the hard-scattering
  if ( isFromHardScattering(vtx) ) {
    return true;
  }

  // check if this vertex matches some decay pattern given 
  // to the McVtxFilterTool
  if ( m_mcVtxFilterTool->isAccepted(vtx) ) {
    return true;
  }

  return false;
}

StatusCode VtxBasedFilterTool::addVertex( const HepMC::ConstGenVertexPtr& srcVtx,
				       HepMC::GenEvent* evt ) const
{
  if ( !srcVtx || !evt ) {
    msg(MSG::ERROR)
      << "In addVertex(vtx,evt) : INVALID pointer given !!" << endmsg
      << " vtx: " << srcVtx << endmsg
      << " evt: " << evt << endmsg;
    return StatusCode::FAILURE;
  }

#ifdef HEPMC3
  HepMC::GenVertexPtr  vtx = (evt == srcVtx->parent_event()) ? std::const_pointer_cast<HepMC3::GenVertex>(srcVtx) : nullptr;
  if ( !vtx ) {
    vtx = HepMC::newGenVertexPtr();
    evt->add_vertex(vtx);
    vtx->set_position( srcVtx->position() );
    vtx->set_status( srcVtx->status() );
    HepMC::suggest_barcode(vtx, HepMC::barcode(srcVtx) );
    vtx->add_attribute("weights",srcVtx->attribute<HepMC3::VectorDoubleAttribute> ("weights"));
  }

  ////////////////////////////
  /// Fill the parent branch
  for ( const auto& parent: srcVtx->particles_in() ) {
    HepMC::GenParticlePtr p = (evt == parent->parent_event()) ? std::const_pointer_cast<HepMC3::GenParticle>(parent) : nullptr;
    if ( !p ) {
      p = HepMC::newGenParticlePtr();
      vtx->add_particle_in( p );
      p->set_momentum( parent->momentum() );
      p->set_generated_mass( parent->generated_mass() );
      p->set_pdg_id( parent->pdg_id() );
      p->set_status( parent->status() );
      HepMC::set_flow(p, HepMC::flow(parent) );
      HepMC::set_polarization(p, HepMC::polarization(parent) );
	  HepMC::suggest_barcode(p,HepMC::barcode(parent) );
    } else {
    // set the mother's decay to our (new) vertex
    vtx->add_particle_in( p );
    }
  }//> loop over ingoing particles
  
  //////////////////////////////
  /// Fill the children branch
  for ( const auto& child: srcVtx->particles_out()) {
    HepMC::GenParticlePtr p = (evt == child->parent_event()) ? std::const_pointer_cast<HepMC3::GenParticle>(child) : nullptr;
    if ( !p ) {
      p = HepMC::newGenParticlePtr();
      vtx->add_particle_out( p );
      p->set_momentum( child->momentum() );
      p->set_generated_mass( child->generated_mass() );
      p->set_pdg_id( child->pdg_id() );
      HepMC::set_flow(p, HepMC::flow(child) );
      HepMC::set_polarization(p, HepMC::polarization(child) );
	  HepMC::suggest_barcode(p,HepMC::barcode(child) );
    } else {
    // set the daughter's production vertex to our new vertex
    vtx->add_particle_out( p );
    }
  }//> loop over outgoing particles
#else
  HepMC::GenVertex * vtx = evt->barcode_to_vertex(srcVtx->barcode());
  if ( 0 == vtx ) {
    vtx = HepMC::newGenVertexPtr();
    vtx->set_position( srcVtx->position() );
    vtx->set_id( srcVtx->id() );
    vtx->suggest_barcode( HepMC::barcode(srcVtx) );
    vtx->weights() = srcVtx->weights();
    evt->add_vertex(vtx);
  }

  ////////////////////////////
  /// Fill the parent branch
  for ( HepMC::GenVertex::particles_in_const_iterator 
	  parent    = srcVtx->particles_in_const_begin(),
	  parentEnd = srcVtx->particles_in_const_end();
	parent != parentEnd;
	++parent ) {
    HepMC::GenParticle * p = evt->barcode_to_particle( (*parent)->barcode() );
    if ( 0 == p ) {
      p = HepMC::newGenParticlePtr();
      p->set_momentum( (*parent)->momentum() );
      p->set_generated_mass( (*parent)->generated_mass() );
      p->set_pdg_id( (*parent)->pdg_id() );
      p->set_status( (*parent)->status() );
      p->set_flow( (*parent)->flow() );
      p->set_polarization( (*parent)->polarization() );
      p->suggest_barcode( (*parent)->barcode() );
    }
    // set the mother's decay to our (new) vertex
    vtx->add_particle_in( p );
    
  }//> loop over ingoing particles
  
  //////////////////////////////
  /// Fill the children branch
  for ( HepMC::GenVertex::particles_out_const_iterator 
	  child = srcVtx->particles_out_const_begin(),
	  childEnd = srcVtx->particles_out_const_end();
	child != childEnd;
	++child ) {
    HepMC::GenParticle * p = evt->barcode_to_particle( (*child)->barcode() );
    if ( 0 == p ) {
      p = HepMC::newGenParticlePtr();
      p->set_momentum( (*child)->momentum() );
      p->set_generated_mass( (*child)->generated_mass() );
      p->set_pdg_id( (*child)->pdg_id() );
      p->set_flow( (*child)->flow() );
      p->set_polarization( (*child)->polarization() );
      p->suggest_barcode( (*child)->barcode() );
    }

    // set the daughter's production vertex to our new vertex
    vtx->add_particle_out( p );

  }//> loop over outgoing particles
#endif

  return StatusCode::SUCCESS;
}

bool VtxBasedFilterTool::isPartonVertex( const HepMC::ConstGenVertexPtr& vtx ) const
{
 if (!vtx) return false;
#ifdef HEPMC3
 for (auto& p: vtx->particles_in()) {
   if (MC::isHadron(p)&&!MC::isBeam(p)) return false;
   auto pv = p->production_vertex();
   if (pv && !isPartonVertex(pv)) return false;
 }
 for (auto& p: vtx->particles_out()) {
   if (MC::isHadron(p)&&!MC::isBeam(p)) return false;
 }
#else
 for ( auto  p = vtx->particles_in_const_begin(), parentEnd = vtx->particles_in_const_end(); p != parentEnd; ++p ) {
   if (MC::isHadron(*p)&&!MC::isBeam(*p)) return false;
   auto pv = (*p)->production_vertex();
   if (pv && !isPartonVertex(pv)) return false;
}
for ( auto  p = vtx->particles_out_const_begin(), parentEnd = vtx->particles_out_const_end(); p != parentEnd; ++p ) {
   if (MC::isHadron(*p)&&!MC::isBeam(*p)) return false;
 }
#endif 
 return true;
}

bool VtxBasedFilterTool::isFromHardScattering( const HepMC::ConstGenVertexPtr& vtx ) const
{
  return isPartonVertex(vtx) && m_ppFilter.isAccepted(vtx) && ! m_showerFilter.isAccepted(vtx);
}

/////////////////////////////////////////////////////////////////// 
/// Non-const methods: 
/////////////////////////////////////////////////////////////////// 

StatusCode VtxBasedFilterTool::initializeTool()
{
  ATH_MSG_INFO("Initializing " << name() << "...");

  // configure hard-scattering vertices
  m_ppFilter.setDecayPattern( "-1|-2|-3|-4|-5|-6|21 + 1|2|3|4|5|6|21 -> " );
  
  // filter which will be used to not select shower vertices while
  // looking for hard-scattering vertices
  m_showerFilter.setDecayPattern( "-> 91|92|94" );

  return StatusCode::SUCCESS;
}
