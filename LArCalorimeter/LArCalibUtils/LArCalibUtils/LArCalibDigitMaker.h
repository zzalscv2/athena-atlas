/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCALIBDIGITMAKER
#define LARCALIBDIGITMAKER
#include "AthenaBaseComps/AthAlgorithm.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "CaloIdentifier/LArEM_ID.h"
#include "LArRecConditions/LArCalibLineMapping.h"
#include <fstream>

class LArCalibDigitMaker : public AthAlgorithm
{
 public:
  LArCalibDigitMaker(const std::string & name, ISvcLocator * pSvcLocator);

  ~LArCalibDigitMaker();

  //standart algorithm methods
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize(){return StatusCode::SUCCESS;}
 private:
  SG::ReadCondHandleKey<LArCalibLineMapping> m_calibMapKey{this,"CalibMapKey","LArCalibLineMap","SG Key of calib line mapping object"};
  SG::ReadCondHandleKey<LArCalibLineMapping> m_calibMapSCKey{this,"CalibMapSCKey","LArCalibIdMapSC","SG Key of calib line mapping object"};
  std::vector<std::string> m_keylist;
  std::vector<std::vector<double>> m_vPattern;
  std::vector<std::vector<double>> m_vDAC;
  std::vector<unsigned> m_vDelay;
  std::vector<int> m_nPatterns;
  std::vector<std::vector<double>> m_vBoardIDs;
  unsigned m_nTrigger;
  double m_delayScale;
  bool m_dontRun;
  bool m_isSC;
  int m_eventNb = 0;
  bool m_skipDuplicates;
  std::vector<HWIdentifier> m_pulsedChids;
};

#endif
