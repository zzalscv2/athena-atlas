/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CaloGeometryFromFile_h
#define CaloGeometryFromFile_h

#include "ISF_FastCaloSimParametrization/CaloGeometry.h"

class CaloGeometryFromFile : public CaloGeometry
{
public:
  CaloGeometryFromFile();

  bool LoadGeometryFromFile(const std::string& fileName,
                            const std::string& treeName,
                            const std::string& hashFileName
                            = "/eos/atlas/atlascerngroupdisk/proj-simul/"
                              "CaloGeometry/cellId_vs_cellHashId_map.txt");
  bool LoadFCalGeometryFromFiles(const std::array<std::string, 3> &fileNames);
  void DrawFCalGraph(int isam, int color);

private:
  void calculateFCalRminRmax();
  bool checkFCalGeometryConsistency();
};

#endif // CaloGeometryFromFile_h
