//
//  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHEXUNITTEST_ATHEXALGWITHFPE
#define ATHEXUNITTEST_ATHEXALGWITHFPE 1

#include "AthenaBaseComps/AthAlgorithm.h"

class AthExAlgWithFPE: public AthAlgorithm { 
public: 

  using AthAlgorithm::AthAlgorithm; //Delgate c'tor

  StatusCode execute();

};

#endif
