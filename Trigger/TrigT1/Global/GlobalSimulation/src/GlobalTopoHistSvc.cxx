/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "GlobalTopoHistSvc.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"

namespace GlobalSim {
  

  void GlobalTopoHistSvc::registerHist(TH1 * h) {
    if(h != nullptr) {
      const std::string key = h->GetName();
      if( m_hists1D.find(key)  == end(m_hists1D) ) {
	m_hists1D[key] = h;
      } else {
	if (h != m_hists1D[key] ) {
	  delete h;
	}
      }
    }
    
  }
  
  void GlobalTopoHistSvc::registerHist(TH2 * h) {
    if(h) {
      const std::string key = h->GetName();
      if( m_hists2D.find(key)  == end(m_hists2D) ) {
	m_hists2D[key] = h;
      } else {
	delete h;
      }
    }
      
  }
  
  TH1 * GlobalTopoHistSvc::findHist(const std::string & histName) {
    auto colPos = histName.find_first_of('/');
    std::string realhistName = histName.substr(colPos+1);
    auto h = m_hists1D.find(realhistName);
    if( h == end(m_hists1D) ) {
      return nullptr;
    } else {
      return h->second;
    }
  }
  
  void GlobalTopoHistSvc::fillHist1D(const std::string & histName,double x) {
    auto h = m_hists1D.find(histName);
    if( h != end(m_hists1D) ) {
      h->second->Fill(x);
    }
  }
  
  void GlobalTopoHistSvc::fillHist2D(const std::string & histName,double x,double y) {
    auto h = m_hists2D.find(histName);
    if( h != end(m_hists2D) ) {
      h->second->Fill(x, y);
    }
  }
  
  void GlobalTopoHistSvc::setBaseDir(const std::string & baseDir) {
    m_baseDir = baseDir;
  }
  
  void GlobalTopoHistSvc::save() {
    
    std::string filename = "L1Topo.root";
    std::string basepath = "";
    
    std::string opt = "RECREATE";
    
    auto colPos = m_baseDir.find_last_of(':');
    if( colPos != std::string::npos ) {
      filename = m_baseDir.substr(0, colPos);
      basepath = m_baseDir.substr(colPos+1);
    } else {
      basepath = m_baseDir;
    }
     
    
    TFile * f = TFile::Open(filename.c_str(),opt.c_str());
    for( auto h : m_hists1D ) {
      
      std::string fullName(h.second->GetName());
      std::string path(basepath);
      
       auto slashPos = fullName.find_last_of('/');
       if(slashPos != std::string::npos) {
	 if(!path.empty())
	   path += "/";
	 path += fullName.substr(0,slashPos);
	 // set the name
	 h.second->SetName( fullName.substr(slashPos+1).c_str() );
       }
       
       const char* dir = path.c_str();
       if( ! f->GetDirectory(dir)) {
	 f->mkdir(dir);
       }
         f->cd(dir);
         h.second->Write();
    }
    for( auto h : m_hists2D ) {
      
      std::string fullName(h.second->GetName());
      std::string path(basepath);
      
      auto slashPos = fullName.find_last_of('/');
      if(slashPos != std::string::npos) {
	if(!path.empty())
	  path += "/";
	path += fullName.substr(0,slashPos);
	// set the name
	 h.second->SetName( fullName.substr(slashPos+1).c_str() );
      }
      
      const char* dir = path.c_str();
      if( ! f->GetDirectory(dir)) {
	f->mkdir(dir);
       }
      f->cd(dir);
      h.second->Write();
    }
    f->Write();
     f->Close();
  }
    
}




