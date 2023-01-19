/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGGEPPERF_ICELLSPRODUCER_H
#define TRIGGEPPERF_ICELLSPRODUCER_H

#include "GaudiKernel/IAlgTool.h"
#include "Identifier/Identifier.h"

#include "GaudiKernel/EventContext.h"

class CaloCell;

class ICaloCellsProducer : virtual public IAlgTool {
  /** PABC (Pure Abstract Base Class) for CaloCellProducers, which are
      AlgTools used by GepPi0Alg. */

public:
  DeclareInterfaceID(ICaloCellsProducer, 1, 0);
  virtual ~ICaloCellsProducer(){};
  
  
  /** obtain a vector of vectors of CaloCells. The outer
      vector may be a single entry containing selected cells as
      is the case of reading cells and selecting a sampling layer,
      or it may have a number of entries, as would occur if the
      cells from clusters were being returned, with each entry
      corresponding to a different cluster. */
  
  virtual StatusCode cells(std::vector<std::vector<const CaloCell*>>&,
			   const EventContext&) const = 0;
};
#endif
