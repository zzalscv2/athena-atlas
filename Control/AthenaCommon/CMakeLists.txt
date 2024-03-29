# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AthenaCommon )

if (XAOD_STANDALONE)
    # Install files from the package:
    atlas_install_python_modules( python/SystemOfUnits.py
                                  python/Logging.py
                                  python/Constants.py
                                  POST_BUILD_CMD ${ATLAS_FLAKE8} )
else()
    # Set up specific Athena runtime environment:
    set( AthenaCommonEnvironment_DIR ${CMAKE_CURRENT_SOURCE_DIR}
       CACHE PATH "Location of AthenaCommonEnvironmentConfig.cmake" )
    find_package( AthenaCommonEnvironment )
    
    # Install files from the package:
    atlas_install_python_modules( python/*.py python/Utils
                                  POST_BUILD_CMD ${ATLAS_FLAKE8} )
    
    atlas_install_joboptions( share/Preparation.py
                              share/Execution.py
                              share/Atlas.UnixStandardJob.py
                              share/zeroJO.py
                              share/Atlas_Gen.UnixStandardJob.py
                              share/MemTraceInclude.py
                              share/runbatch.py
                              test/*.py )
    
    atlas_install_scripts( share/athena_preload.sh
                           share/ThinCAWrapper.sh )

    # Python scripts checked by flake8:
    atlas_install_scripts( share/athena.py
                           share/chappy.py
                           POST_BUILD_CMD ${ATLAS_FLAKE8} )
    
    # Aliases:
    atlas_add_alias( athena "athena.py" )
    
    # Tests:
    atlas_add_test( AthAppMgrUnitTests SCRIPT test/test_AthAppMgrUnitTests.sh
       PROPERTIES TIMEOUT 300
       LOG_IGNORE_PATTERN "Warning in <TFile::Init>: no StreamerInfo found|^Ran .* tests in|built on" )
    
    atlas_add_test( ConfigurableUnitTests SCRIPT test/test_ConfigurableUnitTests.sh
       PROPERTIES TIMEOUT 300
       LOG_IGNORE_PATTERN "Warning in <TFile::Init>: no StreamerInfo found|^Ran .* tests in" )
    
    atlas_add_test( JobPropertiesUnitTests SCRIPT test/test_JobPropertiesUnitTests.sh
       LOG_IGNORE_PATTERN "Warning in <TFile::Init>: no StreamerInfo found|^Ran .* tests in" )
    
    atlas_add_test( KeyStoreUnitTests SCRIPT test/test_KeyStoreUnitTests.sh
       LOG_IGNORE_PATTERN "Warning in <TFile::Init>: no StreamerInfo found|^Ran .* tests in|^outFileName: " )
    
    atlas_add_test( CFElementsTest SCRIPT python -m unittest -v AthenaCommon.CFElements
       POST_EXEC_SCRIPT nopost.sh )
    
    if (NOT XAOD_ANALYSIS)
       atlas_add_test( JobOptionsUnitTests SCRIPT test/test_JobOptionsUnitTests.sh
          LOG_IGNORE_PATTERN "Warning in <TFile::Init>: no StreamerInfo found|^Ran .* tests in" )
    endif()
endif()
