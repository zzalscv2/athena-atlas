################################################################################
# Package: AmdcDb
################################################################################

# Declare the package name:
atlas_subdir( AmdcDb )

# External dependencies:
find_package( CORAL COMPONENTS CoralBase CoralKernel RelationalAccess )

# Component(s) in the package:
atlas_add_library( AmdcDbLib
                   src/*.cxx
                   PUBLIC_HEADERS AmdcDb
                   INCLUDE_DIRS  ${CORAL_INCLUDE_DIRS}
                   LINK_LIBRARIES ${CORAL_LIBRARIES} AthenaBaseComps AthenaKernel GaudiKernel StoreGateLib AmdcAthLib RDBAccessSvcLib
                   PRIVATE_LINK_LIBRARIES AmdcCore )

atlas_add_component( AmdcDb
                     src/components/*.cxx
                     INCLUDE_DIRS ${CORAL_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CORAL_LIBRARIES} AthenaBaseComps AthenaKernel GaudiKernel StoreGateLib AmdcAthLib AmdcCore AmdcDbLib )

# Install files from the package:
atlas_install_joboptions( share/*.py )
atlas_install_scripts( AmdcDbCnvSqlToolKit/* )
