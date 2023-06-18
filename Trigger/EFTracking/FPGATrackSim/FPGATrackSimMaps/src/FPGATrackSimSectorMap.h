/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATrackSimSECTORMAP_H
#define FPGATrackSimSECTORMAP_H

#include <vector>
#include <map>

/* 
   A simple map that links sectors in different cases with different layers
*/
class FPGATrackSimSectorMap {
  typedef std::map<int,int> mapint;
  typedef std::map<int,mapint> mapint2;
private:
  mapint2 m_data;
public:
  FPGATrackSimSectorMap() = default;
  FPGATrackSimSectorMap(const char *fname) { LoadFromFile(fname); }
  ~FPGATrackSimSectorMap() = default;
  void SetSector(int, int, int);
  int GetSector(int, int);
  void LoadFromFile(const char *);
  void CreateFile(const char *,const char *, const char *);
  void Dump();
  static std::map<int,int> makeLookup(const char* fname);
};

#endif
