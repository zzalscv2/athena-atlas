/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "ISF_FastCaloSimEvent/TFCSParametrizationBase.h"
#include "TClass.h"

//=============================================
//======= TFCSParametrizationBase =========
//=============================================

std::set< int > TFCSParametrizationBase::s_no_pdgid;
std::vector< TFCSParametrizationBase* > TFCSParametrizationBase::s_cleanup_list;

#ifndef __FastCaloSimStandAlone__
//Initialize only in constructor to make sure the needed services are ready
Athena::MsgStreamMember* TFCSParametrizationBase::s_msg(nullptr); 
#endif

#if defined(__FastCaloSimStandAlone__)
TFCSParametrizationBase::TFCSParametrizationBase(const char* name, const char* title)
  : TNamed(name, title),
    m_level(MSG::INFO),
    m_msg(&std::cout)
{
}
#else
TFCSParametrizationBase::TFCSParametrizationBase(const char* name, const char* title):TNamed(name,title)
{
  if(s_msg==nullptr) s_msg=new Athena::MsgStreamMember("FastCaloSimParametrization");
}
#endif

void TFCSParametrizationBase::set_geometry(ICaloGeometry* geo)
{
  for(unsigned int i=0;i<size();++i) (*this)[i]->set_geometry(geo);
}

///Result should be returned in simulstate.
///Simulate all energies in calo layers for energy parametrizations.
///Simulate cells for shape simulation.
FCSReturnCode TFCSParametrizationBase::simulate(TFCSSimulationState& /*simulstate*/,const TFCSTruthState* /*truth*/, const TFCSExtrapolationState* /*extrapol*/) const
{
  ATH_MSG_ERROR("now in TFCSParametrizationBase::simulate(). This should normally not happen");
  //Force one retry to issue a printout from the chain causing the call to this method
  return (FCSReturnCode)(FCSRetry+1);
}

bool TFCSParametrizationBase::compare(const TFCSParametrizationBase& ref) const
{
  if(this==&ref) {
    ATH_MSG_DEBUG("compare(): identical instances "<< this <<" == " << &ref);
    return true;
  }
  return false;
}

///If called with argument "short", only a one line summary will be printed
void TFCSParametrizationBase::Print(Option_t *option) const
{
  TString opt(option);
  bool shortprint=opt.Index("short")>=0;
  bool longprint=msgLvl(MSG::DEBUG) || (msgLvl(MSG::INFO) && !shortprint);
  TString optprint=opt;optprint.ReplaceAll("short","");
  
  if(longprint) {
    ATH_MSG_INFO(optprint<<GetTitle()<<" ("<<IsA()->GetName()<<"*)"<<this);
    ATH_MSG(INFO) << optprint <<"  PDGID: ";
    if(is_match_all_pdgid()) {
      msg() << "all";
    } else {
      for (std::set<int>::iterator it=pdgid().begin(); it!=pdgid().end(); ++it) {
        if(it!=pdgid().begin()) msg() << ", ";
        msg() << *it;
      }  
    }
    if(is_match_all_Ekin()) {
      msg()<<" ; Ekin=all";
    } else {
      msg()<<" ; Ekin="<<Ekin_nominal()<<" ["<<Ekin_min()<<" , "<<Ekin_max()<<") MeV";
    }  
    if(is_match_all_eta()) {
      msg()<<" ; eta=all";
    } else {
      msg()<<" ; eta="<<eta_nominal()<<" ["<<eta_min()<<" , "<<eta_max()<<")";
    }  
    msg() << endmsg;
  } else {
    ATH_MSG_INFO(optprint<<GetTitle());
  }
}

void TFCSParametrizationBase::DoCleanup()
{
  //Do cleanup only at the end of read/write operations
  for(auto ptr:s_cleanup_list) if(ptr) {
    delete ptr;
  }  
  s_cleanup_list.resize(0);
}

void TFCSParametrizationBase::FindDuplicates(FindDuplicateClasses_t& dupclasses)
{
  for(unsigned int i=0;i<size();++i) if((*this)[i]) {
    TFCSParametrizationBase* param=(*this)[i];
    FindDuplicates_t& dup=dupclasses[param->GetName()];
    //If param is already in the duplication list, skip over
    auto checkexist=dup.find(param);
    if(checkexist!=dup.end()) {
      ATH_MSG_DEBUG("Found duplicate pointer for: "<<param<<"="<<param->GetName());
      if(checkexist->second.replace) {
        TFCSParametrizationBase* refparam=checkexist->second.replace;
        ATH_MSG_DEBUG("Found duplicate pointer: "<<refparam<<"="<<refparam->GetName()<<", duplicate is "<<param<<"="<<param->GetName()<<" index "<<i<<" of "<<this);
        dup[refparam].mother.push_back(this);
        dup[refparam].index.push_back(i);
      }
      continue;
    }  
    //Add param to duplication list
    dup[param]=Duplicate_t();
    for(auto& ref : dup) {
      TFCSParametrizationBase* refparam=ref.first;
      //skip itself, as this just added above
      if(param==refparam) continue;
      //skip nullptr reference
      if(refparam==nullptr) continue;
      //skip reference that is itself going to get replaced
      if(ref.second.replace) continue;
      //Check for objects with identical content
      if(*param==*refparam) {
        ATH_MSG_DEBUG("Found duplicate: "<<refparam<<"="<<refparam->GetName()<<", duplicate is "<<param<<"="<<param->GetName()<<" index "<<i<<" of "<<this);
        //param->Print("  1 ");
        //ref.first->Print("  2 ");
        dup[param].replace=refparam;
        dup[refparam].mother.push_back(this);
        dup[refparam].index.push_back(i);
        break;
      }
      /*
      if(param->IsA()==ref.first->IsA()) {
        std::string name(param->GetName());
        if(name==ref.first->GetName()) {
          ATH_MSG_INFO("potential same objects "<<param<<"="<<param->GetName()<<" and "<<ref.first<<"="<<ref.first->GetName());
          param->setLevel(MSG::DEBUG);
          param->operator==(*(ref.first));
          param->setLevel(MSG::INFO);
          param->Print("  1 ");
          ref.first->Print("  2 ");
        }
      }
      */
    }
    //Continue for child objects in param
    param->FindDuplicates(dupclasses);
  }
}

void TFCSParametrizationBase::RemoveDuplicates()
{
  FindDuplicateClasses_t dupclasses;
  FindDuplicates(dupclasses);
  
  std::set< TFCSParametrizationBase* > dellist;
  for(auto& dupiter : dupclasses) {
    FindDuplicates_t& dup=dupiter.second;
    for(auto onedup : dup) {
      if(onedup.second.mother.size()==0) continue;
      TFCSParametrizationBase* ref=onedup.first;
      ATH_MSG_DEBUG("Main object "<<ref<<"="<<ref->GetName());
      //ref->Print("  + ");
      for(unsigned int i=0;i<onedup.second.mother.size();++i) {
        int index=onedup.second.index[i];
        TFCSParametrizationBase* mother=onedup.second.mother[i];
        TFCSParametrizationBase* delparam=mother->operator[](index);
        unsigned int delcount=dup[delparam].mother.size();
        if(delcount==0) {
          ATH_MSG_DEBUG("  - Delete object "<<delparam<<"="<<delparam->GetName()<<" index "<<index<<" of "<<mother<<", has "<<delcount<<" other replacements attached. Deleting");
          mother->set_daughter(index,ref);
          dellist.insert(delparam);
        } else {
          ATH_MSG_WARNING("  - Delete object "<<delparam<<"="<<delparam->GetName()<<" index "<<index<<" of "<<mother<<", has "<<delcount<<" other replacements attached. Skipping");
        }  
      }
    }
  }  

  ATH_MSG_INFO("RERUNNING DUPLICATE FINDING");
  FindDuplicateClasses_t dupclasses2;
  FindDuplicates(dupclasses2);

  std::map<std::string,int> ndel;
  for(auto delparam : dellist) {
    FindDuplicates_t& dup2=dupclasses2[delparam->GetName()];
    bool present=dup2.find(delparam) != dup2.end();
    if(present) {
      ATH_MSG_WARNING("- Delete object "<<delparam<<"="<<delparam->GetName()<<" still referenced somewhere!");
    } else {  
      ATH_MSG_DEBUG("- Delete object "<<delparam<<"="<<delparam->GetName());
      ++ndel[delparam->ClassName()];
      delete delparam;
    }  
  }
  for(auto& del : ndel) ATH_MSG_INFO("Deleted "<<del.second<<" duplicate objects of class "<<del.first);
}


