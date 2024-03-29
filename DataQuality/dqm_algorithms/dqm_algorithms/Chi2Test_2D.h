/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/*! \file Chi2Test_2D.h file declares the dqm_algorithms::Chi2Test_2D  class.
 * \author Ian Moult
*/

#ifndef DQM_ALGORITHMS_CHI2TEST_2D_H
#define DQM_ALGORITHMS_CHI2TEST_2D_H

#include "CxxUtils/checker_macros.h"

#include <dqm_core/Algorithm.h>
#include <string>
#include <iosfwd>

namespace dqm_algorithms
{
	struct ATLAS_NOT_THREAD_SAFE Chi2Test_2D : public dqm_core::Algorithm
	//     ^ calls Sumw2 on const histogram
        {
	    Chi2Test_2D ();

	    Chi2Test_2D * clone( );
	    dqm_core::Result * execute( const std::string & , const TObject & , const dqm_core::AlgorithmConfig & );
            using dqm_core::Algorithm::printDescription;
            void  printDescription(std::ostream& out);           
	};
}

#endif // DQM_ALGORITHMS_CHI2TEST_2D_H
