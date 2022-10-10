# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( AnalysisAlgorithmsConfig )

atlas_install_python_modules( python/*.py )



if( XAOD_STANDALONE )

   atlas_install_scripts( share/*_eljob.py )

   atlas_add_test( testJobData
      SCRIPT FullCPAlgorithmsTest_eljob.py --data-type data --unit-test --direct-driver
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( testJobFullSim
      SCRIPT FullCPAlgorithmsTest_eljob.py --data-type mc --unit-test --direct-driver
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( testJobFastSim
      SCRIPT FullCPAlgorithmsTest_eljob.py --data-type afii --unit-test --direct-driver
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )

   atlas_add_test( testJobDataConfig
      SCRIPT FullCPAlgorithmsTest_eljob.py --data-type data --unit-test --direct-driver --block-config
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( testJobFullSimConfig
      SCRIPT FullCPAlgorithmsTest_eljob.py --data-type mc --unit-test --direct-driver --block-config
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( testJobFastSimConfig
      SCRIPT FullCPAlgorithmsTest_eljob.py --data-type afii --unit-test --direct-driver --block-config
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )

   atlas_add_test( testJobDataLite
      SCRIPT FullCPAlgorithmsTest_eljob.py --data-type data --unit-test --direct-driver --physlite --no-physlite-broken
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( testJobFullSimLite
      SCRIPT FullCPAlgorithmsTest_eljob.py --data-type mc --unit-test --direct-driver --physlite --no-physlite-broken
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   atlas_add_test( testJobFastSimLite
      SCRIPT FullCPAlgorithmsTest_eljob.py --data-type afii --unit-test --direct-driver --physlite --no-physlite-broken
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )

   atlas_add_test( testJobDataConfigLite
      SCRIPT FullCPAlgorithmsTest_eljob.py --data-type data --unit-test --direct-driver --block-config --physlite --no-physlite-broken
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )
   # FIX ME: this currently fails on the muons
   # atlas_add_test( testJobFullSimConfigLite
   #    SCRIPT FullCPAlgorithmsTest_eljob.py --data-type mc --unit-test --direct-driver --block-config --physlite --no-physlite-broken
   #    POST_EXEC_SCRIPT nopost.sh
   #    PROPERTIES TIMEOUT 600 )
   atlas_add_test( testJobFastSimConfigLite
      SCRIPT FullCPAlgorithmsTest_eljob.py --data-type afii --unit-test --direct-driver --block-config --physlite --no-physlite-broken
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 600 )

else()

   atlas_install_joboptions( share/*_jobOptions.py )

   atlas_add_test( testJobDataCompare
      SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/share/BlockConfigurationAthenaTest.sh data
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 1800 )

   atlas_add_test( testJobFullSimCompare
      SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/share/BlockConfigurationAthenaTest.sh mc
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 1800 )

   atlas_add_test( testJobFastSimCompare
      SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/share/BlockConfigurationAthenaTest.sh afii
      POST_EXEC_SCRIPT nopost.sh
      PROPERTIES TIMEOUT 1800 )

   # FIXME: there are currently no configuration blocks that work on
   # PHYSLITE, reenable this when there are
   # atlas_add_test( testJobDataCompareLite
   #    SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/share/BlockConfigurationAthenaTest.sh data --physlite
   #    POST_EXEC_SCRIPT nopost.sh
   #    PROPERTIES TIMEOUT 600 )

   #  atlas_add_test( testJobFullSimCompareLite
   #    SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/share/BlockConfigurationAthenaTest.sh mc --physlite
   #    POST_EXEC_SCRIPT nopost.sh
   #    PROPERTIES TIMEOUT 600 )

   # atlas_add_test( testJobFastSimCompareLite
   #    SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/share/BlockConfigurationAthenaTest.sh afii --physlite
   #    POST_EXEC_SCRIPT nopost.sh
   #    PROPERTIES TIMEOUT 600 )

endif()
