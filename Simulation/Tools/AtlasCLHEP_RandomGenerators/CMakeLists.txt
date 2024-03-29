################################################################################
# Package: AtlasCLHEP_RandomGenerators
################################################################################

# Declare the package name:
atlas_subdir( AtlasCLHEP_RandomGenerators )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( dSFMT )

# Component(s) in the package:
atlas_add_library( AtlasCLHEP_RandomGenerators
                   src/*.cxx
                   PUBLIC_HEADERS AtlasCLHEP_RandomGenerators
                   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                   PRIVATE_INCLUDE_DIRS  ${ROOT_INCLUDE_DIRS} ${DSFMT_INCLUDE_DIRS}
                   DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${CLHEP_LIBRARIES}
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} ${DSFMT_LIBRARIES} AthenaBaseComps AthenaKernel GaudiKernel CxxUtils )

atlas_add_component( AtlasCLHEP_RandomGenerators_test
                     src/components/*.cxx
                     src/ValidationTests/*.cxx
                     PUBLIC_HEADERS AtlasCLHEP_RandomGenerators
                     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                     PRIVATE_INCLUDE_DIRS  ${ROOT_INCLUDE_DIRS} ${DSFMT_INCLUDE_DIRS}
                     DEFINITIONS ${CLHEP_DEFINITIONS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES}
                     PRIVATE_LINK_LIBRARIES  ${ROOT_LIBRARIES} ${DSFMT_LIBRARIES} AthenaBaseComps AthenaKernel GaudiKernel AtlasCLHEP_RandomGenerators )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/*.py )
