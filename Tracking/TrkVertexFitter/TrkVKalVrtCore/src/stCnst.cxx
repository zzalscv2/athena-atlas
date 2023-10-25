/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//    Management of constraints
//
//----------------------------------------------------
#include "TrkVKalVrtCore/stCnst.h"

#include <iostream>

#include "TrkVKalVrtCore/CommonPars.h"
#include "TrkVKalVrtCore/Derclc1.h"
#include "TrkVKalVrtCore/Derclc2.h"
#include "TrkVKalVrtCore/DerclcAng.h"
#include "TrkVKalVrtCore/Derivt.h"
#include "TrkVKalVrtCore/ForCFT.h"
#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"

namespace Trk {

void applyConstraints(VKVertex* vk) {
  int NCEntries = vk->ConstraintList.size();
  int NTRK = vk->TrackList.size();
  for (int ic = 0; ic < NCEntries; ic++) {
    vk->ConstraintList[ic]->applyConstraint();
  }
  //
  // Effect of symmetrization
  //
  for (int ii = 0; ii < (int)vk->ConstraintList.size(); ii++) {
    for (int ic = 0; ic < (int)vk->ConstraintList[ii]->NCDim; ic++) {
      vk->ConstraintList[ii]->h0t[ic].X *= 0.5;
      vk->ConstraintList[ii]->h0t[ic].Y *= 0.5;
      vk->ConstraintList[ii]->h0t[ic].Z *= 0.5;
      for (int it = 0; it < NTRK; it++) {
        vk->ConstraintList[ii]->f0t.at(it)[ic].X *= 0.5;
        vk->ConstraintList[ii]->f0t[it][ic].Y *= 0.5;
        vk->ConstraintList[ii]->f0t[it][ic].Z *= 0.5;
      }
    }
  }
}
}  // namespace Trk

