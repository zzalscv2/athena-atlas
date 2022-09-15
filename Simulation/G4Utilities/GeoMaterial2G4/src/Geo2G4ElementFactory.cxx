/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "Geo2G4ElementFactory.h"
#include "GeoModelKernel/GeoElement.h"
#include "G4Element.hh"
#include <iostream>

Geo2G4ElementFactory::Geo2G4ElementFactory()
{
}

G4Element *Geo2G4ElementFactory::Build(const GeoElement* theEle)
{
  //
  // Check if this element has already been defined.
  //
  const std::string& sym = theEle->getSymbol();
  const auto itr = m_definedElements.find(sym);
  if (itr != m_definedElements.end())
    {
      return itr->second;
    }
  G4Element* elm = new G4Element(theEle->getName(),
                                 sym,
                                 theEle->getZ(),
                                 theEle->getA());

  m_definedElements.emplace(sym, elm);
  return elm;
}
