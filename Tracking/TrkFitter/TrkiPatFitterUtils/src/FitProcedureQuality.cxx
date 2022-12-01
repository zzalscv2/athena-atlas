/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 quality variables produced during fit procedure
 ***************************************************************************/

#include "TrkiPatFitterUtils/FitProcedureQuality.h"

#include <iomanip>
#include <iostream>

#include "GaudiKernel/MsgStream.h"

namespace Trk {

void FitProcedureQuality::print(MsgStream& log) const {
  log << std::setiosflags(std::ios::fixed | std::ios::right) << std::setw(3)
      << m_iterations << " iter" << std::setw(4) << m_numberScatterers
      << " scat" << std::setw(2) << m_numberAlignments << " ali" << std::setw(4)
      << m_numberDoF << " DoF" << std::setw(8) << std::setprecision(4)
      << m_fitProbability << " prob" << std::setw(6) << std::setprecision(1)
      << m_chiSq << " chiSq";
}

}  // namespace Trk
