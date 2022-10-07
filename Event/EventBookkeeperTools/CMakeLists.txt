# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( EventBookkeeperTools )

if( XAOD_STANDALONE )
   set( extra_libs xAODMetaData )
   set( xaod_access_lib xAODRootAccess )
# ... for AthAnalysisBase (Athena calls this POOLRootAccess)
else()
   set( extra_libs GaudiKernel AthenaKernel AthenaPoolUtilities EventInfo IOVDbDataModel StoreGateLib xAODMetaData )
   set( xaod_access_lib xAODRootAccess POOLRootAccessLib )
endif()

if( NOT GENERATIONBASE )
   set( extra_libs ${extra_libs} AnaAlgorithmLib xAODTruth )
endif()

# Component(s) in the package:
atlas_add_library( EventBookkeeperToolsLib
                   EventBookkeeperTools/*.h Root/*.cxx
                   PUBLIC_HEADERS EventBookkeeperTools
                   LINK_LIBRARIES ${extra_libs} AsgDataHandlesLib AsgMessagingLib AsgTools
                                  xAODCutFlow xAODEventInfo )

atlas_add_dictionary( EventBookkeeperToolsDict
                      EventBookkeeperTools/EventBookkeeperToolsDict.h
                      EventBookkeeperTools/selection.xml
                      LINK_LIBRARIES EventBookkeeperToolsLib )

if( NOT XAOD_STANDALONE )
   atlas_add_component( EventBookkeeperTools
                        src/*.cxx
                        src/components/*.cxx
                        LINK_LIBRARIES GaudiKernel AthenaKernel EventBookkeeperToolsLib)
endif()

atlas_add_executable( dump-cbk
                      util/dump-cbk.cxx
                      LINK_LIBRARIES ${xaod_access_lib} AsgTools )  

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/*.py )

# Tests
atlas_add_test( DumpCbkTest
                SCRIPT dump-cbk $ASG_TEST_FILE_MC
                POST_EXEC_SCRIPT noerror.sh )

if( NOT XAOD_STANDALONE )
   atlas_add_test( BookkeeperDumperTool
                   SCRIPT test/test_BookkeeperDumperTool.py
                   PRIVATE_WORKING_DIRECTORY
                   POST_EXEC_SCRIPT noerror.sh )

   if( NOT GENERATIONBASE )
      atlas_add_test( CutFlowSvcTestSerial
                      SCRIPT test/test_CutFlowSvc.py
                      PRIVATE_WORKING_DIRECTORY
                      POST_EXEC_SCRIPT noerror.sh )

      atlas_add_test( CutFlowSvcTestMT
                      SCRIPT test/test_CutFlowSvc.py -t 2
                      PRIVATE_WORKING_DIRECTORY
                      POST_EXEC_SCRIPT noerror.sh )

      if( NOT XAOD_ANALYSIS )
         atlas_add_test( CutFlowSvcTestMPStandardIO
                        SCRIPT test/test_CutFlowSvc.py -p 2
                        PRIVATE_WORKING_DIRECTORY
                        POST_EXEC_SCRIPT noerror.sh )

         atlas_add_test( CutFlowSvcTestMPSharedWriter
                        SCRIPT test/test_CutFlowSvc.py -p 2 --sharedWriter
                        PRIVATE_WORKING_DIRECTORY
                        POST_EXEC_SCRIPT noerror.sh )

         atlas_add_test( CutFlowSvcTestData
                        SCRIPT test/test_CutFlowSvc.py -t 2 --data
                        PRIVATE_WORKING_DIRECTORY
                        POST_EXEC_SCRIPT noerror.sh )
      endif()
   endif()

   atlas_add_test( CutFlowSvcDummyAlg
                   SCRIPT athena EventBookkeeperTools/TestCutFlowSvcDummyAlg.py
                   PRIVATE_WORKING_DIRECTORY
                   POST_EXEC_SCRIPT noerror.sh )

   atlas_add_test( CutFlowSvcOutput
                   SCRIPT athena EventBookkeeperTools/TestCutFlowSvcOutput.py
                   PRIVATE_WORKING_DIRECTORY
                   POST_EXEC_SCRIPT noerror.sh )

   atlas_add_test( CutFlowSvcOutputAlt
                   SCRIPT athena EventBookkeeperTools/TestCutFlowSvcOutput.py -c alternativeInput=True
                   PRIVATE_WORKING_DIRECTORY
                   POST_EXEC_SCRIPT noerror.sh )

   atlas_add_test( CutFlowSvcOutputMT
                   SCRIPT athena EventBookkeeperTools/TestCutFlowSvcOutput.py --threads=2
                   PROPERTIES PROCESSORS 2
                   PRIVATE_WORKING_DIRECTORY
                   POST_EXEC_SCRIPT noerror.sh )

   if( NOT GENERATIONBASE AND NOT XAOD_ANALYSIS )
      atlas_add_test( CutFlowSvcOutputMPStandardIO
                      SCRIPT athena EventBookkeeperTools/TestCutFlowSvcOutput.py --nprocs=2 -c sharedWriter=False
                      PROPERTIES PROCESSORS 2
                      PRIVATE_WORKING_DIRECTORY
                      POST_EXEC_SCRIPT noerror.sh )

      atlas_add_test( CutFlowSvcOutputMPSharedWriter
                      SCRIPT athena EventBookkeeperTools/TestCutFlowSvcOutput.py --nprocs=2 -c sharedWriter=True
                      PROPERTIES PROCESSORS 2
                      PRIVATE_WORKING_DIRECTORY
                      POST_EXEC_SCRIPT nopost.sh )

      atlas_add_test( CutFlowSvcOutputMPSharedWriterAlt
                      SCRIPT athena EventBookkeeperTools/TestCutFlowSvcOutput.py --nprocs=2 -c sharedWriter=True -c alternativeInput=True
                      PROPERTIES PROCESSORS 2
                      PRIVATE_WORKING_DIRECTORY
                      POST_EXEC_SCRIPT nopost.sh )
   endif()
endif()
