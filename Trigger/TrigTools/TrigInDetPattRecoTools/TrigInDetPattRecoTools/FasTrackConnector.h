/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETPATTRECOTOOLS_FASTRACK_CONNECTOR_H
#define TRIGINDETPATTRECOTOOLS_FASTRACK_CONNECTOR_H

#include<fstream>
#include<vector>
#include<map>

typedef struct FasTrackConnection {
public:
  FasTrackConnection(unsigned int, unsigned int);
  ~FasTrackConnection() {};

  unsigned int m_src, m_dst;
  std::vector<int> m_binTable;

} FASTRACK_CONNECTION;


typedef class FasTrackConnector {
 public:
  FasTrackConnector(std::ifstream&);
  ~FasTrackConnector();

  float m_etaBin;

  std::map<int, std::vector<FASTRACK_CONNECTION*> > m_connMap;

} FASTRACK_CONNECTOR;

#endif
