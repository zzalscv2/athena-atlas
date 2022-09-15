/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMATERIAL2G4_ElementFactory_H
#define GEOMATERIAL2G4_ElementFactory_H

class G4Element;
class GeoElement;

#include <unordered_map>
#include <string>
typedef std::unordered_map<std::string, G4Element*> elList;

class Geo2G4ElementFactory {
public:
  Geo2G4ElementFactory();
  G4Element* Build(const GeoElement*);
private:
  elList m_definedElements;
};

#endif
