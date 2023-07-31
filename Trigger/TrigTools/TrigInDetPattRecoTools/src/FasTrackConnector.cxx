/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigInDetPattRecoTools/FasTrackConnector.h"
#include <iostream>
#include <cstring>

#include <list>
#include <set>
#include <unordered_map>

FasTrackConnection::FasTrackConnection(unsigned int s, unsigned int d) : m_src(s), m_dst(d) { 

}

FasTrackConnector::FasTrackConnector(std::ifstream& inFile) {

  m_connMap.clear();
  m_layerGroups.clear();

  int nLinks;

  inFile >> nLinks >> m_etaBin;


  for(int l=0;l<nLinks;l++) {

    unsigned int stage, lIdx, src, dst, nEntries;
    int height, width;

    inFile >> lIdx >> stage >> src >> dst >> height >> width >> nEntries;
    
    FASTRACK_CONNECTION* pC = new FASTRACK_CONNECTION(src, dst);
    
    int dummy;

    for(int i=0;i<height;i++) {
      for(int j=0;j<width;j++) inFile >> dummy;//pC->m_binTable[j+i*width];
    }

    int vol_id = src / 1000;

    if(vol_id == 13 || vol_id == 12 || vol_id == 14) {
      delete pC;
      continue;
    }

    vol_id = dst / 1000;
    
    if(vol_id == 13 || vol_id == 12 || vol_id == 14) {
      delete pC;
      continue;
    }

    std::map<int, std::vector<FASTRACK_CONNECTION*> >::iterator it = m_connMap.find(stage);
    
    if(it == m_connMap.end()) {
      std::vector<FASTRACK_CONNECTION*> v = {pC};
      m_connMap.insert(std::make_pair(stage, v));
    } else (*it).second.push_back(pC);
  }

  //re-arrange the connection stages

  std::list<const FASTRACK_CONNECTION*> lConns;

  std::map<int, std::vector<const FASTRACK_CONNECTION*> > newConnMap;
  
  for(const auto& conn : m_connMap) {
    std::copy(conn.second.begin(), conn.second.end(), std::back_inserter(lConns));
  }

  std::size_t nConnTotal = lConns.size();

  int stageCounter = 0;

  while(!lConns.empty()) {

    std::unordered_map<unsigned int, std::pair<int, int> > mCounter;//layerKey, nDst, nSrc

    for(const auto& conn : lConns) {
      auto entryIt = mCounter.find(conn->m_dst);
      if(entryIt != mCounter.end()) {
	(*entryIt).second.first++;
      }
      else {
	int nDst = 1;
	int nSrc = 0;
	mCounter.insert(std::make_pair(conn->m_dst, std::make_pair(nDst, nSrc)));
      }

      entryIt = mCounter.find(conn->m_src);
      if(entryIt != mCounter.end()) {
	(*entryIt).second.second++;
      }
      else {
	int nDst = 0;
	int nSrc = 1;
	mCounter.insert(std::make_pair(conn->m_src, std::make_pair(nDst, nSrc)));
      }
    }

    //find layers with nSrc = 0

    std::set<unsigned int> zeroLayers;

    for(const auto& layerCounts : mCounter) {
      
      if(layerCounts.second.second!=0) continue;

      zeroLayers.insert(layerCounts.first);
    }

    //remove connections which use zeroLayer as destination

    std::vector<const FASTRACK_CONNECTION*> theStage;

    std::list<const FASTRACK_CONNECTION*>::iterator cIt = lConns.begin();

    while(cIt!=lConns.end()) {
      if(zeroLayers.find((*cIt)->m_dst) != zeroLayers.end()) {//check if contains
	theStage.push_back(*cIt);
	cIt = lConns.erase(cIt);
	nConnTotal--;
	continue;
      }
      cIt++;
    }
    newConnMap.insert(std::make_pair(stageCounter, theStage));
    stageCounter++;
  }
  
  //create layer groups

  int currentStage = 0;

  //the doublet making is done using "outside-in" approach hence the reverse iterations

  for(std::map<int, std::vector<const FASTRACK_CONNECTION*> >::reverse_iterator it = newConnMap.rbegin();it!=newConnMap.rend();++it, currentStage++) {

    const std::vector<const FASTRACK_CONNECTION*> & vConn = (*it).second;
    
    //loop over links, extract all connections for the stage, group sources by L1 (dst) index
    
    std::map<unsigned int, std::vector<const FASTRACK_CONNECTION*> > l1ConnMap;

    for(const auto* conn : vConn) {

      unsigned int dst = conn->m_dst;

      std::map<unsigned int, std::vector<const FASTRACK_CONNECTION*> >::iterator l1MapIt = l1ConnMap.find(dst);
      if(l1MapIt != l1ConnMap.end()) 
	(*l1MapIt).second.push_back(conn);
      else {
	std::vector<const FASTRACK_CONNECTION*> v = {conn};
	l1ConnMap.insert(std::make_pair(dst, v));
      } 
    }

    std::vector<LayerGroup> lgv;

    lgv.reserve(l1ConnMap.size());

    for(const auto& l1Group : l1ConnMap) {
      lgv.push_back(LayerGroup(l1Group.first, l1Group.second));
    }
   
    m_layerGroups.insert(std::make_pair(currentStage, lgv));
  }

  newConnMap.clear();

}

FasTrackConnector::~FasTrackConnector() {
  m_layerGroups.clear();
  for(std::map<int, std::vector<FASTRACK_CONNECTION*> >::iterator it = m_connMap.begin();it!=m_connMap.end();++it) {
    for(std::vector<FASTRACK_CONNECTION*>::iterator cIt=(*it).second.begin();cIt!=(*it).second.end();++cIt) {
      delete (*cIt);
    }
    (*it).second.clear();
  }
  m_connMap.clear();

}
