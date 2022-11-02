/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include "InDetGeoModelUtils/ExtraMaterial.h"
#include "InDetGeoModelUtils/GenericTubeMaker.h"
#include "InDetGeoModelUtils/TubeVolData.h"
#include "InDetGeoModelUtils/DistortedMaterialManager.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoTubs.h"
#include "GeoModelKernel/GeoCons.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoMaterial.h"

#include "GeoModelInterfaces/StoredMaterialManager.h"

#include <iostream>
#include <sstream>
#include <string>
#include <utility>


namespace InDetDD {
  ExtraMaterial::ExtraMaterial(IRDBRecordset_ptr xMatTable, StoredMaterialManager* matManager)
    : AthMessaging("ExtraMaterial")
    , m_xMatTable(std::move(xMatTable))
    , m_matManager(matManager)
  {}

  ExtraMaterial::ExtraMaterial(DistortedMaterialManager* manager)
    : AthMessaging("ExtraMaterial")
    , m_xMatTable(manager->extraMaterialTable())
    , m_matManager(manager->materialManager())
  {}

  void
  ExtraMaterial::add(GeoPhysVol* parent, const std::string& region, double zParent) {
    add(parent, nullptr, region, zParent);
  }

  void
  ExtraMaterial::add(GeoFullPhysVol* parent, const std::string& region, double zParent) {
    add(nullptr, parent, region, zParent);
  }

  void
  ExtraMaterial::add(GeoPhysVol* parent, GeoFullPhysVol* fullparent, const std::string& region, double zParent) {
    ATH_MSG_DEBUG("Adding Extra material for region: " << region << ", zParent = " << zParent);

    for (unsigned int i = 0; i < m_xMatTable->size(); i++) {
      std::ostringstream volnamestr;
      volnamestr << "ExtraMaterial" << i;

      ATH_MSG_VERBOSE("In Extra material " << i);

      if ((*m_xMatTable)[i]->getString("REGION") == region) {
        ATH_MSG_VERBOSE("Extra material Match " << i);

	if(!m_matManager) {
	  std::string errorMessage("Null pointer to Stored Material Manager!");
	  ATH_MSG_FATAL(errorMessage);
	  throw std::runtime_error(errorMessage);
	}

        GenericTubeMaker tubeHelper((*m_xMatTable)[i]);
        const GeoMaterial* material = m_matManager->getMaterial(tubeHelper.volData().material());
        const GeoShape* shape = tubeHelper.buildShape();
        GeoLogVol* logVol = new GeoLogVol(volnamestr.str(), shape, material);
        GeoPhysVol* physVol = new GeoPhysVol(logVol);

        if (parent) {
          tubeHelper.placeVolume(parent, physVol, zParent);
        } else {
          tubeHelper.placeVolume(fullparent, physVol, zParent);
        }
      }
    }
  }
} // end namespace
