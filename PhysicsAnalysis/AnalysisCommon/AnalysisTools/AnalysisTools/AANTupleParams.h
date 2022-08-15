/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ANALYSISTOOLS_AANTUPLEPARAMS_H
#define ANALYSISTOOLS_AANTUPLEPARAMS_H

namespace AANTupleParams
{
  /// constants for AANTupleStream
  static const char* const c_tokenBranchName         __attribute__((unused)) = "Token";
  static const char* const c_attributeListLayoutName __attribute__((unused)) = "Schema";
  static const std::string c_treeName          = "CollectionTree";
  static const std::string c_streamName        = "AANT";
  
  /// ID of TTree
  static const std::string c_TreeID = "/"+c_streamName+"/"+c_treeName;
  
  // attributes
  static const char* const name_RunNumber   __attribute__((unused)) = "RunNumber";
  static const char* const name_EventNumber __attribute__((unused)) = "EventNumber";
}

#endif

