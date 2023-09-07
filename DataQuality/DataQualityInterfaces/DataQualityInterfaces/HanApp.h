/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef dqiHanApp_h
#define dqiHanApp_h

#include <string>

#include <TObject.h>
#include "CxxUtils/checker_macros.h"

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;  // standalone application

class TFile;


namespace dqi {

class ATLAS_NOT_THREAD_SAFE HanApp : public TObject {
public:

  HanApp();
  HanApp( std::string configName_, std::string inputName_, std::string outputName_, std::string path_ = "" );
  virtual ~HanApp();
  
  virtual int Analyze( std::string configName_, std::string inputName_, std::string outputName_, std::string path_ = "" );
  
  virtual TFile*  OpenResultsFile() const;
  
  
protected:

  std::string  m_outputName;
  
  
//Get rid of Root macros that confuse Doxygen
///\cond CLASSDEF
  ClassDef( HanApp, 0 ) // Executes the histogram analyzer (han) utility
///\endcond

};

} // namespace dqi

#endif
