# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# @author Tadej Novak

atlas_subdir( TriggerAnalysisAlgorithms )

atlas_add_library( TriggerAnalysisAlgorithmsLib
   TriggerAnalysisAlgorithms/*.h TriggerAnalysisAlgorithms/*.icc Root/*.cxx
   PUBLIC_HEADERS TriggerAnalysisAlgorithms
   LINK_LIBRARIES xAODEventInfo SelectionHelpersLib SystematicsHandlesLib
      AnaAlgorithmLib AsgTools AsgAnalysisInterfaces EventBookkeeperToolsLib TrigDecisionInterface
      xAODEgamma xAODMuon TriggerAnalysisInterfaces TrigGlobalEfficiencyCorrectionLib TriggerMatchingToolLib
   PRIVATE_LINK_LIBRARIES RootCoreUtils )

atlas_add_dictionary( TriggerAnalysisAlgorithmsDict
   TriggerAnalysisAlgorithms/TriggerAnalysisAlgorithmsDict.h
   TriggerAnalysisAlgorithms/selection.xml
   LINK_LIBRARIES TriggerAnalysisAlgorithmsLib )

if( NOT XAOD_STANDALONE )
   atlas_add_component( TriggerAnalysisAlgorithms
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES GaudiKernel TriggerAnalysisAlgorithmsLib )
endif()

atlas_install_python_modules( python/*.py )
atlas_install_joboptions( share/*_jobOptions.py )
atlas_install_scripts( share/*_eljob.py )

if( XAOD_STANDALONE )
   atlas_add_test( TriggerAlgorithmsTestJobData
      SCRIPT TriggerAlgorithmsTest_eljob.py --data-type data --unit-test
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( TriggerAlgorithmsTestJobFullSim
      SCRIPT TriggerAlgorithmsTest_eljob.py --data-type mc --unit-test
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( TriggerAlgorithmsTestJobFastSim
      SCRIPT TriggerAlgorithmsTest_eljob.py --data-type afii --unit-test
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
else()
   atlas_add_test( TriggerAlgorithmsTestJobData
      SCRIPT athena.py
             TriggerAnalysisAlgorithms/TriggerAlgorithmsTest_jobOptions.py - --data-type data
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( TriggerAlgorithmsTestJobFullSim
      SCRIPT athena.py
             TriggerAnalysisAlgorithms/TriggerAlgorithmsTest_jobOptions.py - --data-type mc
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( TriggerAlgorithmsTestJobFastSim
      SCRIPT athena.py
             TriggerAnalysisAlgorithms/TriggerAlgorithmsTest_jobOptions.py - --data-type afii
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
endif()
