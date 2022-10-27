/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "SCT_GeoModel/SCT_ComponentFactory.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "GeoModelRead/ReadGeoModel.h"
#include <sstream>
#include <string>

using InDetDD::SCT_DetectorManager;

const double SCT_ComponentFactory::s_epsilon = 1.0e-6 * Gaudi::Units::mm;

SCT_ComponentFactory::SCT_ComponentFactory(const std::string & name,
                                           InDetDD::SCT_DetectorManager* detectorManager,
                                           SCT_GeometryManager* geometryManager,
                                           SCT_MaterialManager* materials)
  : m_detectorManager(detectorManager), 
    m_geometryManager(geometryManager),
    m_materials(materials),
    m_name(name)
{}

SCT_ComponentFactory::~SCT_ComponentFactory() 
{}

std::string 
SCT_ComponentFactory::intToString(int i) const
{
  std::ostringstream str;
  str << i;
  return str.str();
}

double
SCT_ComponentFactory::epsilon() const
{
  return s_epsilon;
}


SCT_UniqueComponentFactory::SCT_UniqueComponentFactory(const std::string & name,
			    InDetDD::SCT_DetectorManager* detectorManager,
                            SCT_GeometryManager* geometryManager,
                            SCT_MaterialManager* materials, 
                            GeoModelIO::ReadGeoModel* sqliteReader) :
  SCT_ComponentFactory(name, detectorManager, geometryManager, materials),
  m_logVolume(nullptr),
  m_sqliteReader(sqliteReader)
{
  if (sqliteReader) {
    m_mapFPV = std::make_unique<std::map<std::string, GeoFullPhysVol*>>         (m_sqliteReader->getPublishedNodes<std::string, GeoFullPhysVol*>("SCT"));
    m_mapAX  = std::make_unique< std::map<std::string, GeoAlignableTransform*>> (m_sqliteReader->getPublishedNodes<std::string, GeoAlignableTransform*>("SCT"));
  }
};
