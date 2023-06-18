/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */


#ifndef FPGATrackSimSGInput_TRIGFPGATrackSimINPUTUTILS_H
#define FPGATrackSimSGInput_TRIGFPGATrackSimINPUTUTILS_H

#include "AtlasHepMC/GenParticle_fwd.h"
#include <bitset>


namespace FPGATrackSimInputUtils {
  
  typedef enum { TAU_PARENT_BIT , B_PARENT_BIT , PION_PARENT_BIT , PION_IMMEDIATE_PARENT_BIT , NBITS } Bits;
  typedef std::bitset<NBITS> ParentBitmask;
  const ParentBitmask construct_truth_bitmap( HepMC::ConstGenParticlePtr p ) ;
}

#endif
