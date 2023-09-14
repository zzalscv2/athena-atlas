/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//*************************************************
// Class for the MDT interface with the COOL DB
// author Monica Verducci monica.verducci@cern.ch
//************************************************

#ifndef dqutilsCoolMdt_h
#define dqutilsCoolMdt_h

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



#include <sstream>
#include <fstream>
#include <TCanvas.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TH1.h>
#include <TH2.h>
#include <TIterator.h>
#include <TKey.h>
#include <TLegend.h>
#include <TProfile.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TLatex.h>
#include <TMath.h>
#include <TTree.h>


namespace coral {
  class AttributeList;
}

namespace cool {
  class RecordSpecification;
  class ChannelSelection;
}


namespace dqutils {

class CoolMdt : public cool::Application, public TObject {
private:


// Protect CINT from some system definitions that cause problems
// CINT does not need to know about these private variables
#ifndef __CINT__
    cool::ValidityKey m_since;
    cool::ValidityKey m_until;
    cool::IDatabasePtr m_coolDb;
    cool::IFolderPtr m_coolFolder;
    bool m_fist_folder;
#endif

public:

    // Connects to the database. Throws a "DatabaseDoesNotExis" exception if database does not exist.
    cool::IDatabasePtr coolDbInstance(const std::string& dbStr, bool readOnly);
        
    
    // Browses the COOL folder. Throws a "FolderNotFound" exception if folder does not exist.
    cool::IFolderPtr coolFolderInstance(const std::string& folderStr);
    // Various methods to set and print the intervall of validity.
    
    void initialize();
    void coolDbFolder(const std::string& dbStr, const std::string& folderStr);
    void setSince(cool::Int64 run, cool::Int64 lumi);
    void setUntil(cool::Int64 run, cool::Int64 lumi);
    void printIOV();
    void setIOV(cool::Int64 runS, cool::Int64 lumiS, cool::Int64 runU, cool::Int64 lumiU);
    void setIOV(cool::Int64 run);

    // Methods needed to come up to COOL framework.
    cool::RecordSpecification createSpecDataDead();
    cool::RecordSpecification createSpecDataNoisy();
    coral::AttributeList  createPayloadDataNoisy(const std::string& ChamberName,
                                                 const std::string&  NoisyMultilayer,
                                                 const std::string& NoisyLayer,
                                                 const std::string& NoisyMezz,
                                                 const std::string& NoisyAsd,
                                                 const std::string& NoisyTube,
                                                 const cool::RecordSpecification& spec); 
    coral::AttributeList  createPayloadDataDead(const std::string& ChamberName,
                                                const std::string&  DeadMultilayer,
                                                const std::string& DeadLayer,
                                                const std::string& DeadMezz,
                                                const std::string& DeadAsd,
                                                const std::string& DeadTube,
                                                const cool::RecordSpecification& spec); 

    // Constructors and Destructors.
    void CoolOpen(const std::string& dbStr);

    //CoolMdt();
    virtual ~CoolMdt ();


    void dump(cool::ChannelSelection selection);
    std::string dumpField(cool::ChannelId channelId, std::string field);
    int dumpCode(const std::string& channelName);
    
    void dumpall();

    void insertNoisyFlag(cool::Int64 run,
                         cool::ChannelId channelId,
                         const std::string& ChamberName,
                         const std::string& NoisyMultilayer,
                         const std::string& NoisyLayer,
                         const std::string& NoisyMezz,
                         const std::string& NoisyAsd,
                         const std::string& NoisyTube);
    void insertNoisyFlag_withTag(cool::Int64 run,
                                 cool::ChannelId channelId,
                                 const std::string& ChamberName,
                                 const std::string&  NoisyMultilayer,
                                 const std::string& NoisyLayer,
                                 const std::string& NoisyMezz,
                                 const std::string& NoisyAsd,
                                 const std::string& NoisyTube,
                                 const std::string& cool_tag);

    void insertDeadFlag(cool::Int64 run,
                        cool::ChannelId channelId,
                        const std::string& ChamberName,
                        const std::string& DeadMultilayer,
                        const std::string& DeadLayer,
                        const std::string& DeadMezz,
                        const std::string& DeadAsd,
                        const std::string& DeadTube);
    void insertDeadFlag_withTag(cool::Int64 run,
                                cool::ChannelId channelId,
                                const std::string& ChamberName,
                                const std::string& DeadMultilayer,
                                const std::string& DeadLayer,
                                const std::string& DeadMezz,
                                const std::string& DeadAsd,
                                const std::string& DeadTube,
                                const std::string& cool_tag);

    cool::IFolderPtr getCoolFolder();
    cool::IDatabasePtr getCoolDb();


    // Needed for the ROOT interface.
    ClassDef( CoolMdt, 0 ) // A class for modifying DQ info in the COOL database
};

}

#endif
