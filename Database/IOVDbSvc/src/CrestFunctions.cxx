/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
// @file CrestFunctions.cxx
// Implementation for CrestFunctions utilities
// @author Shaun Roe
// @date 1 July 2019

#include "CrestFunctions.h"
#include "CrestApi/CrestApi.h"
#include <iostream>
#include <exception>
#include <regex>
#include "IOVDbStringFunctions.h"
#include <string>
#include <algorithm>
#include <map>

namespace IOVDbNamespace{
  const std::string_view
  urlBase(){
    return "http://crest-02.cern.ch:9090";
  }

  std::vector<IovHashPair>
  extractIovAndHash(const std::string_view jsonReply){
    std::vector<IovHashPair> iovHashPairs;
    bool all_ok = true;
    std::string_view iovSignature = "since\":";
    std::string_view hashSignature = "payloadHash\":\"";
    size_t startpoint = jsonReply.find(hashSignature);
    size_t endpoint = 0;

    while(startpoint!=std::string::npos) {
      startpoint+=hashSignature.size();
      endpoint = jsonReply.find('\"',startpoint);
      if(endpoint==std::string::npos) {
	all_ok = false;
	break;
      }
      std::string_view hashString = jsonReply.substr(startpoint,endpoint-startpoint);
      startpoint= jsonReply.find(iovSignature,endpoint);
      if(startpoint==std::string::npos) {
	all_ok = false;
	break;
      }
      startpoint+=iovSignature.size();
      endpoint = jsonReply.find(',',startpoint);
      if(endpoint==std::string::npos) {
	all_ok = false;
	break;
      }
      std::string_view iovString = jsonReply.substr(startpoint,endpoint-startpoint);
      iovHashPairs.emplace_back(iovString,hashString);
      startpoint= jsonReply.find(hashSignature,endpoint);
    }
    if(!all_ok) {
      std::cerr<<__FILE__<<":"<<__LINE__<< ": Formatting error found while trying to extract IOVs and Hashes from "<<jsonReply<<std::endl;
      iovHashPairs.clear();
    }
    return iovHashPairs;
  }

  std::string
  extractHashFromJson(const std::string & jsonReply){
    std::string hash{};
    try{
      std::string_view signature="payloadHash\":\"";
      auto signaturePosition=jsonReply.rfind(signature);
      if (signaturePosition == std::string::npos) throw std::runtime_error("signature "+std::string(signature)+" not found");
      auto startOfHash=signaturePosition + signature.size();
      auto endOfHash=jsonReply.find('\"',startOfHash);
      auto len=endOfHash-startOfHash;
      if (startOfHash > jsonReply.size()) throw std::runtime_error("Hash start is beyond end of string");
      hash=jsonReply.substr(startOfHash, len);
    } catch (std::exception & e){
      std::cout<<__FILE__<<":"<<__LINE__<< ": "<<e.what()<<" while trying to find the hash in "<<jsonReply<<std::endl;
    }
    return hash;
  }

  std::vector<IovHashPair>
  getIovsForTag(const std::string & tag, const bool testing){
    std::string reply{R"delim([{"insertionTime":"2022-05-26T12:10:58+0000","payloadHash":"99331506eefbe6783a8d5d5bc8b9a44828a325adfcaac32f62af212e9642db71","since":0,"tagName":"LARIdentifierFebRodMap-RUN2-000"}])delim"};
    if (not testing){
      //...CrestApi returns Iovs as a json object
      auto myCrestClient = Crest::CrestClient(urlBase());
      try{
        reply = myCrestClient.findAllIovs(tag).dump();
      } catch (std::exception & e){
        std::cout<<__FILE__<<":"<<__LINE__<< ": "<<e.what()<<" while trying to find the IOVs"<<std::endl;
        return std::vector<IovHashPair>();
      }
    }
    return extractIovAndHash(reply);
  }

  std::string 
  getLastHashForTag(const std::string & tag, const bool testing){
    char tu[] = "";
    strfry(tu);
    std::string reply{R"delim([{"insertionTime":"2022-05-26T12:10:58+0000","payloadHash":"99331506eefbe6783a8d5d5bc8b9a44828a325adfcaac32f62af212e9642db71","since":0,"tagName":"LARIdentifierFebRodMap-RUN2-000"}])delim"};
    if (not testing){
      //...CrestApi returns Iovs as a json object
      auto myCrestClient = Crest::CrestClient(urlBase());
      try{
        reply = myCrestClient.findAllIovs(tag).dump();
      } catch (std::exception & e){
        std::cout<<__FILE__<<":"<<__LINE__<< ": "<<e.what()<<" while trying to find the IOVs"<<std::endl;
        return "";
      }
    }
    return extractHashFromJson(reply);
  }


  std::string 
  getPayloadForHash(const std::string & hash, const bool testing){
    std::string reply{R"delim({"data":{"0":["[DB=B2E3B2B6-B76C-DF11-A505-000423D5ADDA][CNT=CollectionTree(LArTTCell_P/LArTTCellMapAtlas)][CLID=DF8C509C-A91A-40B5-B76C-5B57EEE21EC3][TECH=00000202][OID=00000003-00000000]"]}})delim"};
    if (not testing){
      //CrestApi method:
      try{
        auto   myCrestClient = Crest::CrestClient(urlBase());
        reply = myCrestClient.getPayloadAsString(hash);
      } catch (std::exception & e){
        std::cout<<__FILE__<<":"<<__LINE__<< ": "<<e.what()<<" while trying to find the payload"<<std::endl;
        return "";
      }
    }
    return reply;
  }
  
  std::string 
  getPayloadForTag(const std::string & tag, const bool testing){
    return getPayloadForHash(getLastHashForTag(tag, testing), testing);
  }
  
  std::string 
  extractDescriptionFromJson(const std::string & jsonReply){
    std::string description{};
    try{
      const std::string_view signature="node_description\\\":\\\"";
      const auto signaturePosition = jsonReply.find(signature);
      if (signaturePosition == std::string::npos) throw std::runtime_error("signature "+std::string(signature)+" not found");
      const auto startOfDescription= signaturePosition + signature.size();
      const std::string_view endSignature = "\\\",\\\"payload_spec";
      const auto endOfDescription=jsonReply.find(endSignature);
      if (endOfDescription == std::string::npos) throw std::runtime_error("end signature "+std::string(endSignature)+" not found");
      const auto len=endOfDescription-startOfDescription;
      description=jsonReply.substr(startOfDescription, len);
    } catch (std::exception & e){
      std::cout<<__FILE__<<":"<<__LINE__<< ": "<<e.what()<<" while trying to find the description in "<<jsonReply<<std::endl;
    }
    
    return unescapeQuotes(unescapeBackslash(description));
  }
  
  std::string 
  extractSpecificationFromJson(const std::string & jsonReply){
    std::string spec{};
    try{
      const std::string_view signature="payload_spec\\\":\\\"";
      const auto signaturePosition = jsonReply.find(signature);
      if (signaturePosition == std::string::npos) throw std::runtime_error("signature "+std::string(signature)+" not found");
      const auto startOfSpec= signaturePosition + signature.size();
      const auto endOfSpec=jsonReply.find("\\\"}\"",startOfSpec);
      const auto len=endOfSpec-startOfSpec;
      spec=jsonReply.substr(startOfSpec, len);
    } catch (std::exception & e){
      std::cout<<__FILE__<<":"<<__LINE__<< ": "<<e.what()<<" while trying to find the payload spec in "<<jsonReply<<std::endl;
    }
    return spec;
  }
  
  std::pair<std::vector<cool::ChannelId> , std::vector<std::string>>
  extractChannelListFromJson(const std::string & jsonReply){
    std::vector<cool::ChannelId> list;
    std::vector<std::string> names;
    std::string textRep;
    try{
      const std::string_view signature="channel_list\\\":[";
      const auto startOfList=jsonReply.find(signature) + signature.size();
      const auto endOfList=jsonReply.find(']', startOfList);
      const auto len=endOfList-startOfList;
      textRep=jsonReply.substr(startOfList, len);
    } catch (std::exception & e){
      std::cout<<__FILE__<<":"<<__LINE__<< ": "<<e.what()<<"\n while trying to find the description in "<<jsonReply<<std::endl;
    }
    //channel list is of format [{\"956301312\":\"barrel A 01L PS\"},{\"956334080\":\"barrel A 01L F0\"}]
    std::string s=R"d(\{\\\"([0-9]+)\\\":\\\"([^\"]*)\"},?)d";
    std::regex r(s);
    std::sregex_iterator it(textRep.begin(), textRep.end(), r);
    std::sregex_iterator end;
    for (;it!=end;++it){
      std::smatch  m= *it;
      if (not m.empty()){ 
        list.push_back(std::stoll(m[1].str()));
        //chomp the last backslash
        std::string s = m[2].str();
        s.pop_back();
        names.emplace_back(std::move(s));
      }
    }
    // if all the names are empty, these are unnamed channels, and can just return an empty vector for the names
    auto isEmpty=[](const std::string & s){return s.empty();};
    if ( std::all_of(names.begin(), names.end(), isEmpty)) names.clear();
    return std::make_pair(std::move(list), std::move(names));
  }
  
  std::string 
  folderDescriptionForTag(const std::string & tag, const bool testing){
    std::string jsonReply{R"delim({"format":"TagMetaSetDto","resources":[{"tagName":"LARAlign-RUN2-UPD4-03","description":"{\"dbname\":\"CONDBR2\",\"nodeFullpath\":\"/LAR/Align\",\"schemaName\":\"COOLONL_LAR\"}","chansize":1,"colsize":1,"tagInfo":"{\"channel_list\":[{\"0\":\"\"}],\"node_description\":\"<timeStamp>run-lumi</timeStamp><addrHeader><address_header service_type=\\\"256\\\" clid=\\\"1238547719\\\" /></addrHeader><typeName>CondAttrListCollection</typeName><updateMode>UPD1</updateMode>\",\"payload_spec\":\"PoolRef:String4k\"}","insertionTime":"2022-05-26T12:10:38+0000"}],"size":1,"datatype":"tagmetas","format":null,"page":null,"filter":null})delim"};
    if (not testing){
      auto myCrestClient = Crest::CrestClient(urlBase());
      jsonReply= myCrestClient.getTagMetaInfo(tag).dump();
    }
    return extractDescriptionFromJson(jsonReply);
  }
  
  std::string 
  payloadSpecificationForTag(const std::string & specTag, const bool testing){
    std::string jsonReply{R"delim({"folder_payloadspec": "PoolRef: String4k"})delim"};
    if (not testing){
      auto myCrestClient = Crest::CrestClient(urlBase());
      jsonReply= myCrestClient.getTagMetaInfo(specTag).dump();
    }
    return extractSpecificationFromJson(jsonReply);
  }
  
  std::pair<std::vector<cool::ChannelId> , std::vector<std::string>>
  channelListForTag(const std::string & tag, const bool testing){
    const auto channelListTag=tag;
    std::string reply{R"delim([{"chansize":8,"colsize":5,"description":"{\"dbname\":\"CONDBR2\",\"nodeFullpath\":\"/LAR/BadChannelsOfl/BadChannels\",\"schemaName\":\"COOLOFL_LAR\"}","insertionTime":"2022-05-26T16:40:32+0000","tagInfo":"{\"channel_list\":[{\"0\":\"\"},{\"1\":\"\"},{\"2\":\"\"},{\"3\":\"\"},{\"4\":\"\"},{\"5\":\"\"},{\"6\":\"\"},{\"7\":\"\"}],\"node_description\":\"<timeStamp>run-lumi</timeStamp><addrHeader><address_header service_type=\\\"71\\\" clid=\\\"1238547719\\\" /></addrHeader><typeName>CondAttrListCollection</typeName>\",\"payload_spec\":\"ChannelSize:UInt32,StatusWordSize:UInt32,Endianness:UInt32,Version:UInt32,Blob:Blob64k\"}","tagName":"LARBadChannelsOflBadChannels-RUN2-UPD4-21"}])delim"};
    if (not testing){
     auto myCrestClient = Crest::CrestClient(urlBase());
     reply= myCrestClient.getTagMetaInfo(tag).dump();
    }
    return extractChannelListFromJson(reply);
  }
  
  std::string
  resolveCrestTag(const std::string & globalTagName, const std::string & folderName, const std::string & forceTag, const bool testing){
    std::string result{};
    if (not forceTag.empty()) return forceTag;
    if (testing) return "LARAlign-RUN2-UPD4-03";
    auto crestClient = Crest::CrestClient(urlBase());
    auto j = crestClient.findGlobalTagMap(globalTagName);
    for (const auto &i:j){
      if (i["label"] == folderName){
        result=static_cast<std::string>(i["tagName"]);
        break;
      }
    }
    return result;
  }
  
  std::string
  jsonTagName(const std::string &globalTag, const std::string & folderName){
    return resolveCrestTag(globalTag,folderName);
  }
  
}
