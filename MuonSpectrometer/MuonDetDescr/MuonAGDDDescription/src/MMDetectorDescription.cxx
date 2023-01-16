/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonAGDDDescription/MMDetectorDescription.h"
#include "AGDDModel/AGDDParameterStore.h"
#include "AGDDKernel/AGDDDetectorStore.h"

#include <sstream>


extern int myatoi(std::string_view str);

MMDetectorDescription::MMDetectorDescription(const std::string& s,
                                             AGDDDetectorStore& ds):
	AGDDDetector(s,"Micromegas"),
        m_ds (ds)
{
}

void MMDetectorDescription::Register()
{
	m_ds.RegisterDetector(this);
}


void MMDetectorDescription::SetDetectorAddress(AGDDDetectorPositioner* p)
{
		//std::cout<<"This is AGDDMicromegas::SetDetectorAddress "<<GetName()<<" "<<
		//m_sType;
		p->ID.detectorType="Micromegas";
		p->theDetector=this;
		std::stringstream stringone;
		char side='A';
		if (p->ID.sideIndex<0) side='C';
		int ctype=0;
		std::string_view subt = subType();
		int ml=myatoi(subt.substr(3,1));
		if (subt[2]=='L') ctype=1;
		else if (subt[2]=='S') ctype=3;
		int etaIndex=myatoi(subt.substr(1,1));
		stringone<<"sMD"<<ctype<<'-'<<etaIndex<<'-'<<ml<<"-phi"<<p->ID.phiIndex+1<<side<<std::endl;
		//std::cout<<" stringone "<<stringone.str()<<std::endl;
		p->ID.detectorAddress=stringone.str();
}

MuonGM::MM_Technology* MMDetectorDescription::GetTechnology()
{
   MuonGM::MM_Technology* t =
     dynamic_cast<MuonGM::MM_Technology*>(m_ds.GetTechnology(GetName()));
   return t;
}
