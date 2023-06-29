/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: GenEventGenParticleAssociationTool.cxx 452268 2011-08-04 20:27:55Z ssnyder $
/**
 * @file EventCommonD3PDMaker/src/GenEventGenParticleAssociationTool.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2009
 * @brief Associate from a @c GenEvent to its contained particles.
 */


#include "GenEventGenParticleAssociationTool.h"
#include "AtlasHepMC/GenParticle.h"
#include "TruthUtils/MagicNumbers.h"
#include "AthenaKernel/errorcheck.h"


namespace D3PD {


/**
 * @brief Standard Gaudi tool constructor.
 * @param type The name of the tool type.
 * @param name The tool name.
 * @param parent The tool's Gaudi parent.
 */
GenEventGenParticleAssociationTool::GenEventGenParticleAssociationTool
  (const std::string& type,
   const std::string& name,
   const IInterface* parent)
    : Base (type, name, parent),
      m_haveSeenAHadron(false),
      m_firstHadronBarcode(0)
{
  declareProperty ("WritePartons",     m_doPartons  = false);
  declareProperty ("WriteHadrons",     m_doHadrons  = false);
  declareProperty ("WriteGeant",       m_doGeant    = false);
}


/**
 * @brief Start the iteration for a new association.
 * @param p The object from which to associate.
 */
StatusCode
GenEventGenParticleAssociationTool::reset (const HepMC::GenEvent& p)
{
#ifdef HEPMC3
  m_it = p.particles().begin();
  m_end = p.particles().end();
#else
  m_it = p.particles_begin();
  m_end = p.particles_end();
#endif

  m_haveSeenAHadron = false;

  m_firstHadronBarcode = 0;

  return StatusCode::SUCCESS;
}


/**
 * @brief Return a pointer to the next element in the association.
 *
 * Return null when the association has been exhausted.
 */
const HepMC::GenParticle* GenEventGenParticleAssociationTool::next()
{
  if (m_it == m_end)
    return nullptr;
  
  HepMC::ConstGenParticlePtr out;

  // loop until we see what we want
  bool ok = false;
  while( !ok ) {

    int pdg_id = std::abs ((*m_it)->pdg_id());
    int status = (*m_it)->status();
    int barcode = HepMC::barcode(*m_it);
    
    // are we at parton/hadron level?
    if ( status!=3 && pdg_id > HepMC::PARTONPDGMAX && !m_haveSeenAHadron ) {
      m_haveSeenAHadron = true;
      m_firstHadronBarcode = barcode;
    }

    // OK if we select partons and are at beginning of event record
    if( m_doPartons && !m_haveSeenAHadron )
      ok = true;

    //  OK if we should select hadrons and are in hadron range 
    if( m_doHadrons && m_haveSeenAHadron && barcode < HepMC::PHOTOSMIN )
      ok = true;
 
    // PHOTOS range: check whether photons come from parton range or 
    // hadron range
    int motherBarcode = 999999999;
    HepMC::ConstGenVertexPtr vprod = (*m_it)->production_vertex();
    if( barcode > HepMC::PHOTOSMIN && !HepMC::is_simulation_particle(barcode) && vprod ) {
#ifdef HEPMC3
      if (vprod->particles_in().size() > 0) {
	auto mother = vprod->particles_in().front();
	if (mother) 
	  motherBarcode = HepMC::barcode(mother);
      }
#else
      if (vprod->particles_in_size() > 0) {
	const HepMC::GenParticle* mother = *vprod->particles_in_const_begin();
	if (mother) 
	  motherBarcode = mother->barcode();
      }
#endif
      if( m_doPartons && motherBarcode < m_firstHadronBarcode )
	ok = true;
      if( m_doHadrons && motherBarcode >= m_firstHadronBarcode )
	ok = true;
    }

    // OK if we should select G4 particles and are in G4 range
    if( m_doGeant && HepMC::is_simulation_particle(barcode) )
      ok = true;

    out = *m_it;
    ++m_it;
    
    if (m_it == m_end)
      return nullptr;
  
  }
  
  // exit if we are at geant level and not supposed to write this out
  if(  !m_doGeant && HepMC::is_simulation_particle(out) )
    return nullptr;
#ifdef HEPMC3
  return out.get();
#else
  
  return out;
#endif
}


} // namespace D3PD
