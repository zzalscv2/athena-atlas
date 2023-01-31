/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//
// This is an implemetation of the GeoVDetectorManager which
// is needed by Geo2G4 to get the treetop of the geoetry to 
// have the conversion to G4. Only the basic methods are implemented
//


#ifndef SERVICEEXTENSIONGEOMODELXML_SERVICEEXTENSIONMANAGER_H
#define SERVICEEXTENSIONGEOMODELXML_SERVICEEXTENSIONMANAGER_H

#include "GeoModelKernel/GeoVPhysVol.h"
#include "GeoModelKernel/GeoVDetectorManager.h"
#include "AthenaKernel/CLASS_DEF.h"


namespace ITk {

class ServiceExtensionManager : public GeoVDetectorManager  {

 public:
  
  // Constructor
  ServiceExtensionManager(const std::string & name);

  // Destructor
  ~ServiceExtensionManager();

  // Access to raw geometry:
  virtual unsigned int getNumTreeTops() const;
  virtual PVConstLink getTreeTop(unsigned int i) const;

  // Add a Tree top:
  void addTreeTop(const PVConstLink&);

 private:  
  // prevent copy and assignment
  const ServiceExtensionManager & operator=(const ServiceExtensionManager &right);
  ServiceExtensionManager(const ServiceExtensionManager &right);

  // data members
  std::vector<PVConstLink> m_volume;
};

} // namespace InDetDD

CLASS_DEF(ITk::ServiceExtensionManager, 61947039, 1)

#endif
