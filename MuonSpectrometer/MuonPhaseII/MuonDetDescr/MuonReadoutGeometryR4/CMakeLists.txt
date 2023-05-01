# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonReadoutGeometryR4
################################################################################

# Declare the package name:
atlas_subdir( MuonReadoutGeometryR4 )

# Extra dependencies, based on the environment (no MuonCondSvc needed in AthSimulation):
set( extra_libs )

# External dependencies:
find_package( GeoModel COMPONENTS GeoModelKernel )

atlas_add_library( MuonReadoutGeometryR4
                   src/*.cxx
                   PUBLIC_HEADERS MuonReadoutGeometryR4
                   INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
                   LINK_LIBRARIES ${GEOMODEL_LIBRARIES} AthenaBaseComps AthenaKernel GeoPrimitives Identifier GaudiKernel  
                                  ActsGeometryInterfacesLib ActsGeometryLib MuonReadoutGeometry MuonIdHelpersLib 
                                  StoreGateLib GeoModelUtilities CxxUtils ${extra_libs})
