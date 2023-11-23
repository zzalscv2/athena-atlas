/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELEXAMPLES_TOYDETECTORTOOL_H
#define GEOMODELEXAMPLES_TOYDETECTORTOOL_H

#ifndef BUILDVP1LIGHT

#include "GeoModelUtilities/GeoModelTool.h"
class ToyDetectorTool final : public GeoModelTool 
{
 public:
  ToyDetectorTool( const std::string& type, const std::string& name, const IInterface* parent );
  virtual ~ToyDetectorTool() override final;
  
  virtual StatusCode create() override final;
 private:
  void printVolume(GeoPVConstLink volime, int level = 0);
};

#endif // BUILDVP1LIGHT

#endif // GEOMODELEXAMPLES_TOYDETECTORTOOL_H
