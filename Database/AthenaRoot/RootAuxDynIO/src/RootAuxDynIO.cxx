/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "AthContainers/tools/error.h"
#include "AthContainers/exceptions.h"

#include "RootAuxDynIO/RootAuxDynIO.h"
#include "TBranchAuxDynReader.h"
#include "TBranchAuxDynWriter.h"

#include "TFile.h"
#include "TBranch.h"
#include "TClass.h"
#include "TROOT.h"
#include "TDictAttributeMap.h"


class TTree;

namespace RootAuxDynIO
{

   std::string
   getKeyFromBranch(TBranch* branch)
   {
      TClass *tc = 0;
      EDataType type;
      if( branch->GetExpectedType(tc, type) == 0  && tc != nullptr) {
         const char* brname = branch->GetName();
         const char* clname = tc->GetName();
         size_t namelen = strlen (clname);
         std::string key = brname;
         if( strncmp(brname, clname, namelen) == 0 && brname[namelen] == '_' ) {
            key.erase (0, namelen+1);
         }

         if (key.size() >= 5 && key.substr(key.size()-4, 4) == RootAuxDynIO::AUX_POSTFIX )
            key.erase(key.size()-4);
         return key;
      }
      return "";
   }


   std::string
   auxBranchName(const std::string& attr_name, const std::string& baseBranchName)
   {
      std::string branch_name = baseBranchName;
      if( !branch_name.empty() and branch_name.back() == '.' )  branch_name.pop_back();
      branch_name += RootAuxDynIO::AUXDYN_POSTFIX + attr_name;
      return branch_name;
   }

   std::string
   auxFieldName(const std::string& attr_name, const std::string& baseBranchName)
   {
      std::string field_name = baseBranchName;
      if( field_name.back() == '.' )  field_name.pop_back();
      field_name += ":" + attr_name;    // MN TODO <- find a good delimiter
      return field_name;
   }

   bool
   isAuxDynBranch(TBranch *branch)
   {
      const std::string bname = branch->GetName();
      TClass *tc = 0;
      EDataType type;
      if( branch->GetExpectedType(tc, type) ) {
         // error - not expecting this to happen ever, but better report
         errorcheck::ReportMessage msg (MSG::WARNING, ERRORCHECK_ARGS, "RootAuxDynIO::isAuxDynBranch");
         msg << "GetExpectedType() failed for branch: " << bname;
         return false;
      }
      if( tc and ( endsWithAuxPostfix(bname) or (tc->GetAttributeMap() && tc->GetAttributeMap()->HasKey("IAuxStore")) ) ) {
         return tc->GetBaseClass("SG::IAuxStoreHolder") != nullptr;
      }
      return false;
   }


   std::unique_ptr<RootAuxDynIO::IRootAuxDynReader>
   getBranchAuxDynReader(TTree* tree, TBranch* branch) {
      return std::make_unique<TBranchAuxDynReader>(tree, branch);
   }

   /// generate TBranchAuxDynWriter
   /// tree -> destination tree
   /// do_branch_fill -> flag to Fill each TBranch immediately
   std::unique_ptr<RootAuxDynIO::IRootAuxDynWriter>
   getBranchAuxDynWriter(TTree* tree, int offsettab_len,  bool do_branch_fill) {
      return std::make_unique<TBranchAuxDynWriter>(tree, offsettab_len, do_branch_fill);
   }

}
