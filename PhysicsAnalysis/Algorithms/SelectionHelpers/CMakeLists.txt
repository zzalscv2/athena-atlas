# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack


atlas_subdir( SelectionHelpers )

atlas_add_library( SelectionHelpersLib
   SelectionHelpers/*.h SelectionHelpers/*.icc Root/*.cxx
   PUBLIC_HEADERS SelectionHelpers
   LINK_LIBRARIES AthContainers AsgTools CxxUtils xAODBase AnaAlgorithmLib SystematicsHandlesLib PATCoreAcceptLib
   PRIVATE_LINK_LIBRARIES PATCoreLib PATInterfaces )

atlas_add_dictionary( SelectionHelpersDict
   SelectionHelpers/SelectionHelpersDict.h
   SelectionHelpers/selection.xml
   LINK_LIBRARIES SelectionHelpersLib )

if( NOT XAOD_STANDALONE )
   atlas_add_component( SelectionHelpers
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES GaudiKernel SelectionHelpersLib )
endif()

find_package( GTest )

atlas_add_test( gt_ISelectionAccessor
   SOURCES test/gt_ISelectionAccessor.cxx
   INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
   LINK_LIBRARIES ${GTEST_LIBRARIES} AsgTestingLib SelectionHelpersLib xAODJet CxxUtils
   POST_EXEC_SCRIPT nopost.sh )
