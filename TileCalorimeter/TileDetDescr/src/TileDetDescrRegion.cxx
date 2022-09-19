/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 Tile Calorimeter detector description package
 -----------------------------------------
***************************************************************************/

#include "TileDetDescr/TileDetDescrRegion.h"
#include "AtlasDetDescr/AtlasDetectorID.h"

#include <iostream>
#include <iomanip>

void TileDetDescrRegion::print() const
{

  AtlasDetectorID id;
  std::cout << std::endl << " TileDetDescrRegion print: "
            << std::endl << std::endl;

  id.print(m_id);
  m_descriptor->print();
}
