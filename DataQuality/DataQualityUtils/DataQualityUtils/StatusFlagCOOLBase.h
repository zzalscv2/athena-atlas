/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef dqutilsStatusFlagCOOLBase_h
#define dqutilsStatusFlagCOOLBase_h

// Protect CINT from some system definitions that cause problems
#ifndef __CINT__
  //COOL API include files (CoolKernel)
  #include "CoolKernel/pointers.h"
  #include "CoolKernel/ValidityKey.h"
#else
  namespace cool {
    class IDatabasePtr;
    class IFolderPtr;
  }
#endif


#include <iostream>
#include <string>
#include <cstdlib>

#include <TObject.h>

//CORAL API include files
#include "CoralBase/AttributeList.h"

//COOL API include files (CoolApplication)
#include "CoolApplication/Application.h"
// --> limits.h is needed for CoolKernel/types.h
#include <limits.h>
#include "CoolKernel/types.h"
#include "CoolKernel/ChannelId.h"
#include "CoolKernel/RecordSpecification.h"
#include "CoolKernel/ChannelSelection.h"


namespace coral {
  class AttributeList;
}

namespace cool {
  class RecordSpecification;
  class ChannelSelection;
}


namespace dqutils {

class StatusFlagCOOLBase : public cool::Application, public TObject {
protected:

// Protect CINT from some system definitions that cause problems
// CINT does not need to know about these private variables
#ifndef __CINT__
    cool::ValidityKey m_since;
    cool::ValidityKey m_until;
    cool::IDatabasePtr m_coolDb;
    cool::IFolderPtr m_coolFolder;
#endif

    void insert_helper(cool::ChannelId channelId, 
		       coral::AttributeList& payload,
		       std::string& tag_name);
    void Initialize(const std::string& dbStr, const std::string& folderStr, 
		    int runS, int lumiS, int runU, int lumiU);

public:
    // Connects to the database. Throws a "DatabaseDoesNotExist" exception if database does not exist.
    cool::IDatabasePtr coolDbInstance(const std::string& dbStr, bool readOnly);
    
    // Browses the COOL folder. Throws a "FolderNotFound" exception if folder does not exist.
    cool::IFolderPtr coolFolderInstance(const std::string& folderStr);

    // Various methods to set and print the intervall of validity.
    void setSince(cool::Int64 run, cool::Int64 lumi);
    void setUntil(cool::Int64 run, cool::Int64 lumi);
    void printIOV();
    void setIOV(cool::Int64 runS, cool::Int64 lumiS, cool::Int64 runU, cool::Int64 lumiU);
    void setIOV(cool::Int64 run);

    // to flush after insertions!
    void flush();
   
    // Constructors and Destructors.
    StatusFlagCOOLBase(const std::string& dbStr, const std::string& folderStr, int runS, int lumiS, int runU, int lumiU);
    StatusFlagCOOLBase(int runS, int lumiS, int runU, int lumiU);
    StatusFlagCOOLBase();
    virtual ~StatusFlagCOOLBase ();

    // Methods of "interest". To insert a status flag you can use either ChannelId or ChannelName.
    void dump(cool::ChannelSelection selection, std::string tag_name);
    std::string dumpField(cool::ChannelId channelId, std::string field, std::string tag_name);
    int dumpCode(const std::string& channelName, const std::string& tag_name);
    void dumpall(const std::string& tag_name);

    // Some further getters.
    cool::IFolderPtr getCoolFolder();
    cool::IDatabasePtr getCoolDb();


    // Needed for the ROOT interface.
    ClassDef( StatusFlagCOOLBase, 0 ) // A class for modifying DQ info in the COOL database
};

}

#endif
