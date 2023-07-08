/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "SimpleTruthParticleFilterTool.h"
#include "AthenaKernel/errorcheck.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "TruthUtils/MagicNumbers.h"
#include "TruthUtils/HepMCHelpers.h"
#include "GaudiKernel/SystemOfUnits.h"

namespace D3PD {

using Gaudi::Units::GeV;

/**
 * @brief Standard Gaudi tool constructor.
 * @param type The name of the tool type.
 * @param name The tool name.
 * @param parent The tool's Gaudi parent.
 */
SimpleTruthParticleFilterTool::SimpleTruthParticleFilterTool
  (const std::string& type,
   const std::string& name,
   const IInterface* parent)
    : TruthParticleFilterBase (type, name, parent)
{
  declareProperty ("PDG_ID" ,       m_filterID    = 11 , "PDG ID to filter on");
  declareProperty ("MinPt",         m_minPt       = -1*GeV , "Min pT to filter on");
}


/**
 * @brief Test to see if we want to keep a particle.
 */
bool
SimpleTruthParticleFilterTool::isAccepted (const HepMC::ConstGenParticlePtr& p)
{
  bool ok = false;

  // First, the super simple thing : does the particle pass the simple cuts?
  if ( std::abs(p->pdg_id())==m_filterID &&
       p->momentum().perp()>m_minPt ) ok = true;

  bool last = std::abs(p->pdg_id())==15;
  if ( std::abs(p->pdg_id())==15 && p->status()!=1 && p->end_vertex() ){
    // Special handling for taus - take the ones that are last in the tau chain
#ifdef HEPMC3
    for (const auto& pit: p->end_vertex()->particles_out()){
      if (!pit || std::abs(pit->pdg_id())!=15) continue;
      last=false;
      break;
    }
#else
    for (HepMC::GenVertex::particles_out_const_iterator pit=p->end_vertex()->particles_out_const_begin(); pit!=p->end_vertex()->particles_out_const_end();++pit){
      if (!(*pit) ||
          std::abs((*pit)->pdg_id())!=15) continue;
      last=false;
      break;
    }
#endif    
    if (!last) return false;
  }

  if ( !last && // is it the last tau? (not a tau or not last -> last=false )
       p->status() != 1 &&
       !(p->status()==2 && (!p->end_vertex() || HepMC::is_simulation_vertex(p->end_vertex())) ) ) {
    return false;
  }

  // Cheating.  If we ask for 94 or 95, use those for heavy flavor hadrons
  if (m_filterID==94 && MC::hasCharm  (p->pdg_id()) && p->momentum().perp()>m_minPt) ok=true;
  if (m_filterID==95 && MC::hasBottom (p->pdg_id()) && p->momentum().perp()>m_minPt) ok=true;

  return ok;
}


} // namespace D3PD
