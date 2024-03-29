# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack


# The name of the package:
atlas_subdir( EgammaAnalysisAlgorithms )

atlas_add_library( EgammaAnalysisAlgorithmsLib
   EgammaAnalysisAlgorithms/*.h EgammaAnalysisAlgorithms/*.icc Root/*.cxx
   PUBLIC_HEADERS EgammaAnalysisAlgorithms
   LINK_LIBRARIES xAODEgamma SelectionHelpersLib SystematicsHandlesLib
   IsolationSelectionLib AnaAlgorithmLib IsolationCorrectionsLib FourMomUtils
   EgammaAnalysisInterfacesLib egammaUtils CxxUtils )

atlas_add_dictionary( EgammaAnalysisAlgorithmsDict
   EgammaAnalysisAlgorithms/EgammaAnalysisAlgorithmsDict.h
   EgammaAnalysisAlgorithms/selection.xml
   LINK_LIBRARIES EgammaAnalysisAlgorithmsLib )

if( NOT XAOD_STANDALONE )
   atlas_add_component( EgammaAnalysisAlgorithms
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES GaudiKernel EgammaAnalysisAlgorithmsLib )
endif()

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/*_jobOptions.py )
atlas_install_scripts( share/*_eljob.py )

if( XAOD_STANDALONE )
   atlas_add_test( testJobData
      SCRIPT EgammaAnalysisAlgorithmsTest_eljob.py --data-type data --unit-test
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( testJobFullSim
      SCRIPT EgammaAnalysisAlgorithmsTest_eljob.py --data-type mc --unit-test
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
    # No recommendation available yet for fastsim
    # atlas_add_test( testJobFastSim
    #  SCRIPT EgammaAnalysisAlgorithmsTest_eljob.py --data-type afii --unit-test
    #  POST_EXEC_SCRIPT nopost.sh
    #  PROPERTIES TIMEOUT 600 )
    
    # DNN not currently supported on DAOD_PHYS
    # atlas_add_test( testJobDataDNN
    #    SCRIPT EgammaAnalysisAlgorithmsTest_eljob.py --data-type data --dnn --unit-test
    #    POST_EXEC_SCRIPT nopost.sh
    #    PROPERTIES TIMEOUT 600 )
    # atlas_add_test( testJobFullSimDNN
    #    SCRIPT EgammaAnalysisAlgorithmsTest_eljob.py --data-type mc --dnn --unit-test
    #    POST_EXEC_SCRIPT nopost.sh
    #    PROPERTIES TIMEOUT 600 )
    # atlas_add_test( testJobFastSimDNN
    #    SCRIPT EgammaAnalysisAlgorithmsTest_eljob.py --data-type afii --dnn --unit-test
    #    POST_EXEC_SCRIPT nopost.sh
    #    PROPERTIES TIMEOUT 600 )
else()
   atlas_add_test( testJobData
      SCRIPT athena.py
      EgammaAnalysisAlgorithms/EgammaAnalysisAlgorithmsTest_jobOptions.py - --data-type data
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( testJobFullSim
      SCRIPT athena.py
      EgammaAnalysisAlgorithms/EgammaAnalysisAlgorithmsTest_jobOptions.py - --data-type mc
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )

    # No recommendation available yet for fastsim
    # atlas_add_test( testJobFastSim
    #   SCRIPT athena.py
    #   EgammaAnalysisAlgorithms/EgammaAnalysisAlgorithmsTest_jobOptions.py - --data-type afii
    #   POST_EXEC_SCRIPT nopost.sh
    #   PROPERTIES TIMEOUT 600 )

    # atlas_add_test( testJobDataDNN
    #    SCRIPT athena.py
    #    EgammaAnalysisAlgorithms/EgammaAnalysisAlgorithmsTest_jobOptions.py - --data-type data --dnn
    #    POST_EXEC_SCRIPT nopost.sh
    #    PROPERTIES TIMEOUT 600 )
    # atlas_add_test( testJobFullSimDNN
    #    SCRIPT athena.py
    #    EgammaAnalysisAlgorithms/EgammaAnalysisAlgorithmsTest_jobOptions.py - --data-type mc --dnn
    #    POST_EXEC_SCRIPT nopost.sh
    #    PROPERTIES TIMEOUT 600 )
    # atlas_add_test( testJobFastSimDNN
    #    SCRIPT athena.py
    #    EgammaAnalysisAlgorithms/EgammaAnalysisAlgorithmsTest_jobOptions.py - --data-type afii --dnn
    #    POST_EXEC_SCRIPT nopost.sh
    #    PROPERTIES TIMEOUT 600 )
endif()
