# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TruthUtils )

# Decide what to do about HepMC(3) / AtlasHepMC.
set( extra_libs )
if( NOT XAOD_STANDALONE )
   list( APPEND extra_libs AtlasHepMCLib )
   message( STATUS "Linking TruthUtils against AtlasHepMC" )
endif()

# Component(s) in the package:
atlas_add_library( TruthUtils
   TruthUtils/*.h Root/*.cxx
   PUBLIC_HEADERS TruthUtils
   LINK_LIBRARIES  ${extra_libs}
   )

# Dictionary
atlas_add_dictionary( TruthUtilsDict
                      TruthUtils/TruthUtilsDict.h
                      TruthUtils/selection.xml
                      LINK_LIBRARIES TruthUtils )

# Python module
atlas_install_python_modules( python/*.py )

# Install helper utility
atlas_install_runtime( util/generator-weights.py )

# Install files from the package:
atlas_install_runtime( share/*.txt )

# Run tests:
atlas_add_test( WeightHelpersTest
                SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/testWeightHelpers.py
                POST_EXEC_SCRIPT nopost.sh )
