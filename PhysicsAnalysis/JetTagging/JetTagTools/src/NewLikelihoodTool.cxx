/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "JetTagTools/NewLikelihoodTool.h"

#include "JetTagTools/HistoHelperRoot.h"

#include <cmath>
#include <string>
#include <utility>
#include <vector>

#include "TH1.h"
#include "TH2.h"
#include "TH3.h"

namespace Analysis {

  NewLikelihoodTool::NewLikelihoodTool(const std::string& t, const std::string& n, const IInterface* p) :
    AthAlgTool(t,n,p),
    m_taggerName("undefined"),
    m_hypotheses(std::vector<std::string>()),
    m_normalizedProb(true),
    m_interpolate(false),
    m_smoothNTimes(1),
    m_vetoSmoothingOf(std::vector<std::string>()),
    m_histograms(std::vector<std::string>()) {
   
    declareInterface<NewLikelihoodTool>(this);
    declareProperty("taggerName", m_taggerName);
    declareProperty("hypotheses", m_hypotheses);
    declareProperty("normalizedProb", m_normalizedProb);
    declareProperty("interpolate",m_interpolate);
    declareProperty("smoothNTimes",m_smoothNTimes);
    declareProperty("vetoSmoothingOf",  m_vetoSmoothingOf);
    m_hypotheses.push_back("B");
    m_hypotheses.push_back("U");
    m_hypotheses.push_back("C");
    m_vetoSmoothingOf.push_back("/N2T");
    m_vetoSmoothingOf.push_back("/N2TEffSV2");
    m_vetoSmoothingOf.push_back("/Sip3D");
  }

  StatusCode NewLikelihoodTool::initialize() {
    ATH_CHECK(m_readKey.initialize());
    return StatusCode::SUCCESS;
  }

  void NewLikelihoodTool::defineHypotheses(const std::vector<std::string>& hyp) {
    m_hypotheses = hyp;
  }

  void NewLikelihoodTool::printStatus() const {
    msg(MSG::INFO) << "#BTAG# - hypotheses : ";
    for(const auto& hypo : m_hypotheses) msg(MSG::INFO) << hypo << ", ";
    msg(MSG::INFO) << endmsg;
    msg(MSG::INFO) << "#BTAG# - histograms : " << endmsg;
    for(const auto& histo : m_histograms) msg(MSG::INFO) << histo << endmsg;
  }

  std::vector<std::string> NewLikelihoodTool::gradeList(const std::string& histoName) const {
    ATH_MSG_VERBOSE("#BTAG# gradeList() called for " << histoName);
    const std::string delimSlash("/");
    // first count slashes:
    unsigned int nSlash = 0;
    std::string::size_type slashPos;
    for( unsigned int i = 0; 
         (slashPos = histoName.find(delimSlash, i)) != std::string::npos; 
         i = slashPos + 1) nSlash++;
    // extract string before the 2nd / sign: this is for the grades, the rest is the histogram name
    slashPos = histoName.find_first_of(delimSlash);
    std::string newName = histoName.substr(slashPos+1);
    slashPos = newName.find_first_of(delimSlash);
    std::string grades = newName.substr(0,slashPos);
    std::string hhname = newName.substr(slashPos+1);
    if(nSlash<2) grades = "";
    ATH_MSG_VERBOSE("#BTAG# -> grades: " << grades);
    ATH_MSG_VERBOSE("#BTAG# -> hhname: " << hhname);
    // now decode the grades:
    std::vector<std::string> gradeList;
    if(grades!="") {
      const std::string delimUds("_");
      std::string::size_type sPos, sEnd, sLen;
      sPos = grades.find_first_not_of(delimUds);
      while ( sPos != std::string::npos ) {
        sEnd = grades.find_first_of(delimUds, sPos);
        if(sEnd==std::string::npos) sEnd = grades.length();
        sLen = sEnd - sPos;
        std::string word = grades.substr(sPos,sLen);
        ATH_MSG_DEBUG("#BTAG# --> grade = " << word);
        gradeList.push_back(word);
        sPos = grades.find_first_not_of(delimUds, sEnd);
      }
    }
    // add the histogram name at the end of the list:
    gradeList.push_back(hhname);
    return gradeList;
  }

  TH1* NewLikelihoodTool::prepareHistogram(const std::string& hypo, const std::string& hname) const {
    TH1* histoSum = nullptr;

    SG::ReadCondHandle<JetTagCalibCondData> readCdo(m_readKey);
    std::string channelName = readCdo->channelName(hname);
    std::string histoName = readCdo->histoName(hname);
    std::string actualName = hypo + "/" + histoName;
    std::string longName = channelName + "#" + actualName;
    ATH_MSG_VERBOSE("#BTAG# preparing histogram " << longName);
    ATH_MSG_VERBOSE("#BTAG# -> channel is " << channelName);
    ATH_MSG_VERBOSE("#BTAG# -> histo is " << histoName);
    // now decode the grades:
    std::vector<std::string> gradeList = this->gradeList(histoName);
    // - case with no grade:
    if(1==gradeList.size()) {
      ATH_MSG_DEBUG("#BTAG# Histo "<<actualName<<" has no grade: direct retrieval");
      histoSum = readCdo->retrieveHistogram(m_taggerName, 
                                                                       channelName, 
                                                                       actualName);
    }
    // - case with 1 grade:
    if(2==gradeList.size()) {
      ATH_MSG_DEBUG("#BTAG# Histo "<<actualName<<" has only one grade: direct retrieval");
      histoSum = readCdo->retrieveHistogram(m_taggerName, 
                                                                       channelName, 
                                                                       actualName);
      //Part not fully migrated
      //smoothAndNormalizeHistogram should not be called here but in condition algorithm
      ATH_MSG_VERBOSE("#BTAG# Smoothing histogram " << longName << " ...");
      this->smoothAndNormalizeHistogram(histoSum, longName);
    }
    // - for many grades, get individual histos and sum them up:
    if(gradeList.size()>2) {
      TH1* histo = nullptr;
      ATH_MSG_DEBUG("Histo " << actualName << " has " << (gradeList.size()-1) << " grades:");
      for(unsigned int i=0;i<(gradeList.size()-1);i++) {
        actualName = hypo+"/"+gradeList[i]+gradeList[gradeList.size()-1];
        ATH_MSG_VERBOSE("#BTAG#  -> retrieving histo for grade " << i << " " << gradeList[i] << ": ");
        histo = readCdo->retrieveHistogram(m_taggerName, 
                                                                          channelName, 
                                                                          actualName);
        if(histo) ATH_MSG_VERBOSE("#BTAG#     histo " << actualName 
                  << " has " << histo->GetEntries() << " entries."); 
        if(0==i) {
          histoSum = histo;
        } else {
          if(histo&&histoSum) histoSum->Add(histo,1.);
        }
      }
      //Part not fully migrated
      //smoothAndNormalizeHistogram should not be called here but in condition algorithm     
      ATH_MSG_VERBOSE("#BTAG# Smoothing histogram " << longName << " ...");
      this->smoothAndNormalizeHistogram(histoSum, longName);
    }
    return histoSum;
  }

  void NewLikelihoodTool::smoothAndNormalizeHistogram(TH1* h, const std::string& hname = "") const {
    if(h) {
      double norm = h->Integral();
      if(norm) {
        // check if smoothing of histogram is not vetoed:
        bool veto = false;
	for(const auto& v : m_vetoSmoothingOf) {
          if(hname.find(v)!=std::string::npos) {
            veto = true;
            ATH_MSG_VERBOSE("#BTAG# Smoothing of " << hname << " is vetoed !");
            break;
          }
        }
        if(1==h->GetDimension() && m_smoothNTimes) {
          if(!veto) {
            if(norm>10000)h->Smooth(m_smoothNTimes);
            else h->Smooth((int)(m_smoothNTimes*100./sqrt(norm)));
          }
        }
        if(2==h->GetDimension()) {
          int m2d=3;
          //if(hname.find("#B/")==std::string::npos) m2d=5; //VK oversmoothing!!!
          if(!veto) {
	    TH2 * dc_tmp = dynamic_cast<TH2*>(h);
	    if (dc_tmp) {
              HistoHelperRoot::smoothASH2D(dc_tmp, m2d, m2d, msgLvl(MSG::DEBUG));
            }
          }
        }
        if(3==h->GetDimension()) {
          int m3d1=3;
          int m3d3=2;
          if(!veto) {
	    TH3 * dc_tmp = dynamic_cast<TH3*>(h);
	    if (dc_tmp) {
	      int Nx=dc_tmp->GetNbinsX();
	      int Ny=dc_tmp->GetNbinsY();
	      int Nz=dc_tmp->GetNbinsZ();
	      if(Nz == 7)         //==========Old SV2
                HistoHelperRoot::smoothASH3D(dc_tmp, m3d1, m3d1, m3d3, msgLvl(MSG::DEBUG));
 	      else if(Nz == 6){   //==========New SV2Pt
                double total=dc_tmp->Integral(1,Nx,1,Ny,1,Nz,"");
	        for(int iz=1; iz<=Nz; iz++){
                  double content=dc_tmp->Integral(1,Nx,1,Ny,iz,iz,""); if(content==0.)content=Nz;
		  double dnorm=total/content/Nz;
	          for(int ix=1; ix<=Nx; ix++){
		    for(int iy=1; iy<=Ny; iy++){
		      double cbin=dc_tmp->GetBinContent(ix,iy,iz)*dnorm; cbin= cbin>0. ? cbin : 0.1; //Protection against empty bins
		      dc_tmp->SetBinContent(ix,iy,iz, cbin);
		    }
		  }
                }
                HistoHelperRoot::smoothASH3D(dc_tmp, m3d1, m3d1, m3d3, msgLvl(MSG::DEBUG));
              }
            }
          }
        }
        // normalize:
        norm = h->Integral();
        h->Scale(1./norm);
      } else {
        ATH_MSG_DEBUG("#BTAG# Histo "<<h<<" is empty!");
      }
    }
  }

  std::vector<double> NewLikelihoodTool::calculateLikelihood(const std::vector<Slice>& lhVariableValues) const
  {
    ATH_MSG_VERBOSE("#BTAG# calculate called for " << m_hypotheses.size() << " hypotheses.");
    std::vector<double> probDensityPerEventClassAllVariables;
    probDensityPerEventClassAllVariables.resize(m_hypotheses.size());
    for (unsigned int i = 0 ; i < m_hypotheses.size(); ++i) {
      probDensityPerEventClassAllVariables[i]=1.;
    }
    ATH_MSG_VERBOSE("#BTAG# -- lhVarVal size= " << lhVariableValues.size());
    // loop on Tracks in the Jet (IP) / Vertices in the Jet (SV)
    for (const auto& value : lhVariableValues) {
      ATH_MSG_VERBOSE( "#BTAG# -- element " << value.name );
      int ncompo = value.composites.size();
      ATH_MSG_VERBOSE( "#BTAG# -- element " << value.name
               << " has " << ncompo << " composites." );
      // loop on variables that make up the Tag, e.g. 
      // one 1D for IP2D, one 2D for IP3D, one 1D and one 2D for SV1, one 3D for SV2
      for (const auto& compo : value.composites) {
        double sum(0.);
        std::vector<double> tmpVector; 
        std::string histName = compo.name;
        int idim = compo.atoms.size();
        ATH_MSG_VERBOSE( "#BTAG#   -- composite histo= "
             << histName << " dim= " << idim );
	for (const auto& hypo : m_hypotheses) {
          TH1* tmpHisto = this->prepareHistogram(hypo,histName);
          if(tmpHisto) {
            if(1==idim) {
              double valuex = compo.atoms[0].value;
              int binx = (tmpHisto->GetXaxis())->FindBin(valuex);
              if(valuex >= tmpHisto->GetXaxis()->GetXmax()) binx = tmpHisto->GetXaxis()->GetNbins();
              if(valuex <= tmpHisto->GetXaxis()->GetXmin()) binx = 1;
              double tmp = tmpHisto->GetBinContent(binx);
              if( msgLvl(MSG::VERBOSE) ) msg(MSG::VERBOSE) << "#BTAG#       For hypothesis= " << hypo
                                                           << " (1D) actual value= " << valuex
                                                           << " --> bin= " << binx << " f = " << tmp;
              if(m_interpolate) {
                TH1* dc_tmp = dynamic_cast<TH1*>(tmpHisto);
  		if (dc_tmp) {
                  tmp = HistoHelperRoot::Interpol1d(valuex, dc_tmp);
                  if( msgLvl(MSG::VERBOSE) )msg(MSG::VERBOSE) << " interpolated f = " << tmp;
  		}
              }
              if(m_normalizedProb) { // pdf are already normalized
                if( msgLvl(MSG::VERBOSE) ) msg(MSG::VERBOSE) << " (normalized)" << endmsg;
              } else {
                double binw = tmpHisto->GetBinWidth(binx);
                if( msgLvl(MSG::VERBOSE) ) msg(MSG::VERBOSE) << " binw= " << binw;
                if(0==binw) {
                  msg(MSG::ERROR) << "bin width is 0" << endmsg;
                } else {
                  tmp /= binw;
                }
                if( msgLvl(MSG::VERBOSE) ) msg(MSG::VERBOSE) << " normalized f = " << tmp << endmsg;
              }
              tmpVector.push_back(tmp);
              sum += tmp;
            }
            if(2==idim) {
              double valuex = compo.atoms[0].value;
              double valuey = compo.atoms[1].value;
              int binx = (tmpHisto->GetXaxis())->FindBin(valuex);
              int biny = (tmpHisto->GetYaxis())->FindBin(valuey);
              if(valuex >= tmpHisto->GetXaxis()->GetXmax()) binx = tmpHisto->GetXaxis()->GetNbins();
              if(valuex <= tmpHisto->GetXaxis()->GetXmin()) binx = 1;
              if(valuey >= tmpHisto->GetYaxis()->GetXmax()) biny = tmpHisto->GetYaxis()->GetNbins();
              if(valuey <= tmpHisto->GetYaxis()->GetXmin()) biny = 1;
              double tmp = tmpHisto->GetBinContent(binx, biny);
              if( msgLvl(MSG::VERBOSE) ) msg(MSG::VERBOSE) << "#BTAG#       For hypothesis= " << hypo
                                                           << " (2D) actual value= " << valuex << " " << valuey
                                                           << " --> bin= " << binx << " " << biny << " f = " << tmp;
              if(m_interpolate) {
 		TH2* dc_tmp = dynamic_cast<TH2*>(tmpHisto);
		if (dc_tmp) { 
                  tmp = HistoHelperRoot::Interpol2d(valuex, valuey, dc_tmp);
                  if( msgLvl(MSG::VERBOSE) ) msg(MSG::VERBOSE) << " interpolated f = " << tmp;
		}
              }
              if(m_normalizedProb) { // pdf are already normalized
                if( msgLvl(MSG::VERBOSE) ) msg(MSG::VERBOSE) << " (normalized)" << endmsg;
              } else {
                double binw  = tmpHisto->GetXaxis()->GetBinWidth(binx) 
                             * tmpHisto->GetYaxis()->GetBinWidth(biny);
                if( msgLvl(MSG::VERBOSE) ) msg(MSG::VERBOSE) << " binw= " << binw;
                if(0==binw) {
                  msg(MSG::ERROR) << "bin width is 0" << endmsg;
                } else {
                  tmp /= binw;
                }
                if( msgLvl(MSG::VERBOSE) ) msg(MSG::VERBOSE) << " normalized f = " << tmp << endmsg;
              }
              tmpVector.push_back(tmp);
              sum += tmp;
            }
            if(3==idim) {
              double valuex = compo.atoms[0].value;
              double valuey = compo.atoms[1].value;
              double valuez = compo.atoms[2].value;
              int binx = (tmpHisto->GetXaxis())->FindBin(valuex);
              int biny = (tmpHisto->GetYaxis())->FindBin(valuey);
              int binz = (tmpHisto->GetZaxis())->FindBin(valuez);
              if(valuex >= tmpHisto->GetXaxis()->GetXmax()) binx = tmpHisto->GetXaxis()->GetNbins();
              if(valuex <= tmpHisto->GetXaxis()->GetXmin()) binx = 1;
              if(valuey >= tmpHisto->GetYaxis()->GetXmax()) biny = tmpHisto->GetYaxis()->GetNbins();
              if(valuey <= tmpHisto->GetYaxis()->GetXmin()) biny = 1;
              if(valuez >= tmpHisto->GetZaxis()->GetXmax()) binz = tmpHisto->GetZaxis()->GetNbins();
              if(valuez <= tmpHisto->GetZaxis()->GetXmin()) binz = 1;
              double tmp = tmpHisto->GetBinContent(binx, biny, binz);
              if( msgLvl(MSG::VERBOSE) ) msg(MSG::VERBOSE) << "#BTAG#       For hypothesis= " << hypo
                                                           << " (3D) actual value= " << valuex 
                                                           << " " << valuey << " " << valuez
                                                           << " --> bin= " << binx << " " << biny 
                                                           << " " << binz << " f = " << tmp;
              if(m_interpolate) {
                TH3* dc_tmp = dynamic_cast<TH3*>(tmpHisto);
                if (dc_tmp) { 
                  tmp = HistoHelperRoot::Interpol3d(valuex, valuey, valuez, dc_tmp);
                  if( msgLvl(MSG::VERBOSE) ) msg(MSG::VERBOSE) << " interpolated f = " << tmp;
		}
              }
              if(m_normalizedProb) { // pdf are already normalized
                if( msgLvl(MSG::VERBOSE) ) msg(MSG::VERBOSE) << " (normalized)" << endmsg;
              } else {
                double binw  = tmpHisto->GetXaxis()->GetBinWidth(binx) 
                             * tmpHisto->GetYaxis()->GetBinWidth(biny)
                             * tmpHisto->GetZaxis()->GetBinWidth(binz);
                if( msgLvl(MSG::VERBOSE) ) msg(MSG::VERBOSE) << " binw= " << binw;
                if(0==binw) {
                  msg(MSG::ERROR) << "bin width is 0" << endmsg;
                } else {
                  tmp /= binw;
                }
                if( msgLvl(MSG::VERBOSE) ) msg(MSG::VERBOSE) << " normalized f = " << tmp << endmsg;
              }
              tmpVector.push_back(tmp);
              sum += tmp;
            }
            if(idim>3 || idim<1 ) msg(MSG::DEBUG) << "#BTAG# " << idim 
                                                  << " is not a correct dimensionality for pdfs !" 
                                                  << endmsg;
          }
        } // endloop on hypotheses (B,U,C..) 
        unsigned int classCount(0);
	for (const auto& f : tmpVector) {
          if(sum != 0.) {
            if( msgLvl(MSG::DEBUG) ) msg(MSG::DEBUG) << "#BTAG# sum of pX = " << sum << endmsg;
            double p = f;
            if(m_normalizedProb) p /= sum;
            probDensityPerEventClassAllVariables[classCount] *= p;
          } else {
            msg(MSG::DEBUG) << "#BTAG# Empty bins for all hypothesis... "
                            << "The discriminating variables are not taken into "
                            << "account in this jet." << endmsg;
            msg(MSG::DEBUG) << "#BTAG# Please check your inputs" << endmsg;
          }
          if( msgLvl(MSG::DEBUG) ) msg(MSG::DEBUG) << "#BTAG#   probDensity= "
                                                   << probDensityPerEventClassAllVariables[classCount]
                                                   << " ic= " << classCount << endmsg;
          classCount++;
        } 
        if( msgLvl(MSG::DEBUG) ) msg(MSG::DEBUG) << "#BTAG#  Final probDensity= " 
                                                 << probDensityPerEventClassAllVariables 
                                                 << endmsg;
      }
    }
    if( msgLvl(MSG::DEBUG) ) msg(MSG::DEBUG) << "#BTAG#  Ending ..." << endmsg;
    return probDensityPerEventClassAllVariables;
  }

  double NewLikelihoodTool::getEff(const std::string& hypo, const std::string& hname, const std::string& suffix) const {
    double eff(0.);
    TH1* numH = this->prepareHistogram(hypo, hname+"Eff"+suffix);
    TH1* denH = this->prepareHistogram(hypo, hname+"Norm"+suffix);
    if(numH && denH) {
      double nnum = numH->GetEntries();
      double nden = denH->GetEntries();
      if(nden!=0) {
        eff = nnum / nden;
        if( msgLvl(MSG::VERBOSE) ) msg(MSG::VERBOSE) << "#BTAG# Efficiency for " << hypo << "#" << hname << " " 
                                                     << suffix << ": " << nnum << "/" << nden << "= " << eff << endmsg;
      } else {
        msg(MSG::DEBUG) << "#BTAG# Problem with Efficiency for " << hypo << "#" << hname << " " 
                        << suffix << ": " << nnum << "/" << nden << "= " << eff << endmsg;
      }
    } else {
      msg(MSG::DEBUG) << "#BTAG# Unknown histogram for Efficiency for " << hypo 
                      << "#" << hname << " " << suffix << endmsg;
    }
    return eff;
  }


}
