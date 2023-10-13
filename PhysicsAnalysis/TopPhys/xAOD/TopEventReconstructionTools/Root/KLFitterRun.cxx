/*
   Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
 */

#include "TopEventReconstructionTools/KLFitterRun.h"

#include "TopEvent/Event.h"
#include "TopEvent/EventTools.h"
#include "TopConfiguration/TopConfig.h"
#include "TopConfiguration/Tokenize.h"

#include "TopEventReconstructionTools/MsgCategory.h"
using namespace TopEventReconstructionTools;

#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>

namespace top {
  KLFitterRun::KLFitterRun(const std::string& kSelectionName, const std::string& kParameters,
                           std::shared_ptr<top::TopConfig> config) :
    m_name(""),
    m_useJetAutoSet(false),
    m_useJetAutoSetPCBT(false),
    m_Njcut(-1),
    m_nb(-1),
    m_delta(-1),
    m_Nbmin(-1),
    m_N5max(-1),
    m_myFitter(nullptr) {
    std::string kLeptonType = "";
    std::string kCustomParameters = "";
    if (kParameters.find(" ") != std::string::npos) {
      kLeptonType = kParameters.substr(0, kParameters.find(" "));
      kCustomParameters = kParameters.substr(kParameters.find(" ") + 1);
      std::pair<bool,bool> res = top::KLFitterRun::hasAutoSetOption(kCustomParameters);
      m_useJetAutoSet = res.first;
      m_useJetAutoSetPCBT = res.second;
    } else kLeptonType = kParameters;
    m_name = "RECO::KLFitterRun_" + kLeptonType;
    if(m_useJetAutoSet) m_name=m_name+Form("_Nb%dDelta%dNjcut%d",m_nb,m_delta,m_Njcut);
    m_myFitter = std::unique_ptr<top::KLFitterTool> (new top::KLFitterTool(m_name));
    top::check(m_myFitter->setProperty("config", config), "Failed to setProperty of KLFitterTool");
    top::check(m_myFitter->setProperty("LeptonType", kLeptonType), "Failed to setProperty of KLFitterTool");
    top::check(m_myFitter->setProperty("CustomParameters", kCustomParameters), "Failed to setProperty of KLFitterTool");
    top::check(m_myFitter->setProperty("SelectionName", kSelectionName), "Failed to setProperty of KLFitterTool");
    if(m_useJetAutoSet) {
      if(m_useJetAutoSetPCBT) {
	top::check( m_myFitter->setProperty("Nbmin",m_Nbmin), "Error when setting Nbmin" );
	top::check( m_myFitter->setProperty("N5max",m_N5max), "Error when setting N5max" );
      } else {
	top::check( m_myFitter->setProperty("Nb",m_nb), "Error when setting Nb" );
	top::check( m_myFitter->setProperty("Delta",m_delta), "Error when setting Delta" );
      }
      top::check( m_myFitter->setProperty("Njcut",m_Njcut), "Error when setting Njcut" );
    }

    top::check(m_myFitter->initialize(), "Failed to initialize KLFitterTool");
  }

  bool KLFitterRun::apply(const top::Event& event) const {

    top::check(m_myFitter->execute(event), "Failed to run KLFitterTool");
    return true;
  }


 
  std::pair<bool,bool> KLFitterRun::hasAutoSetOption(const std::string &custom_parameters) {
    const std::string autoSetOpt="KLFitterJetSelectionMode:kAutoSet";
    const std::string autoSetPCBTOpt="KLFitterJetSelectionMode:kAutoSetPCBT";
    const TString nbOpt="nb";
    const TString deltaOpt="delta";
    const TString nbminOpt="Nbmin";
    const TString n5maxOpt="N5max";
    const TString njcutOpt="Njcut";

    int isPCBT=-1;
    if(custom_parameters.find(autoSetOpt)==std::string::npos) return std::make_pair(false,false);
    std::vector<std::string> custom_tokens;
    tokenize(custom_parameters, custom_tokens, " ");
    for(auto token : custom_tokens) {
      if(token.find(autoSetOpt)==std::string::npos && token.find(autoSetPCBTOpt)==std::string::npos) continue;
      token.find(autoSetPCBTOpt)!=std::string::npos ? isPCBT=1 : isPCBT=0;
      std::vector<TString> custom_subtokens;
      tokenize(token, custom_subtokens, ":");
      for(TString subtok : custom_subtokens) {
	if(subtok.BeginsWith(njcutOpt)) {
	  subtok.ReplaceAll(njcutOpt,"");
	  if(subtok.IsFloat()) m_Njcut=subtok.Atoi();
	}
	if(isPCBT==1) {
	  if(subtok.BeginsWith(nbminOpt)) {
	    subtok.ReplaceAll(nbminOpt,"");
	    if(subtok.IsFloat()) m_Nbmin=subtok.Atoi();
	  }
	  if(subtok.BeginsWith(n5maxOpt)) {
	    subtok.ReplaceAll(n5maxOpt,"");
	    if(subtok.IsFloat()) m_N5max=subtok.Atoi();
	  }
	} else {
	  if(subtok.BeginsWith(nbOpt)) {
	    subtok.ReplaceAll(nbOpt,"");
	    if(subtok.IsFloat()) m_nb=subtok.Atoi();
	  }
	  if(subtok.BeginsWith(deltaOpt)) {
	    subtok.ReplaceAll(deltaOpt,"");
	    if(subtok.IsFloat()) m_delta=subtok.Atoi();
	  }
	}
      }
      break;
    }

    std::ostringstream errorMessage;
    errorMessage << "Bad AutoSet option. It should be either: \n\t'" << autoSetOpt << "." << nbOpt << "<X>." << deltaOpt << "<Y>." << njcutOpt << "<Z>' where <X>, <Y> and <Z> are positive integers. Order of nb, delta and Njcut is irrelevant. (NOTE: THIS OPTION USES B-TAGGING WEIGHT FOR JET ORDERING, WHICH IS **NOT SUPPORTED BY THE FLAVOUR TAGGING GROUP**. PLEASE CONSIDER USING THE VERSION BELOW, BASED ON PSEUDO-CONTINUOUS B-TAGGING). \nOr: \n\t'" << autoSetPCBTOpt << "." << nbminOpt << "<X>." << n5maxOpt << "<Y>." << njcutOpt << "<Z>' where <X>, <Y> and <Z> are integers >=0. Order of Nbin, N5max and Njcut is irrelevant";

    bool allok=(m_Njcut>=6);
    bool usePCBT=false;
    if(isPCBT==0) {
      allok=(allok && m_delta>=0 && m_nb>=2);
      top::check(m_delta+m_nb<=m_Njcut,"Njcut must be >= (nb+delta)");
    }
    else if(isPCBT==1){
      allok=(allok && m_Nbmin>=2);
      top::check(allok,errorMessage.str());
      top::check(m_Nbmin<=m_Njcut,"Njcut must be >= Nbmin");
      usePCBT=true;
    }

    if(allok && !usePCBT) {
     
      ATH_MSG_WARNING("KLFitterRun initialization: THE " << autoSetOpt << " OPTION USES B-TAGGING WEIGHT FOR JET ORDERING, WHICH IS **NOT SUPPORTED BY THE FLAVOUR TAGGING GROUP**. PLEASE CONSIDER USING THE VERSION BELOW, BASED ON PSEUDO-CONTINUOUS B-TAGGING). \n\t'" << autoSetPCBTOpt << "." << nbminOpt << "<X>." << n5maxOpt << "<Y>." << njcutOpt << "<Z>' where <X>, <Y> and <Z> are integers >=0. Order of Nbin, N5max and Njcut is irrelevant");
      
    }
    return std::make_pair(allok,usePCBT);
  }



  std::string KLFitterRun::name() const {
    return m_name;
  }
}
