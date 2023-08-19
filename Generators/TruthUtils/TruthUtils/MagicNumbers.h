/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/* Author: Andrii Verbytskyi andrii.verbytskyi@mpp.mpg.de */

#ifndef ATLASHEPMC_MAGICNUMBERS_H
#define ATLASHEPMC_MAGICNUMBERS_H

#include <limits> 
#include <cstdint> 
#include <memory>
#include <deque>
#include <type_traits>
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

constexpr int SIM_STATUS_INCREMENT = 100000;
constexpr int SIM_STATUS_THRESHOLD = 20000;

constexpr int PARTONPDGMAX = 43;
constexpr int NPPDGMIN = 1000000;
constexpr int NPPDGMAX = 8999999;
/// @brief Constant defining the barcode threshold for particles from PHOTOS
constexpr int PHOTOSMIN = 10000;

/// @brief Constant that the meaning of which is currently lost, to be recovered...   
constexpr int SPECIALSTATUS = 902;
constexpr int EVTGENUNDECAYEDSTATUS = 899;
constexpr int PYTHIA8LHESTATUS = 1003;
constexpr int HERWIG7INTERMEDIATESTATUS = 11;
constexpr int PYTHIA8NOENDVERTEXSTATUS = 201;
constexpr int FORWARDTRANSPORTMODELSTATUS = 212;

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


template <class T1,class T2, std::enable_if_t< !std::is_arithmetic<T1>::value &&  !std::is_arithmetic<T2>::value, bool> = true >
inline bool is_same_generator_particle(const T1& p1,const T2& p2) { return p1 && p2 && (p1->barcode() % SIM_REGENERATION_INCREMENT == p2->barcode() % SIM_REGENERATION_INCREMENT); }

template <class T1,class T2, std::enable_if_t< !std::is_arithmetic<T1>::value &&  std::is_arithmetic<T2>::value, bool> = true >
inline bool is_same_generator_particle(const T1& p1,const T2& p2) { return p1 && (p1->barcode() % SIM_REGENERATION_INCREMENT == p2 % SIM_REGENERATION_INCREMENT); }

template <class T1,class T2, std::enable_if_t< std::is_arithmetic<T1>::value &&  std::is_arithmetic<T2>::value, bool> = true >
inline bool is_same_generator_particle(const T1& p1,const T2& p2) { return p1 % SIM_REGENERATION_INCREMENT == p2 % SIM_REGENERATION_INCREMENT; }

/// @brief Method to check if the first particle is a descendant of the second in the simulation, i.e. particle p1 was produced simulations particle p2.
template <class T1,class T2, std::enable_if_t< !std::is_arithmetic<T1>::value &&  !std::is_arithmetic<T2>::value, bool> = true >
inline bool is_sim_descendant(const T1& p1,const T2& p2) { return p1 && p2 && (p1->barcode() % SIM_REGENERATION_INCREMENT == p2->barcode());}

template <class T1,class T2, std::enable_if_t< std::is_arithmetic<T1>::value &&  !std::is_arithmetic<T2>::value, bool> = true >
inline bool is_sim_descendant(const T1& p1,const T2& p2) { return p2 && (p1 % SIM_REGENERATION_INCREMENT == p2->barcode()); }

template <class T> int  unique_id(const T& p1){ return p1->barcode();}
#if  defined(HEPMC3) && !defined(XAOD_STANDALONE)
template <>  inline int unique_id(const ConstGenParticlePtr& p1){ return p1->id();}
template <>  inline int unique_id(const GenParticlePtr& p1){ return p1->id();}
#endif


template <class T> inline void get_particle_history(const T& p, std::deque<int>& out, int direction=0) {
  if (direction < 0) {
    if (p->status()>SIM_STATUS_INCREMENT) {
      auto pv = p->production_vertex();
      if (pv) {
        for (auto pa: pv->particles_in()) {
          if (pa->pdg_id() != p->pdg_id()) continue;
          out.push_front(unique_id(p));
          get_particle_history(pa,out,-1);
          break;
        }
      }
    }
  }
  if (direction > 0) {
    if (p->status()>SIM_STATUS_INCREMENT) {
      auto pv = p->end_vertex();
      if (pv) {
        for (auto pa: pv->particles_out()) {
          if (pa->pdg_id() != p->pdg_id()) continue;
          out.push_back(unique_id(p));
          get_particle_history(pa,out,1);
          break;
        }
      }
    }
  }
}
template <class T>  inline std::deque<int> simulation_history(const T& p, int direction ) { std::deque<int> res; res.push_back(unique_id(p)); get_particle_history(p, res, direction); return res;}

/// @brief Function that converts the old scheme of labeling the simulation particles (barcodes) into the new scheme (statuses).

template <class T> void old_to_new_simulation_scheme(T& evt) {
#ifdef HEPMC3
  for (auto p: evt->particles())  p->set_status( SIM_STATUS_INCREMENT * ( HepMC::barcode(p) / SIM_REGENERATION_INCREMENT)  + (  HepMC::barcode(p) % SIM_REGENERATION_INCREMENT > SIM_BARCODE_THRESHOLD ) ? 0 : SIM_STATUS_THRESHOLD + p->status());
  for (auto v: evt->vertices())   v->set_status( ( -HepMC::barcode(v)> SIM_BARCODE_THRESHOLD ) ? 0 : SIM_STATUS_THRESHOLD + v->status());
#else
  for (auto p = evt->particles_begin(); p != evt->particles_end(); ++p)  (*p)->set_status(SIM_STATUS_INCREMENT *(  (*p)->barcode() / SIM_REGENERATION_INCREMENT)  + ( (*p)->barcode() % SIM_REGENERATION_INCREMENT > SIM_BARCODE_THRESHOLD ) ? 0 : SIM_STATUS_THRESHOLD + (*p)->status());
  for (auto v = evt->vertices_begin(); v != evt->vertices_end(); ++v)    (*v)->set_id( ( -(*v)->barcode() > SIM_BARCODE_THRESHOLD ) ? 0 : SIM_STATUS_THRESHOLD + (*v)->id());
#endif
}


}
#endif
