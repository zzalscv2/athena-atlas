/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonCalibStreamCnvSvc/MuonCalibStreamAddress.h"

// Framework include files
#include "GaudiKernel/GenericAddress.h"
//#include <iostream>

/// Standard Constructor
MuonCalibStreamAddress::MuonCalibStreamAddress(const CLID &clid, const std::string &fname, const std::string &cname, int p1, int p2) :
    GenericAddress(storageType(), clid, fname, cname, p1, p2) {}
MuonCalibStreamAddress::MuonCalibStreamAddress(const CLID &clid) : GenericAddress(storageType(), clid, "", "") {}
