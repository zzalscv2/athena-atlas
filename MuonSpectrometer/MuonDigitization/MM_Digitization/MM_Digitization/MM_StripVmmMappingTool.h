/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// MMStripVmmMappingTool.h

#ifndef MMSTRIPVMMMAPPINGTOOL_H
#define MMSTRIPVMMMAPPINGTOOL_H
#include <string>
#include <tuple>

class electronics {
public:
    int elec(int stripNumber, const std::string& tech, int off_stationEta, int chMax);
};

#endif
