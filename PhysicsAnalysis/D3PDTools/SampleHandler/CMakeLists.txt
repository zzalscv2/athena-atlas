# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( SampleHandler )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO ROOTTPython Proof )
find_package( GTest )
find_package( Boost )

# Component(s) in the package:
atlas_add_root_dictionary( SampleHandler
   SampleHandlerDictSource
   ROOT_HEADERS
   SampleHandler/DiskList.h SampleHandler/DiskListEOS.h
   SampleHandler/DiskListLocal.h SampleHandler/DiskListSRM.h
   SampleHandler/DiskListXRD.h SampleHandler/DiskOutput.h
   SampleHandler/DiskOutputLocal.h SampleHandler/DiskOutputXRD.h
   SampleHandler/DiskWriter.h SampleHandler/DiskWriterLocal.h
   SampleHandler/DiskWriterXRD.h SampleHandler/Global.h
   SampleHandler/Meta.h SampleHandler/MetaData.h SampleHandler/MetaNames.h
   SampleHandler/MetaDataQuery.h SampleHandler/MetaDataSample.h
   SampleHandler/MetaFields.h SampleHandler/MetaObject.h
   SampleHandler/MetaVector.h SampleHandler/Sample.h
   SampleHandler/SampleComposite.h SampleHandler/SampleGrid.h
   SampleHandler/SampleHandler.h SampleHandler/SampleHist.h
   SampleHandler/SampleLocal.h SampleHandler/SampleMeta.h
   SampleHandler/SamplePtr.h SampleHandler/ScanDir.h SampleHandler/TagList.h
   SampleHandler/ToolsDiscovery.h SampleHandler/ToolsDuplicates.h
   SampleHandler/ToolsJoin.h SampleHandler/ToolsMeta.h
   SampleHandler/ToolsSplit.h SampleHandler/fetch.h Root/LinkDef.h
   EXTERNAL_PACKAGES ROOT )

atlas_add_library( SampleHandler
   SampleHandler/*.h Root/*.cxx ${SampleHandlerDictSource}
   PUBLIC_HEADERS SampleHandler
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${BOOST_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} AsgMessagingLib RootCoreUtils)

# Test(s) in the package:
function( _add_gtest cxx_name )
   get_filename_component( test_name ${cxx_name} NAME_WE )
   atlas_add_test( ${test_name}
      SOURCES test/${test_name}.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} ${GTEST_LIBRARIES} RootCoreUtils
      AsgTestingLib AsgTools SampleHandler
      POST_EXEC_SCRIPT nopost.sh )
endfunction( _add_gtest )

file( GLOB _gtests test/gt_*.cxx )
foreach( gtest ${_gtests} )
   _add_gtest( ${gtest} )
endforeach()

# Ok, this should be an automatically run test, but it requires a grid
# certificate, meaning right now it can't run in the CI or nightly.
# Once I figure out how to do this, I should switch this back to being
# a test.
atlas_add_executable (SampleHandler_test_GridTools SOURCES test/it_GridTools.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} ${GTEST_LIBRARIES} AsgTestingLib RootCoreUtils SampleHandler )

function( _add_test cxx_name )
   get_filename_component( test_name ${cxx_name} NAME_WE )
   atlas_add_test( ${test_name}
      SOURCES test/${test_name}.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} RootCoreUtils SampleHandler
      POST_EXEC_SCRIPT nopost.sh )
endfunction( _add_test )

file( GLOB _tests test/ut_*.cxx )
foreach( test ${_tests} lnk_sample.cxx )
   _add_gtest( ${test} )
endforeach()
