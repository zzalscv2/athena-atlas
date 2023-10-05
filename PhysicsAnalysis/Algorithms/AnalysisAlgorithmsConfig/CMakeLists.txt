# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( AnalysisAlgorithmsConfig )

atlas_install_python_modules( python/*.py )



if( XAOD_STANDALONE )

   atlas_install_scripts( share/*_eljob.py )

   function( add_test_job NAME DATA_TYPE )

      atlas_add_test( ${NAME}
         SCRIPT FullCPAlgorithmsTest_eljob.py --data-type ${DATA_TYPE} --direct-driver --submission-dir submitDir-${NAME} ${ARGN}
         POST_EXEC_SCRIPT nopost.sh
         PROPERTIES TIMEOUT 900 )
      
   endfunction()

   function( add_test_compare NAME DATA_TYPE NAME1 NAME2 )

      atlas_add_test( ${NAME}
         SCRIPT compareFlatTrees --require-same-branches analysis submitDir-${NAME1}/data-ANALYSIS/${DATA_TYPE}.root submitDir-${NAME2}/data-ANALYSIS/${DATA_TYPE}.root
         POST_EXEC_SCRIPT nopost.sh
         PROPERTIES TIMEOUT 900
         DEPENDS ${NAME1} ${NAME2})

   endfunction()

else()

   atlas_install_joboptions( share/*_jobOptions.py )

   atlas_install_scripts( share/*_CA.py )

   function( add_test_job NAME DATA_TYPE )

      atlas_add_test( ${NAME}
         SCRIPT athena.py AnalysisAlgorithmsConfig/FullCPAlgorithmsTest_jobOptions.py --evtMax=500 - --force-output TestJobOutput-${NAME}.hist.root --data-type ${DATA_TYPE} ${ARGN}
         POST_EXEC_SCRIPT nopost.sh
         PROPERTIES TIMEOUT 900 )

   endfunction()

   function( add_test_job_CA NAME DATA_TYPE )

      atlas_add_test( ${NAME}_CA
         SCRIPT FullCPAlgorithmsTest_CA.py --evtMax=500 --force-output TestJobOutput-${NAME}-CA.hist.root --data-type ${DATA_TYPE} ${ARGN}
         POST_EXEC_SCRIPT nopost.sh
         PROPERTIES TIMEOUT 900 )
   
   endfunction()

   function( add_test_compare NAME DATA_TYPE NAME1 NAME2 )

      atlas_add_test( ${NAME}
         SCRIPT acmd.py diff-root --error-mode resilient -t analysis TestJobOutput-${NAME1}.hist.root TestJobOutput-${NAME2}.hist.root
         POST_EXEC_SCRIPT nopost.sh
         PROPERTIES TIMEOUT 900
         DEPENDS ${NAME1} ${NAME2})

   endfunction()

   add_test_job_CA( TestJobDataConfig   data --for-compare --block-config --no-systematics )
   add_test_job_CA( TestJobFullSimConfig   mc --for-compare --block-config )
   add_test_job_CA( TestJobFastSimConfig   afii --for-compare --block-config )

endif()



add_test_job( TestJobDataSequence data --for-compare --no-systematics )
add_test_job( TestJobDataConfig   data --for-compare --block-config --no-systematics )
add_test_job( TestJobDataFull     data --block-config --no-systematics )
add_test_compare( TestJobDataCompare data TestJobDataSequence TestJobDataConfig )

add_test_job( TestJobFullSimSequence mc --for-compare )
add_test_job( TestJobFullSimConfig   mc --for-compare --block-config )
add_test_job( TestJobFullSimFull     mc --block-config )
add_test_compare( TestJobFullSimCompare mc TestJobFullSimSequence TestJobFullSimConfig )

add_test_job( TestJobFastSimSequence afii --for-compare )
add_test_job( TestJobFastSimConfig   afii --for-compare --block-config )
add_test_job( TestJobFastSimFull     afii --block-config )
add_test_compare( TestJobFastSimCompare afii TestJobFastSimSequence TestJobFastSimConfig )




# FIX ME: There are some issues with the PHYSLITE tests that need to
# be investigated, but until that happens I run some tests only in
# AnalysisBase, and disable comparisons alltogether.

add_test_job( TestJobDataSequenceLite data --for-compare --physlite --no-physlite-broken --no-systematics )
add_test_job( TestJobDataConfigLite   data --for-compare --block-config --physlite --no-physlite-broken --no-systematics )
add_test_job( TestJobDataFullLite     data --block-config --physlite --no-physlite-broken --no-systematics )
#add_test_compare( TestJobDataCompareLite data TestJobDataSequenceLite TestJobDataConfigLite )

if( XAOD_STANDALONE )

add_test_job( TestJobFullSimSequenceLite mc --for-compare --physlite --no-physlite-broken )
add_test_job( TestJobFullSimConfigLite   mc --for-compare --block-config --physlite --no-physlite-broken )
add_test_job( TestJobFullSimFullLite     mc --block-config --physlite --no-physlite-broken )
add_test_compare( TestJobFullSimCompareLite mc TestJobFullSimSequenceLite TestJobFullSimConfigLite )

add_test_job( TestJobFastSimSequenceLite afii --for-compare --physlite --no-physlite-broken )
add_test_job( TestJobFastSimConfigLite   afii --for-compare --block-config --physlite --no-physlite-broken )
add_test_job( TestJobFastSimFullLite     afii --block-config --physlite --no-physlite-broken )
add_test_compare( TestJobFastSimCompareLite afii TestJobFastSimSequenceLite TestJobFastSimConfigLite )

endif()
