/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file TruthD3PDAnalysis/src/TruthParticleFilterBase.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2011
 * @brief Filter truth particles for writing to D3PD: base class.
 */


#include "TruthD3PDAnalysis/TruthParticleFilterBase.h"
#include "GeneratorObjects/McEventCollection.h"
#include "McParticleEvent/TruthEtIsolationsContainer.h"
#include "McParticleKernel/ITruthIsolationTool.h"
#include "AthenaKernel/errorcheck.h"
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenParticle.h"
#include <utility>


namespace D3PD {


/**
 * @brief Standard Gaudi tool constructor.
 * @param type The name of the tool type.
 * @param name The tool name.
 * @param parent The tool's Gaudi parent.
 */
TruthParticleFilterBase::TruthParticleFilterBase
  (const std::string& type,
   const std::string& name,
   const IInterface* parent)
    : AthAlgTool (type, name, parent),
      m_resolver (name, evtStore(), m_mcEventsName)
{
  declareProperty( "McEvents",       
		   m_mcEventsName = "GEN_AOD",
		   "Name of the input McEventCollection we want to filter" );

  declareProperty( "McEventsOutput", 
		   m_mcEventsOutputName = "GEN_D3PD",
		   "Name of the output McEventCollection which has been filtered" );

  declareProperty( "DoPileup",
                   m_doPileup = false,
                   "If true, include particles from pileup/cavern.");

  declareProperty ("RemoveEmpty",
                   m_removeEmpty = true,
                   "If true, remove empty GenEvent structures.");

  declareProperty ("DoEtIsolations",
                   m_doEtIsolations = false,
		   "Switch to compute or not the Et-isolations for TruthParticle "
		   "(and their underlying @c HepMC::GenParticle).\n" 
		   "Default is to not compute these Et-isolations (and save CPU)." );

  declareProperty( "TruthIsolationTool",
		   m_isolationTool = IsolTool_t( "TruthIsolationTool" ),
		   "Pointer to the TruthIsolationTool to be able to compute "
		   "transverse energy isolations for various isolation cones cuts. " 
		   "See McParticleEvent/TruthParticleParameters.h for cone cuts." );

  declareInterface<ITruthParticleFilterTool>(this);
}


/**
 * @brief Standard Gaudi initialize method.
 */
StatusCode TruthParticleFilterBase::initialize()
{
  CHECK( m_resolver.initialize<McEventCollection>() );
  if (m_doEtIsolations)
    CHECK( m_isolationTool.retrieve() );
  return AthAlgTool::initialize();
}


/**
 * @brief Standard Gaudi finalize method.
 */
StatusCode TruthParticleFilterBase::finalize()
{
  return AthAlgTool::finalize();
}


/**
 * @brief Standard Gaudi execute method.
 */
StatusCode TruthParticleFilterBase::execute()
{
  // Fetch input collection.
  const McEventCollection* mc_in;
  CHECK( evtStore()->retrieve (mc_in, m_resolver.key()) );

  // Create output collection.
  McEventCollection* mc_out = new McEventCollection;
  CHECK( evtStore()->record (mc_out, m_mcEventsOutputName) );
  CHECK( evtStore()->setConst (mc_out) );

  // Compute isolation for gamma/lepton.
  if (m_doEtIsolations) {
    CHECK( m_isolationTool->buildEtIsolations(m_resolver.key()) );
  }

  // Copy and filter.
  CHECK( buildMcAod (mc_in, mc_out) );

  // We have slimmed the filterMcEvent.
  // To not bias the map of isolation energies for this GenEvent, we alias
  // it to its original one
  if (m_doEtIsolations) {
    CHECK( m_isolationTool->registerAlias( m_mcEventsOutputName,
                                           m_resolver.key()) );
  }

  return StatusCode::SUCCESS;
}


/**
 *  This method will check the validity of the input McEventCollection 
 *  and build a filtered one from the strategy implemented by the 
 *  concrete tool.
 *  It is not const to allow derived tools to build statistics during
 *  the filtering process.
 */
StatusCode
TruthParticleFilterBase::buildMcAod (const McEventCollection* mc_in,
                                     McEventCollection* mc_out)
{
  // Loop over GenEvent's.
  mc_out->reserve (mc_in->size());
  for (const HepMC::GenEvent* ev_in : *mc_in) {
    if (!ev_in) continue;
#ifdef HEPMC3
    HepMC::GenEvent* ev_out = HepMC::copyemptyGenEvent( ev_in);
    // Copy and filter the contents.
    CHECK( filterEvent (ev_in, ev_out) );
    // Maybe throw out empty GenEvent's.
    if (m_removeEmpty && ev_out->particles().empty())
      delete ev_out;
    else
      mc_out->push_back (ev_out);
    // If we don't want pileup, only do the first non-empty GenEvent.
    if (!m_doPileup && ev_in->particles().size() != 0)
      break;

#else
    // Copy the GenEvent.
    HepMC::GenEvent* ev_out = new HepMC::GenEvent (ev_in->signal_process_id(),
                                                   ev_in->event_number());
    ev_out->set_event_scale (ev_in->event_scale());
    ev_out->set_alphaQCD (ev_in->alphaQCD());
    ev_out->set_alphaQED (ev_in->alphaQED());
    ev_out->weights() = ev_in->weights();
    ev_out->set_random_states (ev_in->random_states());
    if (ev_in->heavy_ion())
      ev_out->set_heavy_ion (*ev_in->heavy_ion());
    if (ev_in->pdf_info())
      ev_out->set_pdf_info (*ev_in->pdf_info());

    // Copy and filter the contents.
    CHECK( filterEvent (ev_in, ev_out) );

    // Maybe throw out empty GenEvent's.
    if (m_removeEmpty && ev_out->particles_empty())
      delete ev_out;
    else
      mc_out->push_back (ev_out);

    // If we don't want pileup, only do the first non-empty GenEvent.
    if (!m_doPileup && ev_in->particles_size() != 0)
      break;
#endif
  }
  return StatusCode::SUCCESS;
}


/**
 * @brief Filter a single @c GenEvent.
 */
StatusCode
TruthParticleFilterBase::filterEvent (const HepMC::GenEvent* ev_in, HepMC::GenEvent* ev_out)
{
  m_used.clear();
  // Loop over particles.
   for (const auto& ip: *ev_in)
  {
    // Copy the particle if we want to keep it.
    if (isAccepted (ip))
      CHECK( addParticle (ip, ev_out) );
  }
  return StatusCode::SUCCESS;
}


/**
 * @brief Add a @c GenParticle (and its production vertex) to a @c GenEvent.
 */
StatusCode
TruthParticleFilterBase::addParticle (const HepMC::ConstGenParticlePtr& p, HepMC::GenEvent* ev)
{
  // Add parent vertex if it exists.  Otherwise, add decay vertex.
  if (p->production_vertex())
    CHECK( addVertex (p->production_vertex(), ev) );
  else if (p->end_vertex())
    CHECK( addVertex (p->end_vertex(), ev) );
  else {
    REPORT_MESSAGE (MSG::ERROR) << "Encountered GenParticle with no vertices!";
    return StatusCode::FAILURE;
  }
#ifdef HEPMC3
  // Find if the particle was used.
  HepMC::GenParticlePtr pnew = nullptr;
  if (p && m_used.count(p->id()) == 0) pnew = std::make_shared<HepMC::GenParticle>(*p);
  // Add ourself to our vertices.
  if (p && p->production_vertex() && m_used.count(p->production_vertex()->id()) == 0) {
    HepMC::GenVertexPtr v = std::make_shared<HepMC::GenVertex>(*p->production_vertex());
    ev -> add_vertex(v);
    m_used.insert(p->production_vertex()->id());
    v->add_particle_out (pnew);
    m_used.insert(p->id());
    HepMC::suggest_barcode (v,HepMC::barcode(p->production_vertex()));
    HepMC::suggest_barcode (pnew,HepMC::barcode(p));
  }
  if (p && p->end_vertex() && m_used.count(p->end_vertex()->id()) == 0 ) {
    HepMC::GenVertexPtr v =  std::make_shared<HepMC::GenVertex>(*p->end_vertex());
    ev -> add_vertex(v);
    m_used.insert(p->end_vertex()->id());
    v->add_particle_in (pnew);
    m_used.insert(p->id());
    HepMC::suggest_barcode (v,HepMC::barcode(p->end_vertex()));
    HepMC::suggest_barcode (pnew,HepMC::barcode(p));
  }

#else
  // Find the particle in the event.
  // If it doesn't exist yet, copy it.
  HepMC::GenParticle* pnew = nullptr; 
  if (m_used.count(p->barcode()) == 0 && (p->production_vertex() || p->end_vertex()))
    pnew = new HepMC::GenParticle (*p);

  // Add ourself to our vertices.
  if (p->production_vertex() &&  m_used.count(p->production_vertex()->barcode()) == 0 ) {
    HepMC::GenVertex* v = new HepMC::GenVertex (*p->production_vertex()); 
    ev->add_vertex(v);
    v->add_particle_out (pnew);
    m_used.insert(p->production_vertex()->barcode());
    m_used.insert(p->barcode());
    HepMC::suggest_barcode (v,HepMC::barcode(p->production_vertex()));
    HepMC::suggest_barcode (pnew,HepMC::barcode(p));
  }

  if (p->end_vertex() &&  m_used.count(p->end_vertex()->barcode()) == 0 ) {
    HepMC::GenVertex* v = new HepMC::GenVertex (*p->end_vertex()); 
    ev->add_vertex(v);
    v->add_particle_in (pnew);
    m_used.insert(p->end_vertex()->barcode());
    m_used.insert(p->barcode());
    HepMC::suggest_barcode (v,HepMC::barcode(p->end_vertex()));
    HepMC::suggest_barcode (pnew,HepMC::barcode(p));
  }
#endif

  return StatusCode::SUCCESS;
}


/**
 * @brief Add a @c GenVertex to a @c GenEvent.
 */
StatusCode
TruthParticleFilterBase::addVertex (const HepMC::ConstGenVertexPtr& v, HepMC::GenEvent* ev)
{
#ifdef HEPMC3
  // See if this vertex has already been copied.
  if (m_used.count(v->id()) == 0)
   {
    // No ... make a new one.
    HepMC::GenVertexPtr vnew = HepMC::newGenVertexPtr();
    ev->add_vertex (vnew);
    vnew->set_position (v->position());
    vnew->set_status (v->status());
    HepMC::suggest_barcode (vnew,HepMC::barcode(v));
    m_used.insert(v->id());
//AV: Are these needed?FIXME?    vnew->weights() = v->weights();
    // Fill in the existing relations of the new vertex.
    for (const auto&  p : v->particles_in())
    {
      if (m_used.count(p->id()) !=  0) continue;
      auto pnew = std::make_shared<HepMC::GenParticle>(*p);
      vnew->add_particle_in (pnew);
      m_used.insert(p->id());
      HepMC::suggest_barcode (pnew,HepMC::barcode(p));    }
    
   for (const auto&  p :v->particles_out())
    {
      if (m_used.count(p->id()) !=  0) continue;
      auto pnew = std::make_shared<HepMC::GenParticle>(*p);
      vnew->add_particle_out (pnew);
      m_used.insert(p->id());
      HepMC::suggest_barcode (pnew,HepMC::barcode(p));
    }
  }

#else
  // See if this vertex has already been copied.
  if (m_used.count(HepMC::barcode(v)) == 0)
   {
    // No ... make a new one.
    HepMC::GenVertex* vnew = new HepMC::GenVertex();
    vnew->set_position (v->position());
    vnew->set_id (v->id());
    vnew->weights() = v->weights();
    ev->add_vertex (vnew);
    m_used.insert(HepMC::barcode(v));
    HepMC::suggest_barcode (vnew,HepMC::barcode(v));    

    // Fill in the existing relations of the new vertex.
    for (auto ip=v->particles_in_const_begin();ip!=v->particles_in_const_end();++ip)
    {
      auto p = *ip;
      if (m_used.count(HepMC::barcode(p)) != 0) continue;
      HepMC::GenParticle* pnew = new HepMC::GenParticle(*p);
      vnew->add_particle_in (pnew);
      m_used.insert(HepMC::barcode(p));
      HepMC::suggest_barcode (pnew,HepMC::barcode(p));
    }
    for (auto ip = v->particles_out_const_begin();ip!= v->particles_out_const_end();++ip)
    {
      auto p = *ip;
      if (m_used.count(HepMC::barcode(p)) != 0) continue;
      HepMC::GenParticle* pnew = new HepMC::GenParticle(*p);
      vnew->add_particle_out (pnew);
      m_used.insert(HepMC::barcode(p));
      HepMC::suggest_barcode (pnew,HepMC::barcode(p));
    }
  }
#endif

  return StatusCode::SUCCESS;
}


/**
 * @brief Test to see if we want to keep a particle.
 */
bool
TruthParticleFilterBase::isAccepted (const HepMC::ConstGenParticlePtr& /*p*/)
{
  // Default implementation accepts everything.
  return true;
}


/**
 * @brief Return the container of isolation energies that we built.
 */
StatusCode
TruthParticleFilterBase::isolations (TruthEtIsolationsContainer const* &isocont)
{
  if (!m_doEtIsolations) {
    REPORT_MESSAGE(MSG::ERROR) << "Isolations were not computed.";
    return StatusCode::FAILURE;
  }

  std::string isoname = m_isolationTool->etIsolationsName (m_mcEventsOutputName);

  CHECK( evtStore()->retrieve (isocont, isoname) );

  return StatusCode::SUCCESS;
}

} // namespace D3PD
