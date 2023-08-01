/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 thick support panel on one side of the RPC
 ------------------------------------------
 ***************************************************************************/

//<doc><file>	$Id: DED_Technology.cxx,v 1.1 2008-07-31 10:57:55 dquarrie Exp $
//<version>	$Name: not supported by cvs2svn $

#include <utility>

#include "MuonGeoModel/DED_Technology.h"

namespace MuonGM {

    DED::DED(MYSQL& mysql, std::string s) : Technology(mysql, std::move(s)) {}

} // namespace MuonGM
