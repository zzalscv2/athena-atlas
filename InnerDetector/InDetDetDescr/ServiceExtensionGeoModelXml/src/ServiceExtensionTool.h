/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ServiceExtensionGeoModelXml_SERVICEEXTENSIONTOOL_H
#define ServiceExtensionGeoModelXml_SERVICEEXTENSIONTOOL_H
//
//
#include <InDetGeoModelUtils/GeoModelXmlTool.h>

#include <memory>

class GeoPhysVol;

namespace ITk
{
class ServiceExtensionManager;

class ServiceExtensionTool : public GeoModelXmlTool
{
public:
  ServiceExtensionTool(const std::string &type, const std::string &name, const IInterface *parent);
  virtual ~ServiceExtensionTool() = default;
  virtual StatusCode create() override final;

private:
   const ServiceExtensionManager *m_detManager{};
   Gaudi::Property<std::string> m_containingDetectorName{this, "ContainingDetector", "LArBarrel", "Containing detector name"};
   Gaudi::Property<std::string> m_envelopeVolumeName{this, "EnvelopeVolume", "", "Envelope volume name"};
   Gaudi::Property<std::string> m_node{this,"DataBaseNode","InnerDetector", "Node name in Geometry Database"};
   Gaudi::Property<std::string> m_table{this,"DataBaseTable","ITKSERVICESXDD", "Table name in Geometry Database"};

};
} // namespace ITk

#endif // ServiceExtensionGeoModelXml_SERVICEEXTENSIONTOOL_H
