# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack


# The name of the package:
atlas_subdir( FTagAnalysisAlgorithms )

atlas_add_library( FTagAnalysisAlgorithmsLib
   FTagAnalysisAlgorithms/*.h FTagAnalysisAlgorithms/*.icc Root/*.cxx
   PUBLIC_HEADERS FTagAnalysisAlgorithms
   LINK_LIBRARIES xAODJet SelectionHelpersLib SystematicsHandlesLib
   AnaAlgorithmLib FTagAnalysisInterfacesLib )

atlas_add_dictionary( FTagAnalysisAlgorithmsDict
   FTagAnalysisAlgorithms/FTagAnalysisAlgorithmsDict.h
   FTagAnalysisAlgorithms/selection.xml
   LINK_LIBRARIES FTagAnalysisAlgorithmsLib )

if( NOT XAOD_STANDALONE )
   atlas_add_component( FTagAnalysisAlgorithms
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES GaudiKernel FTagAnalysisAlgorithmsLib )
endif()

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/*_jobOptions.py )
atlas_install_scripts( share/*_eljob.py )

if( XAOD_STANDALONE )
   atlas_add_test( testJobData
      SCRIPT FTagAnalysisAlgorithmsTest_eljob.py --data-type data --unit-test
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( testJobFullSim
      SCRIPT FTagAnalysisAlgorithmsTest_eljob.py --data-type mc --unit-test
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( testJobFastSim
      SCRIPT FTagAnalysisAlgorithmsTest_eljob.py --data-type afii --unit-test
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
elseif( NOT "${CMAKE_PROJECT_NAME}" STREQUAL "AthDerivation" )
   atlas_add_test( testJobData
      SCRIPT athena.py
      FTagAnalysisAlgorithms/FTagAnalysisAlgorithmsTest_jobOptions.py - --data-type data
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( testJobFullSim
      SCRIPT athena.py
      FTagAnalysisAlgorithms/FTagAnalysisAlgorithmsTest_jobOptions.py - --data-type mc
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( testJobFastSim
      SCRIPT athena.py
      FTagAnalysisAlgorithms/FTagAnalysisAlgorithmsTest_jobOptions.py - --data-type afii
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
endif()

# Unit test for python modules
atlas_add_test( pymodules
                 SCRIPT python -m unittest discover -v -s ${CMAKE_CURRENT_SOURCE_DIR}/test
                 POST_EXEC_SCRIPT nopost.sh )