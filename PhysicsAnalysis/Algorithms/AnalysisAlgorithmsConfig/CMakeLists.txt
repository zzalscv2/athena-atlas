# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( AnalysisAlgorithmsConfig )

atlas_install_python_modules( python/*.py )



if( XAOD_STANDALONE )

   atlas_install_scripts( share/*_eljob.py )

   atlas_add_test( testJobData
      SCRIPT FullCPAlgorithmsTest_eljob.py --data-type data --unit-test
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( testJobFullSim
      SCRIPT FullCPAlgorithmsTest_eljob.py --data-type mc --unit-test
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( testJobFastSim
      SCRIPT FullCPAlgorithmsTest_eljob.py --data-type afii --unit-test
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )

   atlas_add_test( testJobDataConfig
      SCRIPT FullCPAlgorithmsTest_eljob.py --data-type data --unit-test --block-config
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( testJobFullSimConfig
      SCRIPT FullCPAlgorithmsTest_eljob.py --data-type mc --unit-test --block-config
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( testJobFastSimConfig
      SCRIPT FullCPAlgorithmsTest_eljob.py --data-type afii --unit-test --block-config
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )

else()

   atlas_install_joboptions( share/*_jobOptions.py )

   atlas_add_test( testJobDataConfig
      SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/share/BlockConfigurationAthenaTest.sh data
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )

   atlas_add_test( testJobFullSumConfig
      SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/share/BlockConfigurationAthenaTest.sh mc
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )

   atlas_add_test( testJobFastSimConfig
      SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/share/BlockConfigurationAthenaTest.sh afii
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )

endif()
