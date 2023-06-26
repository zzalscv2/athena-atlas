/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENASERVICES_TEST_METADATATOOLSTUB_H
#define ATHENASERVICES_TEST_METADATATOOLSTUB_H

#include "AthenaKernel/IMetaDataTool.h"
#include "AthenaBaseComps/AthAlgTool.h"


#include <string>
/**the following is in AthenaKernel/​SourceID.h
namespace SG{
  typedef std::string SourceID;
}
**/
class MetaDataToolStub : public AthAlgTool, virtual public IMetaDataTool{
  public:
  MetaDataToolStub(const std::string& type, const std::string& name, const IInterface* parent): 
   AthAlgTool(type, name, parent){
    declareInterface<IMetaDataTool>(this);
 }
  virtual ~MetaDataToolStub()= default;
  virtual StatusCode initialize() {return StatusCode::SUCCESS;}
  virtual StatusCode beginInputFile()  {return StatusCode::SUCCESS;}
  virtual StatusCode endInputFile() {return StatusCode::SUCCESS;}
  virtual StatusCode metaDataStop() {return StatusCode::SUCCESS;}
  //
  virtual StatusCode 
  beginInputFile(const SG::SourceID& guid) {
    if (guid == "badGuid") {
      ATH_MSG_ERROR("Failing");
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }
  //
  virtual StatusCode 
  endInputFile(const SG::SourceID& guid) {
    if (guid == "badGuid") {
      ATH_MSG_ERROR("Failing");
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }
  virtual StatusCode metaDataStop(const SG::SourceID&) {return metaDataStop();}
};

#endif