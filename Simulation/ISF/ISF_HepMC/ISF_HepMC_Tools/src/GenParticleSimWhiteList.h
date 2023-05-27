/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_HEPMC_GENPARTICLESIMWHITELIST_H
#define ISF_HEPMC_GENPARTICLESIMWHITELIST_H 1

// FrameWork includes
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "AthenaBaseComps/AthAlgTool.h"
// ISF includes
#include "ISF_HepMC_Interfaces/IGenParticleFilter.h"

// STL includes
#include <string>
#include <vector>

#include "AtlasHepMC/GenParticle.h"
namespace ISF {

  class ISFParticle;

  /** @class GenParticleSimWhiteList

      Stable/Interacting particle filter for HepMC particles to be used in the
      stack filling process.  Checks this particle and all daughters.

      @author ZLMarshall -at- lbl.gov
  */
  class GenParticleSimWhiteList : public extends<AthAlgTool, IGenParticleFilter> {

  public:
    //** Constructor with parameters */
    GenParticleSimWhiteList( const std::string& t, const std::string& n, const IInterface* p );

    /** Destructor */
    ~GenParticleSimWhiteList(){}

    /** Athena algtool's Hooks */
    virtual StatusCode  initialize() override final;
    virtual StatusCode  finalize() override final;

    /** passes through to the private version */
#ifdef HEPMC3
    virtual bool pass(const HepMC::ConstGenParticlePtr& particle ) const override;
#else
    virtual bool pass(const HepMC::GenParticle& particle ) const override;
#endif

  private:
    /** returns true if the the particle and all daughters are on the white list */
#ifdef HEPMC3
    bool pass(const HepMC::ConstGenParticlePtr& particle , std::vector<int> & used_vertices ) const;
#else
    bool pass(const HepMC::GenParticle& particle , std::vector<int> & used_vertices ) const;
#endif
    StringArrayProperty m_whiteLists{this, "WhiteLists", {"G4particle_whitelist.txt"} }; //!< The location of the white lists
    std::vector<long int>             m_pdgId;                //!< Allowed PDG IDs
    BooleanProperty m_qs{this, "QuasiStableSim", true}; //!< Switch for quasi-stable particle simulation
    DoubleProperty m_minDecayRadiusQS{this, "MinimumDecayRadiusQS", 30.19*Gaudi::Units::mm}; //!< Decay radius below which QS particles should be ignored
  };

}


#endif //> !ISF_HEPMC_GENPARTICLESIMWHITELIST_H
