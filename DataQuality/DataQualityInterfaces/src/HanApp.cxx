/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// **********************************************************************
// $Id: HanApp.cxx,v 1.13 2008-12-07 03:25:37 ponyisi Exp $
// **********************************************************************

#include "DataQualityInterfaces/HanApp.h"

#include <TFile.h>
#include <TROOT.h>

#include "dqm_core/LibraryManager.h"
#include "dqm_core/InputRootFile.h"
#include "DataQualityInterfaces/HanConfig.h"
#include "DataQualityInterfaces/HanOutput.h"
#include "DataQualityInterfaces/HanInputRootFile.h"
#include "DataQualityInterfaces/HanRuntimeConfigSingleton.h"
#include "DataQualityInterfaces/HanUtils.h"

#include <TCanvas.h>

#include <utility>

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;  // standalone application

//Get rid of Root macros that confuse Doxygen
///\cond CLASSIMP
ClassImp(dqi::HanApp)
///\endcond

  /*
void HanApp_dummyMethod()
{
  TCanvas* dummyCanvas = new TCanvas("dummyCanvas","dummyCanvas", 450, 450 );
  dummyCanvas->cd();
}
  */

namespace {

  void RecursivelyDeleteCollection(TCollection* seq) {
    TIter it(seq);
    while (TObject* obj = it()) {
      TCollection* coll = dynamic_cast<TCollection*>(obj);
      if (coll) {
	RecursivelyDeleteCollection(coll);
      }
    }
    seq->Delete();
  }

} // end unnamed namespace


namespace dqi {

// *********************************************************************
// Public Methods
// *********************************************************************

HanApp::
HanApp()
  : m_outputName("")
{
}


HanApp::
HanApp( const std::string& configName, const std::string& inputName, const std::string& outputName, const std::string& path )
{
  HanApp::Analyze( configName, inputName, outputName, path );
}


HanApp::
~HanApp()
{
}


int
HanApp::
Analyze( const std::string& configName_, const std::string& inputName_, const std::string& outputName_, const std::string& path_ )
{
  DisableMustClean disabled;
  
  auto& runtimeConfig = HanRuntimeConfigSingleton::getInstance();
  runtimeConfig.setPath( path_ );
  
  HanOutput::DQOutputMap_t * outputMap = new HanOutput::DQOutputMap_t();
  TSeqCollection *outputList = new TList();

  m_outputName = outputName_;
  
  //dqm_core::InputRootFile	input( inputName_ );
  HanInputRootFile input( inputName_, path_ );				// HanInputRootFile inherits from dqm_core::InputRootFile

  HanOutput output( outputName_, outputMap, outputList );
  output.setInput(const_cast<TDirectory*>(input.getBasedir()));
  
  dqm_core::LibraryManager::instance().loadLibrary( "libdqm_algorithms.so" );
  dqm_core::LibraryManager::instance().loadLibrary( "libdqm_summaries.so" );
  
  HanConfig config;
  std::cout << "Reading configuration and input histograms...\n" << std::flush;
  config.BuildMonitors( configName_, input, output );
  std::cout << "Preparing output...\n" << std::flush;
  config.BuildConfigOutput( configName_, input.file(), path_, outputMap, outputList );
  
  std::cout << "Analyzing histograms...\n" << std::flush;
  output.activate();
  input.activate();
  output.publishMissingDQPars();

  std::cout << "Writing output file...\n" << std::flush;
  output.deactivate(); // essential for the HanOutput object since it writes the output file
  
  std::cout << "Deleting objects from memory.\n" << std::flush;
  RecursivelyDeleteCollection(outputList);
  delete outputList;
  delete outputMap;
  return 0;
}


TFile*
HanApp::
OpenResultsFile() const
{
  if( m_outputName == "" )
    return 0;
  
  return TFile::Open( m_outputName.c_str() );
}

} // namespace dqi

