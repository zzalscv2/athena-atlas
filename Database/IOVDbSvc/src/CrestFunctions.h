/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file CrestFunctions.h
 * @brief Header for CrestFunctions utilities
 * @author Shaun Roe
 * @date 1 July 2019
 */

#ifndef IOVDBSVC_CRESTFUNCTIONS_H
#define IOVDBSVC_CRESTFUNCTIONS_H

#include <vector>
#include <string>
#include <map> //also contains std::pair
#include <string_view>

#include "CoolKernel/ChannelId.h"

namespace IOVDbNamespace{
  typedef std::pair<std::string,std::string> IovHashPair; // <IOV,Hash> pairs extracted from Json

  const std::string_view
  urlBase();

  std::vector<IovHashPair>
  extractIovAndHash(const std::string_view jsonReply);

  std::string
  extractHashFromJson(const std::string & jsonReply);

  std::vector<IovHashPair>
  getIovsForTag(const std::string & tag, const bool testing=false);

  std::string
  getLastHashForTag(const std::string & tag, const bool testing=false);

  std::string 
  getPayloadForHash(const std::string & hash, const bool testing=false);
  
  std::pair<std::vector<cool::ChannelId> , std::vector<std::string>>
  extractChannelListFromJson(const std::string & jsonReply);

  std::pair<std::vector<cool::ChannelId> , std::vector<std::string>>
  channelListForTag(const std::string & tag, const bool testing=false);
  
  std::map<cool::ChannelId, std::string> 
  channelNameMap(const std::string & folderName);
  
  std::string 
  getPayloadForTag(const std::string & tag,const bool testing=false);
  
  std::string 
  folderDescriptionForTag(const std::string & tag, const bool testing=false);
  
  std::string 
  payloadSpecificationForTag(const std::string & tag, const bool testing=false);
  
  std::string 
  extractDescriptionFromJson(const std::string & jsonReply);
  
  std::string
	resolveCrestTag(const std::string & globalTagName, const std::string & folderName, const std::string & forceTag="", const bool testing=false);
	
  
  std::string
  jsonTagName(const std::string &globalTag, const std::string & folderName);

  std::map<std::string, std::string>
    getGlobalTagMap(const std::string globaltag);
}
#endif
