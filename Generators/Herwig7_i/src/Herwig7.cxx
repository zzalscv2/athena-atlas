// -*- C++ -*-
/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


/*! \file Herwig7.cxx
 *  \brief Implementation of the Herwig 7 Athena interface.
 *  \author Daniel Rauch (daniel.rauch@desy.de)
 */

#include "Herwig7_i/Herwig7.h"

#include "ThePEG/Repository/EventGenerator.h"
#include "ThePEG/Repository/Repository.h"
#include "ThePEG/Persistency/PersistentIStream.h"
#include "ThePEG/Utilities/DynamicLoader.h"
#include "ThePEG/Utilities/Debug.h"
#include "ThePEG/EventRecord/Event.h"
#include "ThePEG/EventRecord/SubProcess.h"
#include "ThePEG/Handlers/XComb.h"
#include "ThePEG/Handlers/EventHandler.h"
#include "ThePEG/PDF/PartonExtractor.h"
#include "ThePEG/PDF/PDF.h"

#include "Herwig/API/HerwigAPI.h"

#include "PathResolver/PathResolver.h"

#include "boost/thread/thread.hpp"
#include <filesystem>
#include "boost/algorithm/string.hpp"

void   convert_to_HepMC(const ThePEG::Event & m_event, HepMC::GenEvent & evt, bool nocopies,ThePEG::Energy eunit, ThePEG::Length lunit);


// Constructor
Herwig7::Herwig7(const std::string& name, ISvcLocator* pSvcLocator) :
  GenModule(name, pSvcLocator),
  m_use_seed_from_generatetf(false),
  m_seed_from_generatetf(0),
  m_pdfname_me("UNKNOWN"), m_pdfname_mpi("UNKNOWN") // m_pdfname_ps("UNKONWN"),
{
  declareProperty("RunFile", m_runfile="Herwig7");
  declareProperty("SetupFile", m_setupfile="");

  declareProperty("UseRandomSeedFromGeneratetf", m_use_seed_from_generatetf);
  declareProperty("RandomSeedFromGeneratetf", m_seed_from_generatetf);

  declareProperty("PDFNameME", m_pdfname_me);
  // declareProperty("PDFNamePS", m_pdfname_ps);
  declareProperty("PDFNameMPI", m_pdfname_mpi);

  declareProperty("CrossSectionScaleFactor", m_xsscale=1.0);

  declareProperty("CleanupHerwigScratch", m_cleanup_herwig_scratch);

  declareProperty("Dsid", m_dsid);
}

/*!
 *  \todo Higher-level API to be provided by the Herwig authors to allow for
 *        slimmer interface and for usage of more advanced features such as
 *        the setupfile mechanism for modifying existing runfiles.
 */
StatusCode Herwig7::genInitialize() {
  ATH_MSG_INFO("Herwig7 initialising...");

  ATH_CHECK(m_evtInfoKey.initialize());

  // Get random number seeds from Atlas RNG service, and pass them as Hw7 RNG
  // default seeds (they can be overridden with an explicit Hw7 set command in the JO)
  CLHEP::HepRandomEngine* engine = this->getRandomEngineDuringInitialize("Herwig7", m_randomSeed); // conditionsRun=1, lbn=1
  const long* seeds = engine->getSeeds();
  // The RNG service supplies two seeds, but Hw7 only uses one. To avoid the situation
  // where varying one seed has no effect (this already stung us in pre-production
  // job transform tests), we multiply the two seeds and let them wrap around the long
  // type MAX_SIZE:
  int32_t combined_seed = std::abs(seeds[0] * seeds[1]);
  // Represent the combined seed as a string, so the config system can parse it back to a long ;)
  std::ostringstream ss_seed;
  ss_seed << combined_seed;
  // Configure the API and print the seed to the log
  if (m_use_seed_from_generatetf){
    ATH_MSG_INFO("Using the random number seed " + std::to_string(m_seed_from_generatetf) + " provided via Generate_tf.py");
    m_api.seed(m_seed_from_generatetf);
  } else {
    ATH_MSG_INFO("Using the random number seed " + ss_seed.str() + " provided by athena");
    m_api.seed(combined_seed);
  }

  // Change repo log level and make sure that config errors result in a program exit
  //ThePEG::Debug::level = 10000;
  ThePEG::Repository::exitOnError() = 1;

  // Horrid runtime ATLAS env variable and CMT path mangling to work out ThePEG module search paths
  char* env1 = getenv("CMTPATH");
  char* env2 = getenv("CMTCONFIG");
  std::string reposearchpaths;
  if (env1 == nullptr || env2 == nullptr) {
     // Use everything from $DATAPATH and $LD_LIBRARY_PATH:
     const char* datapath = getenv( "DATAPATH" );
     reposearchpaths = datapath;
     std::vector< std::string > datapaths;
     boost::split( datapaths, datapath,
                   boost::is_any_of( std::string( ":" ) ) );
     for( const std::string& p : datapaths ) {
        ThePEG::Repository::appendReadDir( p );
     }
     const char* ldpath = getenv( "LD_LIBRARY_PATH" );
     std::vector< std::string > ldpaths;
     boost::split( ldpaths, ldpath, boost::is_any_of( std::string( ":" ) ) );
     for( const std::string& p : ldpaths ) {
        ThePEG::DynamicLoader::appendPath( p );
     }
  } else {
    std::vector<std::string> cmtpaths;
    boost::split(cmtpaths, env1, boost::is_any_of(std::string(":")));
    const std::string cmtconfig = env2;
    const std::string sharepath = "/InstallArea/" + cmtconfig + "/share";
    const std::string libpath = "/InstallArea/" + cmtconfig + "/lib";
    // Prepend to the repository and loader command file search paths
    for(const std::string& p: cmtpaths) {
      const std::string cmtsharepath = p + sharepath;
      ATH_MSG_DEBUG("Appending " + cmtsharepath + " to ThePEG repository and command file search paths");
      reposearchpaths = reposearchpaths + (reposearchpaths.length() == 0 ? "" : ":") + cmtsharepath;
      ThePEG::Repository::appendReadDir(cmtsharepath);
      const std::string cmtlibpath = p + libpath;
      ATH_MSG_DEBUG("Appending " + cmtlibpath + " to ThePEG loader search path");
      ThePEG::DynamicLoader::appendPath(cmtlibpath);
    }
  }
  ATH_MSG_DEBUG("Num of library search paths = " << ThePEG::DynamicLoader::allPaths().size());

  // Use PathResolver to find default Hw7 ThePEG repository file.
  const std::string repopath = PathResolver::find_file_from_list("HerwigDefaults.rpo", reposearchpaths);
  ATH_MSG_DEBUG("Loading Herwig default repo from " << repopath);
  ThePEG::Repository::load(repopath);
  ATH_MSG_DEBUG("Successfully loaded Herwig default repository");

  ATH_MSG_INFO("Setting runfile name '"+m_runfile+"'");
  m_api.inputfile(m_runfile);

  ATH_MSG_INFO("starting to prepare the run from runfile '"+m_runfile+"'...");

#ifdef HEPMC3
  m_runinfo = std::make_shared<HepMC3::GenRunInfo>();
  /// Here one can fill extra information, e.g. the used tools in a format generator name, version string, comment.
  struct HepMC3::GenRunInfo::ToolInfo generator={std::string("Herwig7"), std::string("7"), std::string("Used generator")};
  m_runinfo->tools().push_back(generator);  
#endif
  // read in a Herwig runfile and obtain the event generator
  m_gen = Herwig::API::prepareRun(m_api);
  ATH_MSG_DEBUG("preparing the run...");

  return StatusCode::SUCCESS;
}



/*!
 *  Run the generator for one event and store the event internally.
 */
StatusCode Herwig7::callGenerator() {
  ATH_MSG_DEBUG("Herwig7 generating an event");
  assert(m_gen);
  m_event = m_gen->shoot();
  return StatusCode::SUCCESS;
}



/*!
 *  Fill HepMC event from Herwig's internally stored EventPtr.
 *
 *  \todo Conversion to HepMC format will possibly be provided by the authors
 *        as part of a higher-level API.
 */
StatusCode Herwig7::fillEvt(HepMC::GenEvent* evt) {
  // Convert the Herwig event into the HepMC GenEvent
  ATH_MSG_DEBUG("Converting ThePEG::Event to HepMC::GenEvent");
#ifdef HEPMC3
  if (!evt->run_info()) evt->set_run_info(m_runinfo);
  evt->set_units(HepMC3::Units::MEV, HepMC3::Units::MM);
#endif
  convert_to_HepMC(*m_event, *evt, false, ThePEG::MeV, ThePEG::millimeter);
  ATH_MSG_DEBUG("Converted ThePEG::Event to HepMC::GenEvent");

  // Fill the event number into HepMC event record
  SG::ReadHandle<xAOD::EventInfo> evtInfo(m_evtInfoKey);
  evt->set_event_number(evtInfo->eventNumber());

  // Fill event with random seeds from Atlas RNG service
  const EventContext& ctx = Gaudi::Hive::currentContext();
  const long* s = this->getRandomEngine("Herwig7", ctx)->getSeeds();
  std::vector<long> seeds(s, s+2);
  ATH_MSG_DEBUG("Random seeds: " << seeds[0] << ", " << seeds[1]);
  HepMC::set_random_states(evt,seeds);

  // Add a unit entry to the event weight vector if it's currently empty
  if (evt->weights().empty()) {
    evt->weights().push_back(m_event->weight());
  }

  // Add PDF info manually (for now, until natively supported in the ThePEG converter)
  ATH_MSG_DEBUG("Adding PDF info to HepMC");
  // IDs of the partons going into the primary sub process
  ThePEG::tSubProPtr sub = m_event->primarySubProcess();
  int id1 = sub->incoming().first ->id();
  int id2 = sub->incoming().second->id();
  // Get the event handler
  ThePEG::tcEHPtr eh = ThePEG::dynamic_ptr_cast<ThePEG::tcEHPtr>(m_event->handler());
  // Get the values of x
  double x1 = eh->lastX1();
  double x2 = eh->lastX2();
  // Get the pdfs
  std::pair<ThePEG::PDF,ThePEG::PDF> pdfs;
  pdfs.first  = eh->pdf<ThePEG::PDF>(sub->incoming().first);
  pdfs.second = eh->pdf<ThePEG::PDF>(sub->incoming().second);
  // Get the scale
  ThePEG::Energy2 scale = eh->lastScale();
  double Q = std::sqrt(scale/ThePEG::GeV2);
  // Get the values of the pdfs
  double pdf1 = pdfs.first.xfx(sub->incoming().first ->dataPtr(), scale, x1);
  double pdf2 = pdfs.first.xfx(sub->incoming().second->dataPtr(), scale, x2);
  // Create the PDFinfo object
#ifdef HEPMC3
  HepMC3::GenPdfInfoPtr pdfi = std::make_shared<HepMC3::GenPdfInfo>();
  pdfi->set(id1, id2, x1, x2, Q, pdf1, pdf2);
#else
  HepMC::PdfInfo pdfi(id1, id2, x1, x2, Q, pdf1, pdf2);
#endif
  evt->set_pdf_info(pdfi);
  ATH_MSG_DEBUG("Added PDF info to HepMC");

//uncomment to list HepMC events
//#ifdef HEPMC3
//    std::cout << " print::listing Herwig7 " << std::endl;
//    HepMC3::Print::listing(std::cout, *evt);
//#else
//    std::cout << " print::printing Herwig7 " << std::endl;
//    evt->print();
//#endif

  return StatusCode::SUCCESS;
}



/*!
 *  Tidy up, print out run stats, etc.
 */
StatusCode Herwig7::genFinalize() {
  ATH_MSG_INFO("Herwig7 finalizing.");
  assert(m_gen);
  ATH_MSG_INFO( "MetaData: generator = Herwig7 " << HWVERSION ); 
  ATH_MSG_INFO( std::scientific << std::setprecision(5) << "MetaData: cross-section (nb) = " << m_gen->eventHandler()->integratedXSec()*m_xsscale/ThePEG::nanobarn); 
  // ATH_MSG_INFO( "MetaData: PDF = " << m_pdfname_me << " (ME); " << m_pdfname_ps << " (shower); " << m_pdfname_mpi << " (MPI)" ); 
  ATH_MSG_INFO("MetaData: PDF = " << m_pdfname_me << " (ME); " << m_pdfname_mpi << " (shower/MPI)");
  m_gen->finalize();
  ThePEG::Repository::cleanup();

  // possibly tidy up working directory
  if (m_cleanup_herwig_scratch && (std::filesystem::is_directory("Herwig-scratch") || std::filesystem::is_directory("Herwig-cache"))){

    ATH_MSG_INFO("removing Herwig-scratch/Herwig-cache folder from "+std::filesystem::current_path().string());

    // sleep for some time to allow all access to terminate
    boost::this_thread::sleep(boost::posix_time::seconds(5)); /// \todo Think of other way to wait for all access to terminate

    // in case the folder can't be deleted continue with warning
    try {
      (std::filesystem::remove_all("Herwig-scratch") || std::filesystem::remove_all("Herwig-cache"));
    } 
    catch (const std::exception& e) {
      ATH_MSG_WARNING("Failed to delete the folder 'Herwig-scratch': "+std::string(e.what()));
    }

  }

  return StatusCode::SUCCESS;
}

