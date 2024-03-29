# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( CaloEvent )

# Extra dependencies, based on the environment:
set( extra_libs )
if( NOT GENERATIONBASE )
   set( extra_libs LArRawConditions )
endif()

# External dependencies:
find_package( CLHEP )
find_package( Boost )

# Component(s) in the package:
atlas_add_library( CaloEvent CaloEvent/*.h CaloEvent/*.icc src/*.cxx
   PUBLIC_HEADERS CaloEvent
   PRIVATE_INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
   LINK_LIBRARIES CaloConditions CaloGeoHelpers CaloIdentifier AthAllocators
   AthContainers AthLinks CxxUtils Navigation GeoPrimitives
   Identifier EventKernel FourMom NavFourMom CaloDetDescrLib ${extra_libs}
   PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} AthenaKernel )

atlas_add_dictionary( CaloEventDict
   CaloEvent/CaloEventDict.h CaloEvent/selection.xml
   LINK_LIBRARIES CaloEvent
   NAVIGABLES CaloCellContainer CaloClusterContainer
   DATA_LINKS CaloTowerContainer
   ELEMENT_LINKS CaloShowerContainer CaloCellLinkContainer
   CaloClusterCellLinkContainer )

atlas_install_joboptions( share/*.txt )

# Helper variable:
set( _patterns
   "^lar decode|initialize_from_dict|mask/zero|channel range|^AtlasDetectorID::" )

# Test(s) in the package:
atlas_add_test( CaloCellContainer_test
   SOURCES test/CaloCellContainer_test.cxx
   LINK_LIBRARIES CaloDetDescrLib CaloIdentifier TestTools IdDictParser
   GaudiKernel CaloEvent
   LOG_IGNORE_PATTERN "${_patterns}" )

atlas_add_test( CaloConstCellContainer_test
   SOURCES test/CaloConstCellContainer_test.cxx
   LINK_LIBRARIES CaloDetDescrLib CaloIdentifier TestTools IdDictParser
   GaudiKernel CaloEvent
   LOG_IGNORE_PATTERN "${_patterns}" )

atlas_add_test( CaloTowerContainer_test
   SOURCES test/CaloTowerContainer_test.cxx
   LINK_LIBRARIES TestTools CaloEvent  ${CLHEP_LIBRARIES}
   LOG_IGNORE_PATTERN "${_patterns}" )

atlas_add_test( CaloTopoTowerContainer_test
   SOURCES test/CaloTopoTowerContainer_test.cxx
   LINK_LIBRARIES TestTools StoreGateLib GaudiKernel CaloEvent ${CLHEP_LIBRARIES}
   LOG_IGNORE_PATTERN "${_patterns}" )

atlas_add_test( CaloCellPrefetchIterator_test
   SOURCES test/CaloCellPrefetchIterator_test.cxx
   LINK_LIBRARIES CaloDetDescrLib CaloIdentifier TestTools IdDictParser
   GaudiKernel CaloEvent
   LOG_IGNORE_PATTERN "${_patterns}" )

atlas_add_test( CaloSamplingData_test
   SOURCES test/CaloSamplingData_test.cxx
   LINK_LIBRARIES CaloEvent
   LOG_IGNORE_PATTERN "${_patterns}" )

atlas_add_test( CaloTowerSeg_test
   SOURCES test/CaloTowerSeg_test.cxx
   LINK_LIBRARIES CaloEvent
   LOG_IGNORE_PATTERN "${_patterns}" )

atlas_add_test( CaloTester_test
   SOURCES test/CaloTester_test.cxx
   LINK_LIBRARIES CaloEvent
   LOG_IGNORE_PATTERN "${_patterns}" )
