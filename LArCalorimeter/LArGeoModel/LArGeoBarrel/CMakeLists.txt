#Copyright (C) 2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LArGeoBarrel )

# External dependencies:
find_package( GeoModel COMPONENTS GeoModelKernel GeoModelDBManager GeoModelRead)

# Component(s) in the package:
atlas_add_library( LArGeoBarrel
                   src/*.cxx
                   PUBLIC_HEADERS LArGeoBarrel
                   INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
                   LINK_LIBRARIES ${GEOMODEL_LIBRARIES} LArGeoCode StoreGateLib
                   PRIVATE_LINK_LIBRARIES GeoModelUtilities GaudiKernel LArReadoutGeometry RDBAccessSvcLib CrackRegionGeoModel )

