################################################################################
# Package: LArG4EC
################################################################################

# Declare the package name:
atlas_subdir( LArG4EC )

# External dependencies:
find_package( CLHEP )
find_package( CORAL COMPONENTS CoralBase CoralKernel RelationalAccess )
find_package( Geant4 )
find_package( XercesC )

# Component(s) in the package:
atlas_add_library( LArG4EC
                   src/*.cc
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_INCLUDE_DIRS ${XERCESC_INCLUDE_DIRS} ${GEANT4_INCLUDE_DIRS} ${CORAL_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                   PRIVATE_DEFINITIONS ${CLHEP_DEFINITIONS}
                   PRIVATE_LINK_LIBRARIES ${XERCESC_LIBRARIES} ${GEANT4_LIBRARIES} GeoSpecialShapes LArG4Code LArGeoCode CaloG4SimLib StoreGateLib
                   ${CORAL_LIBRARIES} ${CLHEP_LIBRARIES} GeoModelUtilities GaudiKernel LArG4RunControl LArHV PathResolver GeoModelInterfaces RDBAccessSvcLib )

atlas_install_python_modules( python/*.py )
