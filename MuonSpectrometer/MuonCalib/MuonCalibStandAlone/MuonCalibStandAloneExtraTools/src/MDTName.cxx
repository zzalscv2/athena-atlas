/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCalibStandAloneExtraTools/MDTName.h"

namespace MuonCalib {

  MDTName::MDTName() : m_eta_on(-1), m_eta_off(-1), m_sector_on(-1), m_sector_off(-1), m_side('X')
  {}

  MDTName::MDTName(const MuonFixedId& the_id) {
    m_name=TString(the_id.stationNameString());
    m_eta_off=the_id.eta();
    m_eta_on=std::abs(m_eta_off);
    if(m_eta_off<0) m_side='C';
    if(m_eta_off>=0) m_side='A';
    //Here chambers with eta=0 are included in m_side A;
    m_sector_off=the_id.phi();
    if((m_name[2]=='L')||(m_name[2]=='M')||(m_name[2]=='R')) m_sector_on=((the_id.phi())*2)-1;
    else m_sector_on=((the_id.phi())*2);
    if((m_name=="BML")&&(m_sector_on==13)&&(m_eta_off>3)) m_eta_on++;
    if((m_name=="BML")&&(m_sector_on==13)&&(m_eta_off<(-3))) m_eta_on++;
    if(((m_sector_on==12)||(m_sector_on==14))&&(m_name=="BOF")) {
      m_eta_on=m_eta_on*2-1;
    }
    if(((m_sector_on==12)||(m_sector_on==14))&&(m_name=="BOG")){
      m_eta_on=m_eta_on*2;
    }
    if((m_name=="EIL")&&((m_sector_on==1)||(m_sector_on==9))&&((m_eta_on==4)||(m_eta_on==5))) {
      if(m_eta_on==4) m_eta_on=5;
      else m_eta_on=4;
    }
    if((m_name=="EEL")&&(m_sector_on==5)&&(m_eta_on==1)) {
      m_eta_on=2;
    }
  }

  MDTName::MDTName(const std::string& the_name) {
    TString s(the_name);
    MDTName_init(s);
  }

  MDTName::MDTName(const char *the_name) {
    TString s(the_name);
    MDTName_init(s);
  }

  MDTName::MDTName(const TString& the_name) {
    MDTName_init(the_name);
  }

  MDTName::MDTName(const std::string& the_name, const int sector, const int eta) {
    TString temp=the_name;
    temp+="_";
    temp+=sector;
    temp+="_";
    temp+=eta;
    MDTName_init(temp);
  }

  MDTName::MDTName(const std::string& the_name, const int eta, const std::string& side, const int sector) {
    TString temp=the_name;
    temp+=eta;
    temp+=side;
    temp+=sector;
    MDTName_init(temp);
  }

  void MDTName::MDTName_init(const TString& name) {
    TString the_name(name);
    the_name.ToUpper();
    m_name=the_name(0,3);
    if(the_name.Contains('_')) {
      the_name.Remove(0,4);
      m_sector_off=(TString(the_name(0,1))).Atoi();
      the_name.Remove(0,2);
      if(the_name.Contains('-')) {
	m_eta_off=the_name.Atoi();
	m_side='C';
	m_eta_on=std::abs(m_eta_off);
      } else {
	m_eta_off=the_name.Atoi();
	m_side='A';
	//Here chambers with eta=0 are included in m_side A;
	//if(m_eta_off==0) m_side='B';
	m_eta_on=m_eta_off;
      }

      if((m_name[2]=='L')||(m_name[2]=='M')||(m_name[2]=='R')) m_sector_on=(m_sector_off*2)-1;
      else m_sector_on=2*m_sector_off;
	
      if((m_name=="BML")&&(m_sector_on==13)&&(m_eta_off>3)) m_eta_on++;
      if((m_name=="BML")&&(m_sector_on==13)&&(m_eta_off<-3)) m_eta_on++;

      if(((m_sector_on==12)||(m_sector_on==14))&&(m_name=="BOF")) {
	m_eta_on=m_eta_on*2-1;
      }
      if(((m_sector_on==12)||(m_sector_on==14))&&(m_name=="BOG")) {
	m_eta_on=m_eta_on*2;
      }
      if((m_name=="EIL")&&((m_sector_on==1)||(m_sector_on==9))&&((m_eta_on==4)||(m_eta_on==5))) {
	if(m_eta_on==4) m_eta_on=5;
	else m_eta_on=4;
      }
      if((m_name=="EEL")&&(m_sector_on==5)&&(m_eta_on==1)) {
	m_eta_on=2;
      }

    } else {
      the_name.Remove(0,3);
      m_eta_on=(TString(the_name(0,1))).Atoi();
      the_name.Remove(0,1);
      m_side=the_name[0];
      the_name.Remove(0,1);
      m_sector_on=the_name.Atoi();

      if((m_name[2]=='L')||(m_name[2]=='M')||(m_name[2]=='R')) m_sector_off=(m_sector_on+1)/2;
      else m_sector_off=m_sector_on/2;
      if(m_side=='C') m_eta_off=-m_eta_on;
      else m_eta_off=m_eta_on;

      if((m_name=="BML")&&(m_sector_on==13)&&(m_eta_on>4)&&(m_side=='A')) m_eta_off--;
      if((m_name=="BML")&&(m_sector_on==13)&&(m_eta_on>4)&&(m_side=='C')) m_eta_off++;

      if((m_name=="EEL")&&(m_sector_on==5)&&(m_eta_on==2)) {
	if(m_side=='A') m_eta_off=1;
	if(m_side=='C') m_eta_off=-1;
      }

      if(((m_sector_on==12)||(m_sector_on==14))&&(m_name=="BOF")) {
	if(m_side=='A') {
	  m_eta_off=(m_eta_on+1)/2;
	} else {
	  m_eta_off=-(m_eta_on+1)/2;
	}
      }
      if(((m_sector_on==12)||(m_sector_on==14))&&(m_name=="BOG")) {
	if(m_side=='A') {
	  m_eta_off=(m_eta_on)/2;
	} else {
	  m_eta_off=-(m_eta_on)/2;
	}
      }
      if((m_name=="EIL")&&((m_sector_on==1)||(m_sector_on==9))&&((m_eta_on==4)||(m_eta_on==5))) {
	if(m_side=='A') {
	  if(m_eta_on==4) m_eta_off=5;
	  else m_eta_off=4;
	} else {
	  if(m_eta_on==4) m_eta_off=-5;
	  else m_eta_off=-4;
	}
      }
      
    }
  }  //end MDTName::MDTName_init

  std::string MDTName::getOnlineName() {
    TString the_name(m_name);
    the_name+=m_eta_on;
    the_name+=m_side;
    if(m_sector_on<10) the_name+="0";
    the_name+=m_sector_on;
    return (std::string)the_name;
  }

  std::string MDTName::getOfflineName() {
    TString the_name(m_name);
    the_name+='_';
    the_name+=m_sector_off;
    the_name+='_';
    the_name+=m_eta_off;
    return (std::string)the_name;
  }

  bool MDTName::isBarrel() {
    if((m_name=="BIS")&&(m_eta_on==7)) return false;
    if((m_name=="BIS")&&(m_eta_on==8)) return false;
    if((m_name=="BEE")) return false;
    if(m_name[0]=='B') return true;
    return false;
  }

  bool MDTName::isEndcap() {
    if(m_name[0]=='E') return true;
    if((m_name=="BIS")&&(m_eta_on==7)) return true;
    if((m_name=="BIS")&&(m_eta_on==8)) return true;
    if((m_name=="BEE")) return true;
    return false;
  }

  bool MDTName::isInner() {
    if(m_name[1]=='I') return true;
    return false;
  }

  bool MDTName::isMiddle() {
    if(m_name[1]=='M') return true;
    return false;
  }

  bool MDTName::isOuter() {
    if(m_name[1]=='O') return true;
    return false;
  }

  bool MDTName::isExtra() {
    if(m_name[1]=='E') return true;
    return false;
  }

  bool MDTName::isForward() {
    if (m_eta_off>=0) return true; //Here chambers with eta=0 are treated as Forward
    return false;
  }

  bool MDTName::isBackward() {
    if(m_eta_off<0) return true;
    return false;
  }

  bool MDTName::isLarge() {
    if(m_name[2]=='L') return true;
    return false;
  }

  bool MDTName::isSmall() {
    if(m_name[2]=='S') return true;
    return false;
  }

  int MDTName::getOnlineSector() {
    return m_sector_on;
  }

  int MDTName::getOfflineSector() {
    return m_sector_off;
  }

  int MDTName::getOnlineEta() {
    return m_eta_on;
  }

  int MDTName::getOfflineEta() {
    return m_eta_off;
  }

  std::string MDTName::getRegion() {
    std::string temp="Barrel";
    if(this->isEndcap()) temp="Endcap";
    return temp;
  }

  std::string MDTName::getStation() {
    std::string temp="";
    temp+=m_name[1];
    return temp;
  }

  std::string MDTName::getSize() {
    std::string temp="";
    temp+=m_name[2];
    return temp;
  }

  std::string MDTName::getSide() {
    std::string temp="";
    temp+=m_side;
    return temp;
  }

  std::string MDTName::getName() {
    return (std::string)m_name;
  }

  TString MDTName::OnlineToOfflineName(const TString& the_name) {
    MDTName temp(the_name);
    return temp.getOfflineName();
  }

  TString MDTName::OnlineToOfflineName(const char *the_name) {
    MDTName temp(the_name);
    return temp.getOfflineName();
  }

  TString MDTName::OnlineToOfflineName(const std::string& the_name) {
    MDTName temp(the_name);
    return temp.getOfflineName();
  }

  TString MDTName::OfflineToOnlineName(const TString& the_name)   {
    MDTName temp(the_name);
    return temp.getOnlineName();
  }

  TString MDTName::OfflineToOnlineName(const char* the_name) {
    MDTName temp(the_name);
    return temp.getOnlineName();
  }

  TString MDTName::OfflineToOnlineName(const std::string& the_name) {
    MDTName temp(the_name);
    return temp.getOnlineName();
  }

}  //namespace MuonCalib
