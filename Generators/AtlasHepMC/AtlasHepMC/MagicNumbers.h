/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/* Author: Andrii Verbytskyi andrii.verbytskyi@mpp.mpg.de */

#ifndef ATLASHEPMC_MAGICNUMBERS_H
#define ATLASHEPMC_MAGICNUMBERS_H

#include <limits> 
#include <cstdint> 
#include <memory>
#if  defined(HEPMC3) && !defined(XAOD_STANDALONE)
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#endif
namespace HepMC {

/// @brief Constant defining the barcode threshold for simulated particles, eg. can be used to separate generator event record entries from simulated particles
constexpr int SIM_BARCODE_THRESHOLD = 200000;

/// @brief Constant defining the barcode threshold for regenerated particles, eg. secondary simulated particles following an interaction
constexpr int SIM_REGENERATION_INCREMENT = 1000000;

constexpr int PARTONPDGMAX = 43;
constexpr int NPPDGMIN = 1000000;
constexpr int NPPDGMAX = 8999999;
/// @brief Constant defining the barcode threshold for particles from PHOTOS
constexpr int PHOTOSMIN = 10000;

/// @brief Constant that the meaning of which is currently lost, to be recovered...   
constexpr int SPECIALSTATUS = 10902;
constexpr int EVTGENUNDECAYEDSTATUS = 899;
constexpr int PYTHIA8LHESTATUS = 1003;
constexpr int HERWIG7INTERMEDIATESTATUS = 11;

/// @brief This barcode is used by objects matched to particles from pile-up interactions in standard MC Production
constexpr int crazyParticleBarcode(std::numeric_limits<int32_t>::max());

constexpr int INVALID_PARTICLE_BARCODE = -1;

/// @brief Method to establish if a particle (or barcode) was created during the simulation
template <class T>  inline bool is_simulation_particle(const T& p){ return (p->barcode()>SIM_BARCODE_THRESHOLD);}
template <>  inline bool is_simulation_particle(const int& b){ return (b>SIM_BARCODE_THRESHOLD);}
#if  defined(HEPMC3) && !defined(XAOD_STANDALONE)
template <>  inline bool is_simulation_particle(const ConstGenParticlePtr& p){ return (barcode(p)>SIM_BARCODE_THRESHOLD);}
template <>  inline bool is_simulation_particle(const GenParticlePtr& p){ return (barcode(p)>SIM_BARCODE_THRESHOLD);}
#endif

/// @brief Method to return how many interactions a particle has undergone during simulation.
template <class T>  inline int generations(const T& p){ return (p->barcode()/SIM_REGENERATION_INCREMENT);}
template <>  inline int generations(const int& b){ return (b/SIM_REGENERATION_INCREMENT);}
#if  defined(HEPMC3) && !defined(XAOD_STANDALONE)
template <>  inline int generations(const ConstGenParticlePtr& p){ return (barcode(p)/SIM_REGENERATION_INCREMENT);}
template <>  inline int generations(const GenParticlePtr& p){ return (barcode(p)/SIM_REGENERATION_INCREMENT);}
#endif

/// @brief Method to establish if the vertex was created during simulation 
template <class T>  inline bool is_simulation_vertex(const T& p){ return (p->barcode()<-SIM_BARCODE_THRESHOLD);}
template <>  inline bool is_simulation_vertex(const int& p){ return (p<-SIM_BARCODE_THRESHOLD);}
#if  defined(HEPMC3) && !defined(XAOD_STANDALONE)
template <>  inline bool is_simulation_vertex(const ConstGenVertexPtr& p){ return (barcode(p)<-SIM_BARCODE_THRESHOLD);}
template <>  inline bool is_simulation_vertex(const GenVertexPtr& p){ return (barcode(p)<-SIM_BARCODE_THRESHOLD);}
#endif

/// @brief Method to handle inconsistent definition for generator particles used in truth utils
template <class T>  inline bool is_truthhelper_generator_particle(const T& p) {
    return (!is_simulation_particle(p)) &&
           (p->status() < 200 ||
            p->status() % 1000 == 1 || p->status() % 1000 == 2 ||
            p->status() ==HepMC::SPECIALSTATUS); 
}
}
#endif
