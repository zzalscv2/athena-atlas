/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/ServiceHandle.h"

// Data members classes
#include <list>
class StoreGateSvc;
class PileUpMergeSvc;
 

/////////////////////////////////////////////////////////////////////////////

class MyAlg:public Algorithm {
public:
MyAlg (const std::string& name, ISvcLocator* pSvcLocator);
StatusCode initialize();
StatusCode execute();
StatusCode finalize();

//ServiceHandle<StoreGateSvc> p_overStore; 

 PileUpMergeSvc *m_mergeSvc; // Pile up service

};
 
