# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Declare the package name:
atlas_subdir( HGTD_RawData )

find_package(Boost COMPONENTS unit_test_framework)

atlas_add_library( HGTD_RawData
                src/*.cxx
                PUBLIC_HEADERS HGTD_RawData
                LINK_LIBRARIES Identifier EventContainers AthContainers AthenaKernel)

set( _jobOPath "${CMAKE_CURRENT_SOURCE_DIR}/share" )
set( _jobOPath "${_jobOPath}:${CMAKE_JOBOPT_OUTPUT_DIRECTORY}" )
set( _jobOPath "${_jobOPath}:$ENV{JOBOPTSEARCHPATH}" )

atlas_add_test(test_HGTD_RDO
           SOURCES test/test_HGTD_RDO.cxx
           INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
           LINK_LIBRARIES ${Boost_LIBRARIES} HGTD_RawData
           POST_EXEC_SCRIPT nopost.sh)

atlas_add_test(test_HGTD_RDO_Collection
          SOURCES test/test_HGTD_RDO_Collection.cxx
          INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
          LINK_LIBRARIES ${Boost_LIBRARIES} HGTD_RawData
          POST_EXEC_SCRIPT nopost.sh)

atlas_add_test(test_HGTD_RDO_Container
          SOURCES test/test_HGTD_RDO_Container.cxx
          INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
          LINK_LIBRARIES ${Boost_LIBRARIES} HGTD_RawData TestTools SGTools
          GaudiKernel StoreGateLib
          ENVIRONMENT "JOBOPTSEARCHPATH=${_jobOPath}"
          POST_EXEC_SCRIPT nopost.sh)
