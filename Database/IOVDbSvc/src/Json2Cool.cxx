/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "Json2Cool.h"

#include "IOVDbStringFunctions.h"
#include "CoolKernel/RecordSpecification.h"
#include "CoolKernel/Record.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CxxUtils/checker_macros.h"
#include "boost/regex.hpp"
#include "Base64Codec.h"
#include <stdexcept>
#include <iostream>

using json = nlohmann::json;
using namespace cool;
using namespace IOVDbNamespace;


namespace {


const std::map<std::string, cool::StorageType::TypeId> typeCorrespondance={ 
    //http://cool-doxygen.web.cern.ch/COOL-doxygen/classcool_1_1_storage_type.html
      {"Bool", StorageType::Bool},
      {"UChar",StorageType::UChar},
      {"Int16", StorageType::Int16},
      {"UInt16", StorageType::UInt16},
      {"Int32", StorageType::Int32},
      {"UInt32", StorageType::UInt32},
      {"UInt63",StorageType::UInt63},
      {"Int64", StorageType::Int64},
      {"Float", StorageType::Float},
      {"Double", StorageType::Double},
      {"String255", StorageType::String255},
      {"String4k", StorageType::String4k},
      {"String64k", StorageType::String64k},
      {"String16M", StorageType::String16M},
      {"Blob64k", StorageType::Blob64k},
      {"Blob16M", StorageType::Blob16M}
    };


} // anonymous namespace
  
  
namespace IOVDbNamespace{


  Json2Cool::Json2Cool(std::istream & stream, BasicFolder & b, const std::string & specString, const IovStore::Iov_t* iov):m_basicFolder(b){
  init(stream, specString, iov);
  }
  
  void
  Json2Cool::init(std::istream & s, const std::string & specString, const IovStore::Iov_t* iov){
    if (not s.good() or s.eof()){
      const std::string msg("Json2Cool constructor; Input is invalid and could not be opened.");
      throw std::runtime_error(msg);
    } else {
      json j;
      try{
        s>>j; //read into json
      }catch (const std::exception& e) {
        std::cout<<"ERROR AT LINE "<<__LINE__<<" of "<<__FILE__<<std::endl;
        std::cout<<e.what()<<std::endl; //typically a parsing error
      }
      m_sharedSpec = parsePayloadSpec(specString);
      const auto & payload=j["data"];//payload is an object in any case, of form {"0":["datastring"]}
      //keep these lines for reference: iov handling is not yet implemented, but should be
      //m_basicFolder.setVectorPayloadFlag(m_isVectorPayload);
      //const auto & iovFromFile=j["iov"];//iov is a two-element array
      //const std::pair<cool::ValidityKey, cool::ValidityKey> iov(iovFromFile[0], iovFromFile[1]);
      m_isVectorPayload = false;
      if(iov) {
	m_basicFolder.setIov(*iov);
      }
      else {
	m_basicFolder.setIov(IovStore::Iov_t(0, cool::ValidityKeyMax));
      }
      if (m_isVectorPayload){
        for (json::const_iterator k=payload.begin();k!=payload.end();++k){ //k are {"0":}
          const json& f=k.value(); //channel id
          std::vector<coral::AttributeList> tempVector;//can optimise this by pre constructing it and using 'clear'
          for (json::const_iterator i=f.begin();i!=f.end();++i){
            const std::string& keyString=i.key();
            const long long key=std::stoll(keyString);
            const auto & val=i.value();
            for (const auto & aList:val){        
              auto r=createAttributeList(m_sharedSpec,aList);
              const auto & attList=r.attributeList();
              tempVector.push_back(attList);
            }
            m_basicFolder.addChannelPayload(key, tempVector);
          }
          //add payload with channelId here
        }
      } else {
        for (json::const_iterator i=payload.begin();i!=payload.end();++i){
          const json& f=i.value();
          const std::string& ks=i.key();
          const long long key=std::stoll(ks);
          auto r=createAttributeList(m_sharedSpec,f);
          const auto & attList=r.attributeList();
          m_basicFolder.addChannelPayload(key, attList);
        }
      }
    }
  }
  
  //parsing something like
  // "folder_payloadspec": "crate: UChar, slot: UChar, ROB: Int32, SRCid: Int32, BCIDOffset: Int16, slave0: Int32, slave1: Int32, slave2: Int32, slave3: Int32"
  cool::RecordSpecification *
  Json2Cool::parsePayloadSpec(const std::string & stringSpecification){
    if (stringSpecification.empty()) return nullptr;
    std::string input(stringSpecification);
    auto *spec = new cool::RecordSpecification();
    
    std::string regex=R"delim(([^\s,:]*):\s?([^\s,]*),?)delim";
    boost::regex expression(regex);
    boost::smatch what;
    
    bool match=boost::regex_search(input, what, expression);
    while (match){
      std::string n(what[1]);
      std::string t(what[2]);
      //todo: need to catch error if type not found, also
      spec->extend(n, typeCorrespondance.find(t)->second);
      input = what.suffix();
      match=boost::regex_search(input, what, expression);
      
    }
    return spec;
  }
  
  cool::Record 
  Json2Cool::createAttributeList(cool::RecordSpecification * pSpec, const nlohmann::json & j){
    cool::Record a(*pSpec);
    unsigned int s=a.size();
    
    json::const_iterator it = j.begin();
    for (unsigned int i(0);i!=s;++i){
      auto & f=a[i];
      if (it == j.end()){
        continue;
      }
      const auto  thisVal = it.value();
      ++it;
      
      try{
        // cool::Record does not provide non-const access to AttributeList.
        // But this is safe because we are filling a local instance.
        auto & att ATLAS_THREAD_SAFE = const_cast<coral::Attribute&>(a.attributeList()[i]);
        if (thisVal.is_null()){
          att.setNull();
          continue;
        }
	
	cool::StorageType::TypeId typespec = f.storageType().id();

        switch (typespec) {
	case StorageType::Bool:
	  {
	    const bool newVal=(thisVal.get<std::string>() == "true");
	    att.setValue<bool>(newVal);
	    break;
	  }
	case StorageType::UChar:
	  {
	    const unsigned char newVal=std::stoul(thisVal.get<std::string>());
	    att.setValue<unsigned char>(newVal);
	    break;
	  }
	case StorageType::Int16:
	  {
	    const short newVal=std::stol(thisVal.get<std::string>());
	    att.setValue<short>(newVal);
	    break;
	  }
	case StorageType::UInt16:
	  {
	    const unsigned short newVal=std::stoul(thisVal.get<std::string>());
	    att.setValue<unsigned short>(newVal);
	    break;
	  }
	case StorageType::Int32:
	  {
	    const int newVal=std::stoi(thisVal.get<std::string>());
	    att.setValue<int>(newVal);
	    break;
	  }
	case StorageType::UInt32:
	  {
	    const std::string valString=thisVal.get<std::string>();
	    const unsigned int newVal=std::stoull(valString);
	    att.setValue<unsigned int>(newVal);
	    break;
	  }
	case StorageType::UInt63:
	  {
	    const unsigned long long newVal=thisVal;
	    att.setValue<unsigned long long>(newVal);
	    break;
	  }
	case StorageType::Int64:
	  {
	    const  long long newVal=std::stoll(thisVal.get<std::string>());
	    att.setValue< long long>(newVal);
	    break;
	  }
	case StorageType::Float:
	  {
	    const  float newVal=std::stof(thisVal.get<std::string>());;
	    att.setValue<float>(newVal);
	    break;
	  }
	case StorageType::Double:
	  {
	    const  double newVal=std::stod(thisVal.get<std::string>());
	    att.setValue<double>(newVal);
	    break;
	  }
	case StorageType::String255:
	case StorageType::String4k:
	case StorageType::String64k:
	case StorageType::String16M:
	  {
	    const  std::string newVal=thisVal;
	    att.setValue<std::string>(newVal);
	    break;
	  }
	case StorageType::Blob16M:
	case StorageType::Blob64k:
	  {
	    const std::string & s = thisVal.get<std::string>();
	    auto blob = base64Decode(s);
	    att.setValue<coral::Blob>(blob);
	    break;
	  }
	default:
	  {
	    std::string typeName{};
	    for (auto& [key,val] : typeCorrespondance) {
	      if(val==typespec) {
		typeName = key;
		break;
	      }
	    }
	    if(typeName.empty()) {
	      typeName = "Unexpected Type";
	    }
	    std::string errorMessage("UNTREATED TYPE! " + typeName);
	    std::cerr << errorMessage << std::endl;
	    throw std::runtime_error(errorMessage);
	    break;
	  }
	}
      } 
      catch (json::exception& e){
        std::cerr << e.what() << std::endl;
	throw std::runtime_error(e.what());
      }
    }
    return a;
  }
}//end of namespace

