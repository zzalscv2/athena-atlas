# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( GeoModelSvc )

# External dependencies:
find_package( GeoModel COMPONENTS GeoModelKernel GeoModelDBManager GeoModelRead)

# Component(s) in the package:
atlas_add_component( GeoModelSvc
   src/*.h src/*.cxx src/components/*.cxx
   INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
   LINK_LIBRARIES ${GEOMODEL_LIBRARIES} AthenaBaseComps AthenaKernel CxxUtils SGTools
                  StoreGateLib GeoModelInterfaces GeoModelUtilities GaudiKernel
                  RDBAccessSvcLib EventInfoMgtLib )

