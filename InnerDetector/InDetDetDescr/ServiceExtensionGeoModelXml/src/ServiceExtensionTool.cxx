/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "ServiceExtensionTool.h"

#include <GeoModelKernel/GeoPhysVol.h>
#include <GeoModelUtilities/GeoModelExperiment.h>
#include "GeoModelXml/GmxInterface.h"
#include <SGTools/DataProxy.h>
#include "ServiceExtensionManager.h"

namespace ITk
{

ServiceExtensionTool::ServiceExtensionTool(const std::string &type,
                                     const std::string &name,
                                     const IInterface *parent)
 : GeoModelXmlTool(type, name, parent)
{
}

StatusCode ServiceExtensionTool::create()
{
  // retrieve the common stuff
  ATH_CHECK(createBaseTool());

  GeoModelExperiment *theExpt = nullptr;
  ATH_CHECK(detStore()->retrieve(theExpt, "ATLAS"));
  auto *manager = new ServiceExtensionManager();
  if (!isAvailable(m_node, m_table)) {
      ATH_MSG_ERROR("No ServiceExtension geometry found. ServiceExtension can not be built.");
      return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Building Service Extension");
  
  GeoPhysVol *world = &*theExpt->getPhysVol();

  // Load the geometry, create the volume,
  // and then find the volume index within the world to allow it to be added
  // last two arguments are the location in the DB to look for the clob
  // (may want to make those configurables)
  GmxInterface gmxInterface;
  const GeoVPhysVol * topVol = createTopVolume(world, gmxInterface, m_node,m_table, m_containingDetectorName, m_envelopeVolumeName);
  if (!topVol) {
    ATH_MSG_FATAL("Could not find the Top Volume!!!");
    return StatusCode::FAILURE;
  }
  
  manager->addTreeTop(topVol);
  m_detManager = manager;
  
  ATH_CHECK(detStore()->record(m_detManager, m_detManager->getName()));
  theExpt->addManager(m_detManager);
  
  return StatusCode::SUCCESS;
}


} // namespace ITk
