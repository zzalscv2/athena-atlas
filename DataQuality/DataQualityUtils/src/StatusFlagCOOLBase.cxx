/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// **********************************************************************
// $Id: StatusFlagCOOLBase.cxx,v 1.6 2009-02-13 12:32:11 ponyisi Exp $
// **********************************************************************

#include "DataQualityUtils/StatusFlagCOOLBase.h"

//CORAL API include files
#include "CoralBase/Attribute.h"

//COOL API include files (CoolKernel)
#include "CoolKernel/IDatabase.h"
#include "CoolKernel/IFolder.h"
#include "CoolKernel/IObjectIterator.h"
#include "CoolKernel/IObject.h"
#include "CoolKernel/Record.h"
#include "CoolKernel/Exception.h"
#include "CoolKernel/IDatabaseSvc.h"
#include "CoolKernel/StorageType.h"
#include "CoolKernel/ConstRecordAdapter.h"

ClassImp(dqutils::StatusFlagCOOLBase)

namespace dqutils{

cool::IDatabasePtr 
StatusFlagCOOLBase::
coolDbInstance(const std::string& dbStr, bool readOnly) {
    try {
        std::cout << "Opening database '" << dbStr << "'...";
        cool::IDatabaseSvc& dbSvc = this->databaseService();
        std::cout << "done." << std::endl;
        return dbSvc.openDatabase(dbStr.c_str(), readOnly);
    }
    catch (cool::DatabaseDoesNotExist&) {
        std::cout << "Error! Database does not exist!" << std::endl;
        throw;
    }
}

cool::IFolderPtr 
StatusFlagCOOLBase::
coolFolderInstance(const std::string& folderStr) {
    try {
        cool::IFolderPtr folder = m_coolDb->getFolder(folderStr.c_str());
        std::cout << "Browsing objects of '" << folderStr << "'" << std::endl;
	//folder->setupStorageBuffer();
        return folder;
    }
    catch (cool::FolderNotFound& ) {
        std::cout << "Error! Folder '" << folderStr << "' does not exist!" << std::endl;
        throw;
    }
}

void 
StatusFlagCOOLBase::
setSince(cool::Int64 run, cool::Int64 lumi) {
    m_since = ((run << 32) + lumi);
}

void 
StatusFlagCOOLBase::
setUntil(cool::Int64 run, cool::Int64 lumi) {
    m_until = ((run << 32) + lumi);
}

void
StatusFlagCOOLBase::
setIOV(cool::Int64 runS, cool::Int64 lumiS, cool::Int64 runU, cool::Int64 lumiU) {
    this->setSince(runS, lumiS);
    this->setUntil(runU, lumiU);
    //this->printIOV();
}

void
StatusFlagCOOLBase::
setIOV(cool::Int64 run) {
    this->setSince(run, 0);
    this->setUntil(run, cool::UInt32Max);
    //this->printIOV();
}


void
StatusFlagCOOLBase::
printIOV(){
    cool::Int64 runS=m_since>>32;
    cool::Int64 lumiS=m_since-(runS<<32);
    cool::Int64 runU=m_until>>32;
    cool::Int64 lumiU=m_until-(runU<<32);
    std::cout << "Using IOVrange [(" << runS << "," << lumiS << "),("  << runU << "," << lumiU << ")[ " << "[" << m_since << "," << m_until << "[" << std::endl;
}

void
StatusFlagCOOLBase::
flush() {
  //m_coolFolder->flushStorageBuffer();
}

void
StatusFlagCOOLBase::
Initialize(const std::string& dbStr, const std::string& folderStr, int runS, int lumiS, int runU, int lumiU) {
  m_coolDb = this->coolDbInstance(dbStr, false);
  m_coolFolder = this->coolFolderInstance(folderStr);
  this->setIOV(runS, lumiS, runU, lumiU);
}

  
StatusFlagCOOLBase::
StatusFlagCOOLBase (const std::string& dbStr, const std::string& folderStr, int runS, int lumiS, int runU, int lumiU) {
  Initialize(dbStr, folderStr, runS, lumiS, runU, lumiU);
}

StatusFlagCOOLBase::
StatusFlagCOOLBase(int runS, int lumiS, int runU, int lumiU) {
  Initialize("COOLOFL_GLOBAL/OFLP200", "/GLOBAL/DETSTATUS/SHIFTOFL", 
	     runS, lumiS, runU, lumiU);
}

StatusFlagCOOLBase::
StatusFlagCOOLBase() {
  Initialize("COOLOFL_GLOBAL/OFLP200", "/GLOBAL/DETSTATUS/SHIFTOFL", 
	     0, 0, 0, 0);
}  

StatusFlagCOOLBase::
~StatusFlagCOOLBase () {
  //m_coolFolder->flushStorageBuffer();
    m_coolDb->closeDatabase();
    std::cout << "Cleared!" << std::endl;
}

void 
StatusFlagCOOLBase::
dump(cool::ChannelSelection selection, std::string tag_name) {      
    try {
        cool::IObjectIteratorPtr objects = m_coolFolder->browseObjects(m_since, m_until-1, selection, tag_name);
        while (objects->goToNext()) {
            const cool::IObject& element = objects->currentRef();
            std::cout << element << std::endl;
        }
    }
    catch (cool::Exception& e) {
        std::cout << "Unknown exception caught!" << e.what() << std::endl;
    }
}

std::string 
StatusFlagCOOLBase::
dumpField(cool::ChannelId channelId, std::string field, std::string tag_name) {      
    std::string result ="";
    try {
        cool::ChannelSelection selection = cool::ChannelSelection(channelId);
        cool::IObjectIteratorPtr objects = m_coolFolder->browseObjects(m_since, m_until-1, selection, tag_name);
        while (objects->goToNext()) {
            const cool::IObject& element = objects->currentRef();
            result = element.payloadValue(field);
        }
    }
    catch (cool::Exception& e) {
        std::cout << "Unknown exception caught!" << e.what() << std::endl;
    }
    return result;
}

int
StatusFlagCOOLBase::
dumpCode(const std::string& channelName, const std::string& tag_name) {
    std::string result = this->dumpField(this->getCoolFolder()->channelId(channelName.c_str()), "Code", tag_name);
    if (result == "") {
      return INT_MAX;
    } else {
      return atoi(result.c_str());
    }
}

void 
StatusFlagCOOLBase::
dumpall(const std::string& tag_name) {      
  this->dump(cool::ChannelSelection::all(), tag_name);
}

void
StatusFlagCOOLBase::
insert_helper(cool::ChannelId channelId, coral::AttributeList& payload,
	      std::string& tag_name) {
  cool::ConstRecordAdapter record (m_coolFolder->payloadSpecification(), payload);
  if (tag_name=="HEAD") {
    m_coolFolder->storeObject(m_since, m_until, cool::Record(m_coolFolder->payloadSpecification(), payload), channelId);
  } else {
    m_coolFolder->storeObject(m_since, m_until, cool::Record(m_coolFolder->payloadSpecification(), payload), channelId, tag_name, true);
  }
}

cool::IFolderPtr 
StatusFlagCOOLBase::
getCoolFolder() {
    return this->m_coolFolder;
}

cool::IDatabasePtr 
StatusFlagCOOLBase::
getCoolDb() {
    return this->m_coolDb;
}


} //namespace dqutils
