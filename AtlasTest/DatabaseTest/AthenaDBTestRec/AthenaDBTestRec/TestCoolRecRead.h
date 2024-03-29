/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENADBTESTREC_TESTCOOLRECREAD_H
#define ATHENADBTESTREC_TESTCOOLRECREAD_H

#include <string>
#include <vector>
#include <fstream>
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/DataHandle.h"
#include "AthenaDBTestRec/TestCoolRecFolder.h"
#include "CxxUtils/checker_macros.h"

class IOVTime;

class ATLAS_NOT_THREAD_SAFE TestCoolRecRead : public AthAlgorithm
{
 public:
  TestCoolRecRead(const std::string& name, ISvcLocator* pSvcLocator);
  ~TestCoolRecRead();

  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

 private:
  int readAuxFiles();
  // parameters
  std::vector<std::string> m_folders;
  std::vector<int> m_ftypes;
  std::vector<std::string> m_auxfiles;
  bool m_par_checkmatch;
  int m_par_delay;
  int m_par_dumpchan;
  std::string m_par_dumpfile;

  StoreGateSvc* p_detstore;
  std::vector<TestCoolRecFolder> m_folderlist;
  int m_nbadaux;
  std::ofstream* m_dumpf;
};

#endif // ATHENADBTESTREC_TESTCOOLRECREAD_H
