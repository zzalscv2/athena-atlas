/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/* Standalone application to produce a reference file from the pixel TOT calibration DB */

#include "CoolKernel/DatabaseId.h"
#include "CoolKernel/Exception.h"
#include "CoolKernel/IDatabaseSvc.h"
#include "CoolKernel/IDatabase.h"
#include "CoolApplication/Application.h"
#include "CoolKernel/IFolder.h"
#include "CoolKernel/FolderSpecification.h"
#include "CoolKernel/RecordSpecification.h"
#include "CoolKernel/Record.h"
#include "CoolKernel/FieldSpecification.h"
#include "CoolKernel/IObject.h"
#include "CoolKernel/IObjectIterator.h"
#include "CoolKernel/IRecordIterator.h"
#include "CoolKernel/StorageType.h"
#include "CoolKernel/ChannelSelection.h"
//
#include "RelationalAccess/ConnectionService.h"
//STL
#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <stdexcept>
#include <ctime>
#include <sstream>

#ifdef ATLAS_GCC_CHECKERS
  #include "CxxUtils/checker_macros.h"
  ATLAS_NO_CHECK_FILE_THREAD_SAFETY;
#endif



class DbConnection{
 public:
   DbConnection(const std::string & sourceDb);
   ~DbConnection();
   bool isOpen() const;
   cool::IDatabasePtr dbPtr() const;
  
 private:
   coral::ConnectionService m_coralsvc{};
   cool::Application m_coolapp;
   const cool::IDatabaseSvc & m_dbSvc;
   cool::IDatabasePtr m_sourceDbPtr{};
   bool m_isOpen{};
 };
 

//make db connection upon construction
DbConnection::DbConnection(const std::string & sourceDb):
    m_coolapp(&m_coralsvc),
    m_dbSvc(m_coolapp.databaseService()){
  bool readOnly(true);
  const std::string dbConnectionString{sourceDb};
  try {
    m_sourceDbPtr=m_dbSvc.openDatabase(dbConnectionString,readOnly);
  }
  catch (std::exception& e) {
    std::cout << "Cool exception caught: " << e.what() << std::endl;
    if (not readOnly){
      try {
        std::cout<<"creating "<<dbConnectionString<<std::endl;
        m_sourceDbPtr=m_dbSvc.createDatabase(dbConnectionString);
      } catch (std::exception& e){
        std::cout << "Cool exception caught: " << e.what() << std::endl;
      }
    }
  }
  m_isOpen=(m_sourceDbPtr!=nullptr);
}

//break db connection on destruction
DbConnection::~DbConnection(){
  if (isOpen()){
      m_sourceDbPtr->closeDatabase();
    }
}

//return bare db pointer
cool::IDatabasePtr
DbConnection::dbPtr() const{
  return m_sourceDbPtr;
}

bool
DbConnection::isOpen() const{
  return m_isOpen;
}

//specification to identify a given folder from full name and tag
struct FolderSpec{
  const std::string name;
  const std::string tag;
  FolderSpec(const std::string & thename, const std::string & thetag):name(thename),tag(thetag){
   /**nop**/};
};

 //Folder interface class
class Folder{
public:
  Folder(const DbConnection & theConnection, const FolderSpec & theFolder);
  bool isValid() const;
  std::string description() const;
  FolderSpec folderSpec() const;
  cool::RecordSpecification foreignKeySpec();
  cool::RecordSpecification payloadSpec() const; //!< specification of the payload entries
  bool isSingleVersion() const;
  std::string tag() const;
  const cool::IFolderPtr & ptr() const;
  cool::IObjectIteratorPtr objectIterator() const;
  std::vector<cool::ChannelId> channels() const;
  std::vector<std::string> tags() const;
private:
  const DbConnection & m_connection;
  const FolderSpec m_folderSpec;
  cool::IFolderPtr m_folderPtr;
};

Folder::Folder(const DbConnection & theConnection, const FolderSpec & theFolder)
  :m_connection(theConnection),m_folderSpec(theFolder),m_folderPtr{}{
  try{
    m_folderPtr=m_connection.dbPtr()->getFolder(theFolder.name);//!< get the cool folder
  } catch (cool::Exception & e){
    std::cout<<" Could not get folder "<<theFolder.name<<std::endl;
  }
}

FolderSpec
Folder::folderSpec() const{
    return m_folderSpec;
}

const cool::IFolderPtr &
Folder::ptr() const{
  return m_folderPtr;
}

//assume the coracool folder is valid if the pointer to it is valid
bool
Folder::isValid() const{
  bool result = (m_folderPtr != nullptr);
  return result;
}

bool
Folder::isSingleVersion() const{
  return (cool::FolderVersioning::SINGLE_VERSION == m_folderPtr->versioningMode());
}

std::string
Folder::description() const{
    return (m_folderPtr->description());
}

cool::RecordSpecification 
Folder::payloadSpec() const{
  return m_folderPtr->payloadSpecification();
}

std::string
Folder::tag() const{
  return m_folderSpec.tag;
}

cool::IObjectIteratorPtr 
Folder::objectIterator() const{
  //iovs to browse
  const auto lo = cool::ValidityKeyMin;
  const auto hi = cool::ValidityKeyMax;
  const auto channelSelection = cool::ChannelSelection::all();
  return m_folderPtr->browseObjects(lo, hi, channelSelection, m_folderSpec.tag);
}

std::vector<cool::ChannelId> 
Folder::channels() const{
  return m_folderPtr->listChannels(); 
}

std::vector<std::string> 
Folder::tags() const{
  return m_folderPtr->listTags(); 
}

std::string 
timeRep(const cool::ValidityKey &t, bool isEnd = false, bool runLumi=true){
  std::string result;
  const std::string trail = isEnd?(")"):("]");
  if (not runLumi){ //ns of epoch
    if (t == cool::ValidityKeyMin){
      result = "ValidityKeyMin";
    } else if (t == cool::ValidityKeyMax){
      result = "ValidityKeyMax";
    } else {
      long int seconds = static_cast<long int> (t/1000000000);
      std::string timeStr{std::asctime(std::gmtime(&seconds))};
      result = timeStr + " UTC";
    }
  } else {//runLumi
    auto run = t >>32;
    auto lumiblock = t & 0xFFFFFFFF;
    result ="[" +std::to_string(run)+","+std::to_string(lumiblock)+trail;
  }
  return result;
}
  
std::string
iovToString(const cool::IObject & obj, bool runLumi = true){
  std::string sinceStr = timeRep(obj.since(), false, runLumi);
  std::string untilStr = timeRep(obj.until(), true, runLumi);
  return sinceStr + " - " + untilStr;
}


std::string
payloadToString(const cool::IObject & obj){
  std::string result;
  const cool::IRecord & record = obj.payload();
  const auto & thisSpec = record.specification();
  for (cool::UInt32 i=0;i!=thisSpec.size();++i){
    const std::string delimiter = (i==0)?(""):(",");
    std::string typeName = thisSpec[i].storageType().name();
    result+= " ["+thisSpec[i].name() + " (" + typeName + ") : ";
    std::ostringstream os;
    record[i].printValue(os);
    result +=os.str();
  }
  result +="]";
  return result;
}




//defaults
namespace{
  const std::string  defaultTagName{"PixCalib-DATA-RUN2-UPD4-21"};
  const std::string  folderName{"/PIXEL/PixCalib"};
  const std::string  outputFileName{"PixCalib-DATA-RUN2-UPD4-21.log"};
  const std::string  dbName{"COOLOFL_PIXEL/CONDBR2"};
}


int main(int argc, char* argv[]) { 
  std::string tagName = defaultTagName;
  if (argc==2){
    tagName = std::string(argv[1]);
    std::cout<<"Using command line tag name"<<std::endl;
  }
  std::cout<<"Using tag selection: "<<tagName<<std::endl;
  int returnCode=0;
  DbConnection connection(dbName);
  FolderSpec fs(folderName,tagName);
  Folder f(connection, fs);
  const std::string fileName = tagName + ".log";
  std::ofstream opFile(fileName);
  auto objectsIterator = f.objectIterator();
  while (objectsIterator->goToNext()){
    const auto & thisObject = objectsIterator->currentRef();
    std::string display = iovToString(thisObject) + " (" + std::to_string(thisObject.channelId()) + ")";
    display += payloadToString(thisObject);
    opFile<<display<<"\n";
  }
  opFile<<std::endl;
  opFile.close();
  return returnCode;
}