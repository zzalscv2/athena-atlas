/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DQM_ALGORITHMS_TRTWEIGHTEDAVERAGE_H
#define DQM_ALGORITHMS_TRTWEIGHTEDAVERAGE_H

#include <dqm_core/Algorithm.h>
#include <string>
#include <iosfwd>

namespace dqm_algorithms
{
	struct  TRTWeightedAverage : public dqm_core::Algorithm
        {
	  TRTWeightedAverage();

	  ~TRTWeightedAverage();

	    //overwrites virtual functions
	  TRTWeightedAverage * clone( );
	  dqm_core::Result * execute( const std::string & , const TObject & , const dqm_core::AlgorithmConfig & );
	  using dqm_core::Algorithm::printDescription;
	  virtual void  printDescription(std::ostream& out);
	};
}

#endif // DQM_ALGORITHMS_TRTWEIGHTEDAVERAGE_H
