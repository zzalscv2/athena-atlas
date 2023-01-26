/*
   Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
 */

#include <fstream>
#include <sstream>
#include <string>

#include "TopDataPreparation/SampleXsection.h"
#include <iostream>
#include <stdlib.h>

using namespace std;

SampleXsection& SampleXsection::operator = (const SampleXsection& xs) {
  if (this != &xs) {
    m_Xsects = xs.m_Xsects;
  }
  return *this;
}

bool SampleXsection::readFromFile(const char* fName) {
  ifstream in(fName);

  if (!in) return false;

  for (; !in.eof(); ) {
    string line;
    if (!getline(in, line)) break;
    if (!line.empty() && line[0] != '#') {
      istringstream istr(line);
      int dsid = -1;
      double xSect, kFact;
      string s_shower, s_showerJES;
      showering shower;

      istr >> dsid >> xSect >> kFact >> s_shower >> s_showerJES;
      if (!s_shower.empty() && s_shower[0] != '#') {
        shower = stringToShower(s_shower);
        if (shower == showering::unknown) {
          cerr <<
          "ERROR!! TopDataPreparation::SampleXsection::readFromFile: unknown showering (which is needed for btagging SF!!!) : "
               << s_shower << " for DSID= " << dsid << endl;
        }
	
      }// !s_showering.empty()
      else {
        shower = unknown;
      }
      
      if (!s_showerJES.empty() && s_showerJES[0] != '#') {
	if(!checkShower_JES(s_showerJES)){
	  cerr <<
	    "ERROR!! TopDataPreparation::SampleXsection::readFromFile: unknown showering (which is needed for JES MC2MC scale factors!!!) : "
               << s_showerJES << " for DSID= " << dsid << endl;
	}
      }
      else {
	s_showerJES = "Undefined";
      }

      setSample(dsid, xSect, kFact, shower, s_showerJES);
    }
  }

  in.close();
  return true;
}

void SampleXsection::setSample(const int dsid, const double xSect, const double kFact, const showering shower,
			       std::string showerJES) {
  if (dsid < 0) return;

  map<int, pair<double, double> >::const_iterator it = m_Xsects.find(dsid);
  if (it != m_Xsects.end()) {
    cerr << "ERROR!! SampleXsection::setSample: duplicate entry ! DSID= " << dsid << endl;
    //cerr << "ERROR!! SampleXsection::setSample:                   xSect=" << xSect << "\told xSect=" <<
    // it->second.first <<endl;
  }
  m_Xsects[dsid] = pair<double, double>(xSect, kFact);
  m_Showering[dsid] = shower;
  m_ShoweringJES[dsid] = showerJES;
}

double SampleXsection::getRawXsection(const int dsid) const {
  map<int, pair<double, double> >::const_iterator it = m_Xsects.find(dsid);
  if (it != m_Xsects.end()) return it->second.first;

  return -1;
}

double SampleXsection::getKfactor(const int dsid) const {
  map<int, pair<double, double> >::const_iterator it = m_Xsects.find(dsid);
  if (it != m_Xsects.end()) return it->second.second;

  return -1;
}

double SampleXsection::getXsection(const int dsid) const {
  map<int, pair<double, double> >::const_iterator it = m_Xsects.find(dsid);
  if (it != m_Xsects.end()) return (it->second.first) * (it->second.second);

  return -1;
}

SampleXsection::showering SampleXsection::getShowering(const int dsid) const {
  map<int, showering >::const_iterator it = m_Showering.find(dsid);
  if (it != m_Showering.end()) {

    showering shower = it->second;
    // apply the translation of set
    shower = applyTranslation(shower); 

    return shower;
  }

  return unknown;
}

std::string SampleXsection::getShowering_JES(const int dsid) const {
 
  std::string shower = "Undefined";

  map<int, std::string >::const_iterator it = m_ShoweringJES.find(dsid);
  if (it != m_ShoweringJES.end()) {

    shower = it->second;
    
  }

  return shower;
}


std::string SampleXsection::getShoweringString(const int dsid) const
{
  return showerToString(getShowering(dsid));
}

int SampleXsection::getShoweringIndex(const int dsid) const {
  // A.Knue: labelling taken from here:
  // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BTagCalib2015#Pre_Recommendations_for_MC15_Bas

  showering shower = getShowering(dsid);

  if (shower == showering::pythia) return 0;
  else if (shower == showering::herwigpp || shower == showering::herwig) // use the herwig++ maps for the few fortran
                                                                         // herwig samples we may have
    return 1;
  else if (shower == showering::pythia8) return 2;
  else if (shower == showering::sherpa) return 3;
  else if (shower == showering::sherpa21) return 4;
  else if (shower == showering::amcatnlopythia8) return 5;
  else if (shower == showering::herwigpp713) return 6;
  else if (shower == showering::sherpa228) return 7;
  else if (shower == showering::sherpa2210) return 8;
  else if (shower == showering::herwigpp721) return 9;
  else {
    std::cout <<
    "==========================================================================================================================================="
              << std::endl;
    std::cout << "ERROR:TopDataPreparation: The DSID " << dsid <<
    " does not have a showering algorithm defined!!! This is needed for MC/MC SF!!! ---> EXIT." << std::endl;
    std::cout << "\t" << std::endl;
    exit(1);
  }

  return 0;
}

SampleXsection::showering SampleXsection::applyTranslation(const SampleXsection::showering shower) const {
  const std::string& stringName = showerToString(shower);

  // check the map
  auto itr = m_translator.find(stringName);
  if (itr == m_translator.end()) {
    return shower;
  } else {
    // need to apply the translation
    return stringToShower(itr->second);
  }
}

std::string SampleXsection::showerToString(const SampleXsection::showering shower) const {
  if (shower == showering::pythia) return "pythia";
  else if (shower == showering::herwig) return "herwig";
  else if (shower == showering::herwigpp) return "herwig";
  else if (shower == showering::sherpa) return "sherpa";
  else if (shower == showering::sherpa21) return "sherpa21";
  else if (shower == showering::pythia8) return "pythia8";
  else if (shower == showering::amcatnlopythia8) return "amcatnlopythia8";
  else if (shower == showering::herwigpp713) return "herwigpp713";
  else if (shower == showering::sherpa228) return "sherpa228";
  else if (shower == showering::sherpa2210) return "sherpa2210";
  else if (shower == showering::herwigpp721) return "herwigpp721";

  return "";
}

SampleXsection::showering SampleXsection::stringToShower(const std::string& name) const {
  if (name == "pythia") return showering::pythia;
  else if (name == "herwig") return showering::herwig;
  else if (name == "sherpa") return showering::sherpa;
  else if (name == "sherpa21") return showering::sherpa21;
  else if (name == "pythia8") return showering::pythia8;
  else if (name == "herwigpp") return showering::herwigpp;
  else if (name == "amcatnlopythia8") return showering::amcatnlopythia8;
  else if (name == "herwigpp713") return showering::herwigpp713;
  else if (name == "sherpa228") return showering::sherpa228;
  else if (name == "sherpa2210") return showering::sherpa2210;
  else if (name == "herwigpp721") return showering::herwigpp721;

  return showering::unknown;
}

bool SampleXsection::checkShower_JES(const std::string& name) const{
  
  if (name == "pythia") return true;
  else if (name == "H713") return true;
  else if (name == "H716") return true;
  else if (name == "H716dip") return true;
  else if (name == "H721") return true;
  else if (name == "Sh221") return true;
  else if (name == "Sh222") return true;
  else if (name == "Sh225") return true;
  else if (name == "Sh225Lund") return true;
  else if (name == "Sh2211") return true;
  else if (name == "Sh2211tun") return true;
  else if (name == "None") return true;
  else return false;
  
}
