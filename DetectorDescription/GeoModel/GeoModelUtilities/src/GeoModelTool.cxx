/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BUILDVP1LIGHT

#include "GeoModelUtilities/GeoModelTool.h"

/**
 ** Constructor(s)
 **/
GeoModelTool::GeoModelTool( const std::string& type, const std::string& name, const IInterface* parent )
  : base_class( type, name, parent )
  , m_detector(0)
{
}

/**
 ** The Detector Node corresponding to this tool
 **/
GeoVDetectorManager* GeoModelTool::manager() {
  return m_detector;
}
const GeoVDetectorManager* GeoModelTool::manager() const {
  return m_detector;
}

StatusCode GeoModelTool::clear()
{
  return StatusCode::SUCCESS;
}

StatusCode GeoModelTool::align(IOVSVC_CALLBACK_ARGS)
{
  return StatusCode::SUCCESS;
}

StatusCode GeoModelTool::registerCallback()
{
  // Return Failure since no function has been registered
  return StatusCode::FAILURE;
}

#endif // BUILDVP1LIGHT
