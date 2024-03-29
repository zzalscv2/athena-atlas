# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrkExTools )

# External dependencies:
find_package( CLHEP )
find_package( Boost COMPONENTS unit_test_framework)

# Component(s) in the package:
atlas_add_component( TrkExTools
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                     LINK_LIBRARIES ${Boost_LIBRARIES} ${CLHEP_LIBRARIES} AthenaBaseComps AthenaKernel GeoPrimitives
		             EventPrimitives xAODTracking GaudiKernel TrkDetDescrUtils TrkGeometry TrkSurfaces TrkVolumes TrkEventPrimitives
		             TrkNeutralParameters TrkParameters TrkExInterfaces TrkExUtils AthContainers CxxUtils TrkDetDescrInterfaces TrkEventUtils
		             TrkPatternParameters
		             TrkMaterialOnTrack TrkParticleBase TrkTrack StoreGateLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Tests in the package:
atlas_add_test( ObjContainer_boost_test
  SOURCES test/ObjContainer_boost_test.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS} 
  LINK_LIBRARIES ${Boost_LIBRARIES}  CxxUtils
  POST_EXEC_SCRIPT nopost.sh
)
	
atlas_add_test( TrkExToolsStringUtility_test	
  SOURCES 
  test/TrkExToolsStringUtility_test.cxx src/TrkExToolsStringUtility.cxx
  INCLUDE_DIRS  ${Boost_INCLUDE_DIRS}  
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils
  POST_EXEC_SCRIPT nopost.sh
)

