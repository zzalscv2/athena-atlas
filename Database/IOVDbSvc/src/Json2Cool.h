/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/
#ifndef IOVDbSvc_Json2Cool_h
#define IOVDbSvc_Json2Cool_h

#include "CoolKernel/StorageType.h"
#include <string>
#include <istream>
#include <map>
#include "nlohmann/json.hpp"
#include "BasicFolder.h"
#include "IovStore.h"

namespace cool{
  class RecordSpecification;
  class Record;
}


namespace IOVDbNamespace {

  //@brief Produces cool objects from their JSON representation, storing them in a 'BasicFolder'
  class Json2Cool {
  public:
    
    Json2Cool(std::istream & stream, BasicFolder & b, const std::string &specString, const IovStore::Iov_t* iov = nullptr);
    ~Json2Cool() = default;
    
    static cool::Record 
    createAttributeList(cool::RecordSpecification * pSpec, const nlohmann::json & j);
    
    static cool::RecordSpecification *
    parsePayloadSpec(const std::string & stringSpecification);
    
  private:
    void init(std::istream & stream, const std::string & specString, const IovStore::Iov_t* iov);
    cool::RecordSpecification * m_sharedSpec = nullptr;
    //
    bool m_isVectorPayload = false;
    //
    BasicFolder &m_basicFolder;
    

  };

}
#endif

