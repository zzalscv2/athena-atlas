/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////
//                                                         //
//  Header file for class VP1Alg                           //
//                                                         //
//  update: Riccardo-Maria BIANCHI <rbianchi@cern.ch>      //
//          23 May 2014                                    //
//                                                         //
//  This is the Athena algorithm starting the production   //
//  of event files for VP1 Live, the online 3D event       //
//  display at P1.                                         //
//                                                         //
/////////////////////////////////////////////////////////////

#ifndef VP1ALGS_VP1EVENTPROD
#define VP1ALGS_VP1EVENTPROD

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/IIncidentListener.h"
#include "PoolSvc/IPoolSvc.h"
#include <string>

class StoreGateSvc;
class VP1FileUtilities;

class VP1EventProd: public AthAlgorithm,
		    public IIncidentListener
{
 public:
  VP1EventProd(const std::string& name, ISvcLocator* pSvcLocator);
  ~VP1EventProd();

  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

  void handle(const Incident& inc);

 private:
  VP1FileUtilities* m_fileUtil;
 
  // run/event number to be used in the vp1 event file name
  int m_runNumber;
  int m_eventNumber;
  unsigned int m_timeStamp;

  // properties
  std::string m_inputPoolFile;
  std::string m_destinationDir;
  int m_maxProducedFiles;

  // service handle
  ServiceHandle<IPoolSvc> m_poolSvc;
};

#endif
