/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/**
 */

#include <iostream>
#include <string>
#include <cstdlib>

#include <TFile.h>
#include <TKey.h>

#include "CxxUtils/checker_macros.h"

#include "DataQualityInterfaces/HanConfigGroup.h"
#include "DataQualityInterfaces/HanConfigMetadata.h"
#include "DataQualityInterfaces/ConditionsSingleton.h"

namespace {

int usage( const std::string& command_name, int exit_code );

struct CmdLineArgs {
  int parse( int argc, char *argv[] );

  std::string command;
  std::string hconfig;
  std::string conditions;
};

} // unnamed namespace


int main ATLAS_NOT_THREAD_SAFE ( int argc, char *argv[] )
{
  CmdLineArgs arg;
  int rc = arg.parse( argc, argv );
  if (rc!=0) return rc;

  dqi::ConditionsSingleton::getInstance().setCondition(arg.conditions);
  //std::cout<<__PRETTY_FUNCTION__<<"Input Conditions="<<dqi::ConditionsSingleton::getInstance().getCondition()
  //	   <<std::endl;
  std::string configName( arg.hconfig );
  TFile* config = TFile::Open( configName.c_str() );
  if( config == 0 ) {
    std::cerr << "Cannot open file \"" << configName << "\"\n";
    return 1;
  }

  TKey* key = config->FindKey("HanMetadata");
  if (key) {
    std::cout << "Metadata:\n";
    TDirectory* metadatadir = dynamic_cast<TDirectory*>(key->ReadObj());
    if (! metadatadir) {
      std::cerr << "Metadata directory is malformed\n";
      return 1;
    }
    dqi::HanConfigMetadata* hcm(0); TKey* mdkey(0);
    TIter next(metadatadir->GetListOfKeys());
    while ((mdkey = dynamic_cast<TKey*>(next()))) {
      metadatadir->GetObject(mdkey->GetName(), hcm);
      if (hcm) {
	std::cout << *hcm << std::endl;
      }
    }
    delete metadatadir;
  }

  key = config->FindKey("top_level");
  if( key == 0 ) {
    std::cerr << "Cannot find configuration in file \"" << configName << "\"\n";
    return 1;
  }
  
  dqi::HanConfigGroup* g = dynamic_cast<dqi::HanConfigGroup*>( key->ReadObj() );
  if( g == 0 ) {
    std::cerr << "Cannot find configuration in file \"" << configName << "\"\n";
    return 1;
  }
  
  std::cout << g;
  return 0;
}


// ************************************************************
// Private Functions
// ************************************************************

namespace {

int CmdLineArgs::parse( int argc, char *argv[] )
{
  command = argv[0];
  if( argc > 3 ) return usage( command, 1 );
  if( argc < 2 ) return usage( command, 0 );
  
  hconfig = argv[1];
  if(argc==3)conditions = argv[2];
  return 0;
}


int usage( const std::string& command_name, int exit_code )
{
  std::string message;
  message += "\n";
  message += "This program takes as an argument the name of a \'han\' configuration\n";
  message += "file and prints the configuration.\n";

  std::string::size_type s = command_name.size();
  std::string::size_type p = command_name.rfind( '/', s );
  std::string short_name = command_name.substr( p+1, s );

  std::cout << "\n";
  std::cout << "Usage: " << short_name << " <filename>\n";
  std::cout << message << "\n";
  return exit_code;
}


} // unnamed namespace
