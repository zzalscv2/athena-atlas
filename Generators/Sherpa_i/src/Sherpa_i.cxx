/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "HepMC/GenEvent.h"
#include "GaudiKernel/MsgStream.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"

#include "Sherpa_i/Sherpa_i.h"

//needed from Sherpa EvtGenerator
#include "SHERPA/Main/Sherpa.H"
#include "SHERPA/Initialization/Initialization_Handler.H"
#ifdef IS_SHERPA_3
#include "ATOOLS/Phys/Variations.H"
#else
#include "SHERPA/Tools/Variations.H"
#endif
#include "ATOOLS/Org/Exception.H"
#include "ATOOLS/Org/Run_Parameter.H"

#include <cstdio>
#include <cstring>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "CLHEP/Random/RandFlat.h"
#include "AthenaKernel/IAtRndmGenSvc.h"

CLHEP::HepRandomEngine* p_rndEngine;

Sherpa_i::Sherpa_i(const std::string& name, ISvcLocator* pSvcLocator) 
  : GenModule(name, pSvcLocator), p_sherpa(NULL)
{
  #ifdef IS_SHERPA_3
  declareProperty("BaseFragment", m_inputfiles["Base.yaml"] = "");
  declareProperty("RunCard", m_inputfiles["Sherpa.yaml"] = "");
  #else
  declareProperty("RunCard", m_runcard = "");
  declareProperty("Parameters", m_params);
  #endif
  declareProperty("OpenLoopsLibs", m_openloopslibs);
  declareProperty("ExtraFiles", m_extrafiles);
  declareProperty("NCores", m_ncores=1);
  declareProperty("MemoryMB", m_memorymb=2500.);
  declareProperty("PluginCode", m_plugincode = "");

  declareProperty("VariationWeightCap", m_variation_weight_cap=10.0);
  declareProperty("CrossSectionScaleFactor", m_xsscale=1.0);
  declareProperty("CleanupGeneratedFiles", m_cleanup=true);
}



StatusCode Sherpa_i::genInitialize(){
  ATH_MSG_INFO("Sherpa initialising... ");

  #ifdef IS_SHERPA_3
  for(auto& inputfile : m_inputfiles) {
    // remove first line and last character containing '"'
    // TODO fix Python/C++ string passing, to not contain " in first place
    inputfile.second.erase(0, inputfile.second.find("\n") + 1);
    inputfile.second.pop_back();
  }
  #endif

  ATH_MSG_DEBUG("... compiling plugin code");
  if (m_plugincode != "") {
    compilePlugin(m_plugincode);
    #ifdef IS_SHERPA_3
    m_inputfiles["Base.yaml"] += "SHERPA_LDADD: Sherpa_iPlugin \n";
    #else
    m_params.push_back("SHERPA_LDADD=Sherpa_iPlugin");
    #endif
  }

  ATH_MSG_DEBUG("... seeding Athena random number generator");
  p_rndEngine = atRndmGenSvc().GetEngine("SHERPA");
  const long* sip = p_rndEngine->getSeeds();
  long int si1 = sip[0];
  long int si2 = sip[1];
  atRndmGenSvc().CreateStream(si1, si2, "SHERPA");

  #ifdef IS_SHERPA_3
  ATH_MSG_DEBUG("... adapting output level");
  if( msg().level()==MSG::FATAL || msg().level()==MSG::ERROR || msg().level()==MSG::WARNING ){
    m_inputfiles["Base.yaml"] += "EVT_OUTPUT: 0 \n";
  }
  else if(msg().level()==MSG::INFO){
    m_inputfiles["Base.yaml"] += "EVT_OUTPUT: 2 \n";
  }
  else if(msg().level()==MSG::DEBUG){
    m_inputfiles["Base.yaml"] += "EVT_OUTPUT: 15 \n";
  }
  else{
    m_inputfiles["Base.yaml"] += "EVT_OUTPUT: 15 \n";
  }

  ATH_MSG_DEBUG("... writing input files to directory");
  for(auto& inputfile : m_inputfiles) {
    // write input content to file in working directory
    FILE *file = fopen(inputfile.first.c_str(),"w");
    fputs(inputfile.second.c_str(),file);
    fclose(file);
    ATH_MSG_INFO("Sherpa_i using the following settings in "+inputfile.first);
    ATH_MSG_INFO("\n"+inputfile.second+"\n");
  }

  ATH_MSG_DEBUG("... go Sherpa!");
  int argc = 2;
  char** argv = new char*[2];
  argv[0] = new char[7];
  argv[1] = new char[34];
  strcpy(argv[0], "Sherpa");
  strcpy(argv[1], "RUNDATA: [Base.yaml, Sherpa.yaml]");
  p_sherpa = new SHERPA::Sherpa(argc, argv);
  delete [] argv;
  #else
  p_sherpa = new SHERPA::Sherpa();
  #endif


  /***
      translate ATOOLS:SignalHandler
      TODO: is this even needed?
  ***/
  std::set_terminate(ATOOLS::Terminate);
  std::set_unexpected(ATOOLS::Terminate);
  #ifdef IS_SHERPA_3
  signal(SIGSEGV,ATOOLS::HandleSignal);
  signal(SIGINT,ATOOLS::HandleSignal);
  signal(SIGPIPE,ATOOLS::HandleSignal);
  signal(SIGBUS,ATOOLS::HandleSignal);
  signal(SIGFPE,ATOOLS::HandleSignal);
  signal(SIGABRT,ATOOLS::HandleSignal);
  signal(SIGTERM,ATOOLS::HandleSignal);
  signal(SIGXCPU,ATOOLS::HandleSignal);
  signal(SIGUSR1,ATOOLS::HandleSignal);

  try {
    p_sherpa->InitializeTheRun();
    p_sherpa->InitializeTheEventHandler();
  }
  catch (const ATOOLS::normal_exit& exception) {
    ATH_MSG_ERROR("Normal exit caught, this probably means:");
    ATH_MSG_ERROR("Have to compile Amegic libraries");
    ATH_MSG_ERROR("You probably want to run ./makelibs");
    return StatusCode::FAILURE;
  }
  catch (const ATOOLS::Exception& exception) {
    ATH_MSG_ERROR("Unwanted ATOOLS::exception caught.");
    ATH_MSG_ERROR(exception);
    return StatusCode::FAILURE;
  }
  #else 
  signal(SIGSEGV,ATOOLS::SignalHandler);
  signal(SIGINT,ATOOLS::SignalHandler);
  signal(SIGBUS,ATOOLS::SignalHandler);
  signal(SIGFPE,ATOOLS::SignalHandler);
  signal(SIGABRT,ATOOLS::SignalHandler);
  signal(SIGTERM,ATOOLS::SignalHandler);
  signal(SIGXCPU,ATOOLS::SignalHandler);

  try {
    int argc;
    char** argv;
    getParameters(argc, argv);
    p_sherpa->InitializeTheRun(argc,(char **)argv);
    delete [] argv;

    p_sherpa->InitializeTheEventHandler();
  }
  catch (ATOOLS::Exception exception) {
    if (exception.Class()=="Matrix_Element_Handler" && exception.Type()==ATOOLS::ex::normal_exit) {
      ATH_MSG_ERROR("Have to compile Amegic libraries");
      ATH_MSG_ERROR("You probably want to run ./makelibs");
    }
    else {
      ATH_MSG_ERROR("Unwanted ATOOLS::exception caught.");
      ATH_MSG_ERROR(exception);
    }
    return StatusCode::FAILURE;
  }
  #endif
  catch (std::exception exception) {
    ATH_MSG_ERROR("std::exception caught.");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}


StatusCode Sherpa_i::callGenerator() {
  ATH_MSG_DEBUG("Sherpa_i in callGenerator()");

  do {
    ATH_MSG_DEBUG("Trying to generate event with Sherpa");
  } while (p_sherpa->GenerateOneEvent()==false);
  
  if (ATOOLS::rpa->gen.NumberOfGeneratedEvents()%1000==0) {
    ATH_MSG_INFO("Passed "<<ATOOLS::rpa->gen.NumberOfGeneratedEvents()<<" events.");
  }

  return StatusCode::SUCCESS;
}

StatusCode Sherpa_i::fillEvt(HepMC::GenEvent* event) {
  ATH_MSG_DEBUG( "Sherpa_i Filling event");

  p_sherpa->FillHepMCEvent(*event);
  if (event->weights().size()>2) {
    //double weight_normalisation = event->weights()[2];
    for (size_t i=0; i<event->weights().size(); ++i) {
      //if (i==0 || i>3) event->weights()[i] /= weight_normalisation;
      if (i>3) { // cap variation weights
        // cap variation weights at m_variation_weight_cap*nominal to avoid spikes from numerical instability
        if (fabs(event->weights()[i]) > m_variation_weight_cap*fabs(event->weights()[0])) {
          ATH_MSG_INFO("Capping variation" << i << " = " << event->weights()[i]/event->weights()[0] << "*nominal");
          event->weights()[i] *= m_variation_weight_cap*fabs(event->weights()[0])/fabs(event->weights()[i]);
        }
      }
      ATH_MSG_DEBUG("Sherpa WEIGHT " << i << " value="<< event->weights()[i]);
    }
  }

  GeVToMeV(event); //unit check

  return StatusCode::SUCCESS;
}

StatusCode Sherpa_i::genFinalize() {

  ATH_MSG_INFO("Sherpa_i finalize()");

  std::cout << "MetaData: generator = Sherpa" << SHERPA_VERSION << "." << SHERPA_SUBVERSION << std::endl;
  std::cout << "MetaData: cross-section (nb)= " << p_sherpa->TotalXS()/1000.0*m_xsscale << std::endl;

  std::cout << "MetaData: PDF = " << p_sherpa->PDFInfo() << std::endl;

  std::cout << "Named variations initialised by Sherpa:" << std::endl;
  std::cout << *p_sherpa->GetInitHandler()->GetVariations() << std::endl;

  p_sherpa->SummarizeRun();
  delete p_sherpa;

  if (m_cleanup) {
    ATH_MSG_INFO("Deleting left-over files from working directory.");
    system("rm -rf Results.db Process MIG_*.db MPI_*.dat libSherpa*.so libProc*.so");
  }
  
  return StatusCode::SUCCESS;
}


#ifndef IS_SHERPA_3
void Sherpa_i::getParameters(int &argc, char** &argv) {
  std::vector<std::string> params;

  // set some ATLAS specific default values as a starting point
  params.push_back("EXTERNAL_RNG=Atlas_RNG");

  /***
      Adopt Atlas Debug Level Scheme
  ***/

  std::string verbose_arg;
  MsgStream log(messageService(), name());
  if( log.level()==MSG::FATAL || log.level()==MSG::ERROR || log.level()==MSG::WARNING ){
    params.push_back("OUTPUT=0");
  }
  else if(log.level()==MSG::INFO){
    params.push_back("OUTPUT=2");
  }
  else if(log.level()==MSG::DEBUG){
    params.push_back("OUTPUT=3");
  }
  else{
    params.push_back("OUTPUT=15");
  }

  // disregard manual RUNDATA setting if run card given in JO
  if (m_runcard != "") m_params.push_back("RUNDATA=Run.dat");
  
  // allow to overwrite all parameters from JO file
  params.insert(params.begin()+params.size(), m_params.begin(), m_params.end());

  // create Run.dat file if runcard explicitely given
  if (m_runcard != "") {
    FILE *file = fopen("Run.dat","w");
    fputs(m_runcard.c_str(),file);
    fclose(file);
  }

  /***
      Convert into Sherpas argc/argv arguments
  ***/
  argc = 1+params.size();
  argv = new char * [ 1+params.size() ];
  argv[0] = new char[7];
  strcpy(argv[0], "Sherpa");

  ATH_MSG_INFO("Sherpa_i using the following Arguments");
  ATH_MSG_INFO(m_runcard);
  for(size_t i=0; i<params.size(); i++) {
    ATH_MSG_INFO(" [ " << params[i] << " ] ");
    argv[i+1] = new char[params[i].size()+1];
    strcpy(argv[i+1], params[i].c_str());
  }
  ATH_MSG_INFO("End Sherpa_i Argument List");
  ATH_MSG_INFO("Further Sherpa initialisation output will be redirected to the LOG_FILE specified above.");

}
#endif


void Sherpa_i::compilePlugin(std::string pluginCode) {
  // TODO: not very pretty, should we eventually do this in Python instead (base fragment)
  FILE *file = fopen("Sherpa_iPlugin.C","w");
  fputs(pluginCode.c_str(),file);
  fclose(file);
  std::string command;
  // Python -> C++ string conversion seems to add quote character as first
  // and last line if the string contains quotes (like always in a plugin)
  // thus removing them here
  command += "tail -n +2 Sherpa_iPlugin.C | head -n -1 > Sherpa_iPlugin.C.tmp; mv Sherpa_iPlugin.C.tmp Sherpa_iPlugin.C; ";
  command += "g++ -shared -std=c++0x -g ";
  command += "`Sherpa-config --cppflags` ";
  command += "`Sherpa-config --ldflags` ";
  command += "-fPIC -o libSherpa_iPlugin.so Sherpa_iPlugin.C";
  ATH_MSG_INFO("Now compiling plugin library using: "+command);
  if (system(command.c_str())!=0) {
    ATH_MSG_ERROR("Error compiling plugin library.");
  }
}

/**
   Use ATLAS random number generator for Sherpa's internal random numbers
**/
using namespace ATOOLS;

Atlas_RNG::Atlas_RNG(CLHEP::HepRandomEngine* engine) :
  External_RNG(), p_engine(engine), p_filename("Config.conf")
{
  const int nMax = 26;
  char alphabet[nMax] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g',
                          'h', 'i', 'j', 'k', 'l', 'm', 'n',
                          'o', 'p', 'q', 'r', 's', 't', 'u',
                          'v', 'w', 'x', 'y', 'z' };
                                                                                 
  struct stat info;
  if ( !stat("/dev/shm", &info)) {
    p_filename = "/dev/shm/Config.conf.";
    for (size_t i = 0; i < 6; ++i)
      p_filename += alphabet[rand() % nMax];
  }
  std::cout << "RNG state being saved to: " << p_filename << std::endl;
}

Atlas_RNG::~Atlas_RNG() { std::remove(p_filename.c_str()); }

double Atlas_RNG::Get(){

  return CLHEP::RandFlat::shoot(p_engine);

}

void Atlas_RNG::SaveStatus() { p_engine->saveStatus(p_filename.c_str()); }

void Atlas_RNG::RestoreStatus() { p_engine->restoreStatus(p_filename.c_str()); }

// some getter magic to make this random number generator available to sherpa
DECLARE_GETTER(Atlas_RNG,"Atlas_RNG",External_RNG,RNG_Key);

External_RNG *ATOOLS::Getter<External_RNG,RNG_Key,Atlas_RNG>::operator()(const RNG_Key &) const
{ return new Atlas_RNG(p_rndEngine); }

void ATOOLS::Getter<External_RNG,RNG_Key,Atlas_RNG>::PrintInfo(std::ostream &str,const size_t) const
{ str<<"Atlas RNG interface"; }
