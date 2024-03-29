/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


#include "GoodRunsLists/GoodRunsListSelectorTool.h"
#include "GoodRunsLists/TGRLCollection.h"
#include "GoodRunsLists/TGoodRunsListReader.h"
#include "GoodRunsLists/TMsgLogger.h"
#include "GoodRunsLists/StrUtil.h"

#include "PathResolver/PathResolver.h"

#include "GaudiKernel/MsgStream.h"

#include "TFormula.h"
#include "TSystem.h"

#include <sys/stat.h>

using namespace std;

GoodRunsListSelectorTool::GoodRunsListSelectorTool( const std::string& type, const std::string& name, const IInterface* parent )
 : AthAlgTool( type, name, parent )
 , m_reader(0)
 , m_boolop(0)
 , m_passthrough(true)
 , m_verbose(false)
 , m_rejectanybrl(false)
 , m_eventselectormode(false)
{
  declareInterface<IGoodRunsListSelectorTool>(this);
  declareInterface<IAthenaEvtLoopPreSelectTool>(this);

  declareProperty( "GoodRunsListVec", m_goodrunslistVec, "list of input xml files" );
  declareProperty( "BlackRunsListVec", m_blackrunslistVec, "list of input xml files" );
  declareProperty( "BoolOperation", m_boolop );
  declareProperty( "PassThrough", m_passthrough = true);
  declareProperty( "VerboseDetStatus", m_verbose = false);
  declareProperty( "RejectBlackRunsInEventSelector", m_rejectanybrl = false );
  declareProperty( "EventSelectorMode", m_eventselectormode = false );

  m_grlcollection = new Root::TGRLCollection();
  m_brlcollection = new Root::TGRLCollection();
  m_reader = new Root::TGoodRunsListReader();
}


GoodRunsListSelectorTool::~GoodRunsListSelectorTool()
{
  if (m_grlcollection!=0) delete m_grlcollection;
  if (m_brlcollection!=0) delete m_brlcollection;
  if (m_reader!=0) delete m_reader;

  // delete all the formula pntrs in the map
  while ( ! m_dqformula.empty() ) {
    std::map< std::string,TFormula* >::iterator itr= m_dqformula.begin();
    TFormula* form = (*itr).second;
    m_dqformula.erase(itr);
    delete form;
  }
}


StatusCode
GoodRunsListSelectorTool::queryInterface( const InterfaceID& riid, void** ppvIf )
{
  if ( riid == IGoodRunsListSelectorTool::interfaceID() )  {
     *ppvIf = (IGoodRunsListSelectorTool*)this;
     addRef();
     return StatusCode::SUCCESS;
  }

  return AthAlgTool::queryInterface( riid, ppvIf );
}


StatusCode
GoodRunsListSelectorTool::initialize()
{
  ATH_MSG_DEBUG ("initialize() ");

  Root::TMsgLogger::SetMinLevel(static_cast<Root::TMsgLevel>(msgLevel()));

  /// reset pass-through mode
  if (!m_goodrunslistVec.empty() || !m_blackrunslistVec.empty() || m_usecool)
    m_passthrough=false;

  /// warn about pass-thru mode
  if (m_passthrough) ATH_MSG_WARNING ("Set to pass-through mode.");

  /// checking existence of goodrunslists / blacklists
  std::vector<std::string>::iterator itr;
  for (itr=m_goodrunslistVec.begin(); itr!=m_goodrunslistVec.end() && !m_usecool; ++itr)  {
    //const char* fname;
    std::string fname;
    if ( itr->find("/")==0 || itr->find("$")==0 || itr->find(".")==0 || itr->find(":")!=string::npos )  {
      fname = gSystem->ExpandPathName( itr->c_str() );
    }
    else {
      fname = (PathResolverFindXMLFile( *itr ));
    }
    if ( !fileExists(fname.c_str()) ) {
      ATH_MSG_ERROR ("Cannot open file : " << fname);
      return StatusCode::FAILURE;
    }
  }
  for (itr=m_blackrunslistVec.begin(); itr!=m_blackrunslistVec.end() && !m_usecool; ++itr)  {
    //const char* fname;
    std::string fname;
    if ( itr->find("/")==0 || itr->find("$")==0 || itr->find(".")==0 || itr->find(":")!=string::npos )  {
      fname = gSystem->ExpandPathName( itr->c_str() );
    }
    else {
      fname = (PathResolverFindXMLFile( *itr ));
    }
    if ( !fileExists(fname.c_str()) ) {
      ATH_MSG_ERROR ("Cannot open file : " << fname);
      return StatusCode::FAILURE;
    }
  }
  /// start reading xml files
  if ( !m_goodrunslistVec.empty() ) {
    m_reader->Reset();
    for (itr=m_goodrunslistVec.begin(); itr!=m_goodrunslistVec.end() && !m_usecool; ++itr) {
      //const char* fname;
      std::string fname;
      if ( itr->find("/")==0 || itr->find("$")==0 || itr->find(".")==0 || itr->find(":")!=string::npos )  {
        fname = gSystem->ExpandPathName( itr->c_str() );
      }
      else {
        fname = (PathResolverFindXMLFile( *itr ));
      }
      m_reader->AddXMLFile(fname);
    }
    m_reader->Interpret();
    /// this merge accounts for same identical metadata, version, name, etc.
    *m_grlcollection = m_reader->GetMergedGRLCollection(static_cast<Root::BoolOperation>(m_boolop));
  }
  if ( !m_blackrunslistVec.empty() ) {
    m_reader->Reset();
    for (itr=m_blackrunslistVec.begin(); itr!=m_blackrunslistVec.end() && !m_usecool; ++itr) {
      //const char* fname;
      std::string fname;
      if ( itr->find("/")==0 || itr->find("$")==0 || itr->find(".")==0 || itr->find(":")!=string::npos )  {
        fname = gSystem->ExpandPathName( itr->c_str() );
      }
      else {
        fname = (PathResolverFindXMLFile( *itr ));
      }
      m_reader->AddXMLFile(fname);
    }
    m_reader->Interpret();
    /// this merge accounts for same identical metadata, version, name, etc.
    *m_brlcollection = m_reader->GetMergedGRLCollection(static_cast<Root::BoolOperation>(m_boolop));
  }

  return StatusCode::SUCCESS;
}


bool
GoodRunsListSelectorTool::passEvent(const EventIDBase& pEvent)
{
  ATH_MSG_DEBUG ("passEvent() ");

  auto eventNumber = pEvent.event_number();
  int runNumber   = pEvent.run_number();
  int lumiBlockNr = pEvent.lumi_block();
  auto timeStamp   = pEvent.time_stamp();

  ATH_MSG_DEBUG ("passEvent() :: run number = " << runNumber <<
                 " ; event number = " << eventNumber <<
                 " ; lumiblock number = " << lumiBlockNr <<
                 " ; timestamp = " << timeStamp
                );

  /// now make query decision ...
  bool pass(false);
  if (m_passthrough) {
    ATH_MSG_DEBUG ("passEvent() :: Pass through mode.");
    pass = true;
  }
  /// decide from XML files
  else {
    pass = this->passRunLB(runNumber,lumiBlockNr);
  }

  return pass;
}


bool
GoodRunsListSelectorTool::passThisRunLB( const std::vector<std::string>& grlnameVec,
                                         const std::vector<std::string>& brlnameVec )
{
  ATH_MSG_DEBUG ("passThisRunLB() ");

  const EventContext& ctx = Gaudi::Hive::currentContext();

  int eventNumber = ctx.eventID().event_number();
  int runNumber   = ctx.eventID().run_number();
  int lumiBlockNr = ctx.eventID().lumi_block();
  int timeStamp   = ctx.eventID().time_stamp();

  ATH_MSG_DEBUG ("passThisRunLB() :: run number = " << runNumber <<
                 " ; event number = " << eventNumber <<
                 " ; lumiblock number = " << lumiBlockNr <<
                 " ; timestamp = " << timeStamp
                );

  /// now make query decision ...
  bool pass(false);
  if (m_passthrough) {
    ATH_MSG_DEBUG ("passThisRunLB() :: Pass through mode.");
    pass = true;
  }
  /// decide from XML files
  else {
    pass = this->passRunLB(runNumber,lumiBlockNr,grlnameVec,brlnameVec);
  }

  return pass;
}


bool
GoodRunsListSelectorTool::passRunLB( int runNumber, int lumiBlockNr,
                                     const std::vector<std::string>& grlnameVec,
                                     const std::vector<std::string>& brlnameVec )
{
  ATH_MSG_DEBUG ("passRunLB() ");

  /// pass through
  if (m_passthrough) {
    ATH_MSG_DEBUG ("passRunLB() :: Pass through mode.");
    return true;
  }

  /// decision based on merged blackrunslist
  if ( m_rejectanybrl && m_eventselectormode ) {
    if ( m_brlcollection->HasRunLumiBlock(runNumber,lumiBlockNr) ) {
      ATH_MSG_DEBUG ("passRunLB() :: Event rejected by (_any_ of) merged black runs list.");
      return false;
    }
  /// decision based on specific blackrunlists
  } else if (!brlnameVec.empty()) {
    bool reject(false);
    std::vector<Root::TGoodRunsList>::const_iterator brlitr;
    for (unsigned int i=0; i<brlnameVec.size() && !reject; ++i) {
      brlitr = m_brlcollection->find(brlnameVec[i]);
      if (brlitr!=m_brlcollection->end())
        reject = brlitr->HasRunLumiBlock(runNumber,lumiBlockNr);
    }
    if (reject) {
      ATH_MSG_DEBUG ("passRunLB() :: Event rejected by specific black runs list.");
      return false;
    }
  }

  /// decision based on specific goodrunlists
  if (!grlnameVec.empty()) {
    bool pass(false);
    std::vector<Root::TGoodRunsList>::const_iterator grlitr;
    for (unsigned int i=0; i<grlnameVec.size() && !pass; ++i) {
      grlitr = m_grlcollection->find(grlnameVec[i]);
      if (grlitr!=m_grlcollection->end())
        pass = grlitr->HasRunLumiBlock(runNumber,lumiBlockNr);
    }
    if (pass) {
      ATH_MSG_DEBUG ("passRunLB() :: Event accepted by specific good runs list.");
      return true;
    }
  /// decision based on merged goodrunslist
  } else if (m_grlcollection->HasRunLumiBlock(runNumber,lumiBlockNr)) {
    ATH_MSG_DEBUG ("passRunLB() :: Event accepted by (_any_ of) merged good runs list.");
    return true;
  }

  ATH_MSG_DEBUG ("passRunLB() :: Event rejected, not in (any) good runs list.");
  return false;
}


StatusCode
GoodRunsListSelectorTool::finalize()
{
  ATH_MSG_DEBUG ("finalize() ");
  return StatusCode::SUCCESS;
}


bool
GoodRunsListSelectorTool::fileExists(const char* fileName)
{
  struct stat info;
  int ret = -1;

  //get the file attributes
  ret = stat(fileName, &info);

  if(ret == 0) {
    /// stat() is able to get the file attributes, so the file obviously exists
    /// if filesize==0 assume the copying failed.
    //if (info.st_size == 0) return false;
    //else
    return true;
  } else {
    /// stat() is not able to get the file attributes, so the file obviously does not exist.
    return false;
  }
}


bool
GoodRunsListSelectorTool::registerGRLSelector(const std::string& name, const std::vector<std::string>& grlnameVec, const std::vector<std::string>& brlnameVec)
{
  if (m_registry.find(name)!=m_registry.end()) {
    ATH_MSG_WARNING ("registerGRLSelector() :: GRL selector with name <" << name << "> already registered. Return false.");
    return false;
  }

  /// check if brl names are really known
  if (!brlnameVec.empty()) {
    std::vector<Root::TGoodRunsList>::const_iterator brlitr;
    for (unsigned int i=0; i<brlnameVec.size(); ++i) {
      brlitr = m_brlcollection->find(brlnameVec[i]);
      if (brlitr==m_brlcollection->end()) {
        ATH_MSG_ERROR ("registerGRLSelector() :: requested BRL object with name <" << brlnameVec[i] << "> not found. Have you provided an object name from the BRL xml-file(s)?");
        return false;
      }
    }
  }

  /// check if grl names are really known
  if (!grlnameVec.empty()) {
    std::vector<Root::TGoodRunsList>::const_iterator grlitr;
    for (unsigned int i=0; i<grlnameVec.size(); ++i) {
      grlitr = m_grlcollection->find(grlnameVec[i]);
      if (grlitr==m_grlcollection->end()) {
        ATH_MSG_ERROR ("registerGRLSelector() :: requested GRL object with name <" << grlnameVec[i] << "> not found. Have you provided an object name from the GRL xml-file(s)?");
        return false;
      }
    }
  }

  ATH_MSG_DEBUG ("registerGRLSelector() :: GRL selector with name <" << name << "> registered.");
  m_registry[name] = vvPair(grlnameVec,brlnameVec);
  return true;
}
