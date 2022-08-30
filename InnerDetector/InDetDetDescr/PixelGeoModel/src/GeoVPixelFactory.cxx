/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoVPixelFactory.h"

using InDetDD::PixelDetectorManager;

GeoVPixelFactory::GeoVPixelFactory(InDetDD::PixelDetectorManager* ddmgr
				   , PixelGeometryManager* mgr
				   , GeoModelIO::ReadGeoModel* sqliteReader)
  : m_gmt_mgr (mgr)
  , m_mat_mgr (m_gmt_mgr->getMaterialManager())
  , m_DDmgr (ddmgr)
  , m_sqliteReader(sqliteReader)
  , m_epsilon(0.0001)
{
}

GeoVPixelFactory::~GeoVPixelFactory()
{
}
