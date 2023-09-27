/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "PathResolver/PathResolver.h"

#ifndef XAOD_STANDALONE
#include "GaudiKernel/System.h"
#endif


#include <iostream>

#include <vector>
#include <stdexcept>
#include <stdlib.h>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <filesystem>
#include "CxxUtils/checker_macros.h"



#include "TFile.h"
#include "TSystem.h"
#include "TError.h" //needed to suppress errors when attempting file downloads

namespace bf = std::filesystem;
using namespace std;

namespace {
  enum class DevAreaResponse {
    DEFAULT,
    THROW,
    SILENT,
    INFO,
    WARNING,
    ERROR,
  };
  const std::string pathResolverEnvVar = "PATHRESOLVER_DEVAREARESPONSE";
  DevAreaResponse getDevAreaResponse(
    const std::string& varname = pathResolverEnvVar)
  {
    const char* env = std::getenv(varname.c_str());
    if (env == nullptr) return DevAreaResponse::DEFAULT;
    std::string envstr(env);
#define TRYSTR(string) if (envstr == #string) return DevAreaResponse::string
    TRYSTR(THROW);
    TRYSTR(SILENT);
    TRYSTR(DEFAULT);
    TRYSTR(INFO);
    TRYSTR(WARNING);
    TRYSTR(ERROR);
#undef TRYSTR
    throw std::runtime_error(
      varname + " set to '" + envstr + "', not sure what to do. "
      "Option are DEFAULT, THROW, INFO, WARNING, ERROR, or SILENT");
  }

  void checkForDev(asg::AsgMessaging& asgmsg,
                   const std::string& logical_file_name,
                   const std::string& envvar = pathResolverEnvVar) {
    DevAreaResponse dev_area_response = getDevAreaResponse(envvar);
    asgmsg.msg(MSG::DEBUG) << "Trying to locate " << logical_file_name << endmsg;
    if(logical_file_name.compare(0, 4, "dev/")==0 &&
       dev_area_response != DevAreaResponse::SILENT)
    {
      auto level = MSG::ERROR;
#ifdef XAOD_ANALYSIS
      level = MSG::WARNING;
#endif
      switch(dev_area_response) {
      case DevAreaResponse::INFO: level = MSG::INFO; break;
      case DevAreaResponse::WARNING: level = MSG::WARNING; break;
      case DevAreaResponse::ERROR: level = MSG::ERROR; break;
      default: /* do nothing use the cases above */ ;
      }
      asgmsg.msg(level) << "Locating dev file " << logical_file_name << ". Do not let this propagate to a release" << endmsg;
      if (dev_area_response == DevAreaResponse::THROW) {
        throw std::runtime_error(
          "dev area file " + logical_file_name + " is not allowed! "
          "to override this error set the environment variable " +
          envvar + " to SILENT, DEFAULT, INFO, WARNING, or ERROR");
      }
    }
  }
}

static const char* const path_separator = ",:";
std::atomic<MSG::Level> PathResolver::m_level=MSG::INFO; 

PathResolver::PathResolver() { }

asg::AsgMessaging& PathResolver::asgMsg() {
#ifdef XAOD_STANDALONE
   static thread_local asg::AsgMessaging asgMsg("PathResolver");
#else
   static asg::AsgMessaging asgMsg ATLAS_THREAD_SAFE ("PathResolver");
#endif
/// In AnalysisBase this method is not available
#ifndef XAOD_ANALYSIS
   asgMsg.setLevel(m_level);   
#else
   asgMsg.msg().setLevel(m_level);
#endif
   return asgMsg;
}

//
///////////////////////////////////////////////////////////////////////////
//





/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool
PathResolver::PR_find( const std::string& logical_file_name, const string& search_list,
         PR_file_type file_type, PathResolver::SearchType search_type,
         string& result ) {

   std::string trimmed_logical_file_name = logical_file_name;
   boost::algorithm::trim(trimmed_logical_file_name); //trim again for extra safety

   bf::path file( trimmed_logical_file_name );

  bool found(false);

  // look for file as specified first

  try {
    if ( ( file_type == PR_regular_file && is_regular_file( file ) ) ||
         ( file_type == PR_directory && is_directory( file ) ) ) {
      result = bf::absolute(file).string();
      return true;
    }
  } catch (const bf::filesystem_error& /*err*/) {
  }

  // assume that "." is always part of the search path, so check locally first

  try {
    bf::path local = bf::current_path() / file;
    //AV: boost has bf::path local = bf::initial_path() / file;
    if ( ( file_type == PR_regular_file && is_regular_file( local ) ) ||
         ( file_type == PR_directory && is_directory( local ) ) ) {
      result = bf::absolute(file).string();
      return true;
    }
  } catch (const bf::filesystem_error& /*err*/) {
  }

   std::string locationToDownloadTo = "."; //will replace with first search location 

  // iterate through search list
  vector<string> spv;
  split(spv, search_list, boost::is_any_of( path_separator), boost::token_compress_on);
  for (vector<string>::const_iterator itr = spv.begin();
       itr != spv.end(); ++itr ) {

   if( itr->find("http//")==0 && file_type==PR_regular_file && gSystem->Getenv("PATHRESOLVER_ALLOWHTTPDOWNLOAD") ) { //only http download files, not directories
      //try to do an http download to the local location
      //restore the proper http protocal (had to remove for sake of env var splitting) 
      std::string addr = "http://"; addr += itr->substr(6,itr->length());
      bf::path lp = locationToDownloadTo/file;
      bf::path lpd = lp; lpd.remove_filename();
      msg(MSG::DEBUG) <<"Attempting http download of " << addr << "/" << file.string() << " to " << lp << endmsg;

      if(!is_directory(lpd)) {
         msg(MSG::DEBUG) <<"   Creating directory: " << lpd  << endmsg;
         if(!bf::create_directories(lpd)) {
            msg(MSG::DEBUG) <<"Unable to create directories to write file to : " << lp << endmsg;
            continue;
            //throw std::runtime_error("Unable to download calibration file");
         }
      }
      std::string fileToDownload = addr + "/" + file.string();
      //disable error output from root while attempting to download
      // FIXME: Disabling errors now commented out because it is not
      //        thread-safe.  Needs changes in ROOT.
      //long errLevel = gErrorIgnoreLevel;
      //gErrorIgnoreLevel = kError+1;
      if(!TFile::Cp(fileToDownload.c_str(),(locationToDownloadTo+"/"+file.string()).c_str())) {
         msg(MSG::DEBUG) <<"Unable to download file : " << fileToDownload << endmsg;
      } else {
         msg(MSG::INFO) <<"Successfully downloaded " << fileToDownload << endmsg;
         result = (locationToDownloadTo+"/"+file.string()).c_str();
         //gErrorIgnoreLevel=errLevel;
         return true;
      }
      //gErrorIgnoreLevel=errLevel;
   } else if(locationToDownloadTo=="." && itr->find("/afs/cern.ch/atlas/www/")==std::string::npos) { //don't let it ever download back to the www area!
      //prefer first non-pwd location for downloading to. But must be fully accessible. This should be the local InstallArea in cmt
     FILE *fp = std::fopen((*itr+"/._pathresolver_dummy").c_str(), "a+");
     if(fp!=NULL) {
         locationToDownloadTo=*itr;
         std::fclose(fp);
         (void)std::remove((*itr+"/._pathresolver_dummy").c_str());
      }
   }

   //std::cout << "searching path: " << *itr);

    bf::path fp = *itr / file;

    try {
      if ( ( file_type == PR_regular_file && is_regular_file( fp ) ) ||
           ( file_type == PR_directory && is_directory( fp ) ) ) {
        result = bf::absolute(fp).string();
        return true;
      }
    } catch (const bf::filesystem_error& /*err*/) {
    }


    // if recursive searching requested, drill down
    if (search_type == PathResolver::RecursiveSearch &&
        is_directory( bf::path(*itr) ) ) {

      bf::recursive_directory_iterator end_itr;
      try {
        for ( bf::recursive_directory_iterator ritr( *itr );
              ritr != end_itr; ++ritr) {

          // skip if not a directory
          if (! is_directory( bf::path(*ritr) ) ) { continue; }

          bf::path fp2 = bf::path(*ritr) / file;
          if ( ( file_type == PR_regular_file && is_regular_file( fp2 ) ) ||
               ( file_type == PR_directory && is_directory( fp2 ) ) ) {
            result = bf::absolute( fp2 ).string();
            return true;
          }
        }
      } catch (const bf::filesystem_error& /*err*/) {
      }
    }

  }

  return found;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

string
PathResolver::find_file(const std::string& logical_file_name,
              const std::string& search_path,
              SearchType search_type) {
   //std::cout << "finding file: " <<logical_file_name << " in path=" << search_path);

  std::string path_list;

#ifdef XAOD_STANDALONE
   const char* envVarVal = gSystem->Getenv(search_path.c_str());
   if(envVarVal == NULL) {
      msg(MSG::WARNING) <<search_path.c_str() << " environment variable not defined!" << endmsg;
      path_list = ""; //this will allow search in pwd ... maybe we should throw exception though!
   }
   else { path_list = envVarVal; }
#else
  System::getEnv(search_path, path_list);
#endif

#ifndef XAOD_ANALYSIS
  if (!logical_file_name.empty() && logical_file_name[0]=='/') {
    msg(MSG::ERROR) << "Use of an absolute file name: " << logical_file_name << endmsg;
  }
#endif

  return (find_file_from_list (logical_file_name, path_list, search_type));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

std::string
PathResolver::find_file_from_list (const std::string& logical_file_name,
                                   const std::string& search_list,
                                   SearchType search_type)
{
  std::string result("");

  

  /* bool found = */
  PR_find (logical_file_name, search_list, PR_regular_file, search_type, result);

  // The following functionality was in the original PathResolver, but I believe
  // that it's WRONG. It extracts the filename of the requested item, and searches
  // for that if the preceding search fails. i.e., if you're looking for "B/a.txt",
  // and that fails, it will look for just "a.txt" in the search list.

  // if (! found && lfn.filename() != lfn ) {
  //   result = "";
  //   PR_find (lfn.filename(), search_list, PR_regular_file, search_type, result);
  // }

  return (result);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

string PathResolver::find_directory (const std::string& logical_file_name,
                                     const std::string& search_path,
                                     SearchType search_type)
{
     std::string path_list;

#ifdef XAOD_STANDALONE
   const char* envVarVal = gSystem->Getenv(search_path.c_str());
   if(envVarVal == NULL) {
      msg(MSG::WARNING) <<search_path.c_str() << " environment variable not defined!" << endmsg;
      path_list = ""; //this will allow search in pwd ... maybe we should throw exception though!
   }
   else { path_list = envVarVal; }
#else
  System::getEnv(search_path, path_list);
#endif

  return (find_directory_from_list (logical_file_name, path_list, search_type));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

string
PathResolver::find_directory_from_list (const std::string& logical_file_name,
                                        const std::string& search_list,
                                        SearchType search_type)
{
  std::string result;

  if (!PR_find (logical_file_name, search_list, PR_directory, search_type, result))
  {
    result = "";
  }

  return (result);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PathResolver::SearchPathStatus
PathResolver::check_search_path (const std::string& search_path)
{
  std::string path_list = gSystem->Getenv(search_path.c_str());
  if ( path_list.empty() )
    return (EnvironmentVariableUndefined);

  vector<string> spv;
  boost::split( spv, path_list, boost::is_any_of( path_separator ), boost::token_compress_on);
  vector<string>::iterator itr=spv.begin();

  try {
    for (; itr!= spv.end(); ++itr) {
      bf::path pp(*itr);
      if (!is_directory(pp)) {
        return (UnknownDirectory);
      }
    }
  } catch(const bf::filesystem_error& /*err*/) {
    return (UnknownDirectory);
  }

  return ( Ok );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

std::string PathResolverFindXMLFile (const std::string& logical_file_name)
{
  return PathResolver::find_file (logical_file_name, "XMLPATH");
}

std::string PathResolverFindDataFile (const std::string& logical_file_name)
{
  return PathResolver::find_file (logical_file_name, "DATAPATH");
}

std::string PathResolver::find_calib_file (const std::string& logical_file_name)
{
  checkForDev(asgMsg(), logical_file_name);
  //expand filename before finding .. 
  TString tmpString(logical_file_name);
  gSystem->ExpandPathName(tmpString);
  if(tmpString.BeginsWith("root://")) {
    //xrootd access .. try to open file ...
    TFile* fTmp = TFile::Open(tmpString);
    if(!fTmp || fTmp->IsZombie()) {
      msg(MSG::WARNING) << "Could not open " << logical_file_name << endmsg;
      tmpString = "";
    }
    
    if(fTmp) {
      fTmp->Close();
      delete fTmp;
    }
    
    return tmpString.Data();
  }
  std::string expandedFileName(tmpString.Data());
   //strip any spaces off of the name
  boost::algorithm::trim(expandedFileName);
  std::string out = PathResolver::find_file (expandedFileName, "CALIBPATH");
  if(out=="") msg(MSG::WARNING) <<"Could not locate " << logical_file_name << endmsg;
   return out;
}

std::string PathResolver::find_calib_directory (const std::string& logical_file_name)
{
  checkForDev(asgMsg(), logical_file_name);
  //expand filename before finding 
  TString tmpString(logical_file_name);
  gSystem->ExpandPathName(tmpString);
  std::string expandedFileName(tmpString.Data());
  boost::algorithm::trim(expandedFileName);
  std::string out = PathResolver::find_directory (expandedFileName, "CALIBPATH");
  if(out=="") msg(MSG::WARNING) <<"Could not locate " << logical_file_name << endmsg;
   return out;
}

void PathResolver::setOutputLevel(MSG::Level level) {
   m_level = level;
}


std::string PathResolverFindCalibFile (const std::string& logical_file_name) { return PathResolver::find_calib_file(logical_file_name); }
std::string PathResolverFindCalibDirectory (const std::string& logical_file_name) { return PathResolver::find_calib_directory(logical_file_name); }
void PathResolverSetOutputLevel(int lvl) { PathResolver::setOutputLevel(MSG::Level(lvl)); }
