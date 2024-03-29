# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonCondDump )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( CORAL )

# Component(s) in the package:
atlas_add_component( MuonCondDump
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CORAL_LIBRARIES} ${ROOT_LIBRARIES} AthenaBaseComps AthenaKernel CoralUtilitiesLib GaudiKernel 
                                    GeoPrimitives Identifier MuonAlignmentData MuonCondData MuonCondInterface MuonCondSvcLib MuonIdHelpersLib 
                                    MuonReadoutGeometry ActsGeoUtils StoreGateLib MuonCablingData)

# Install files from the package:
atlas_install_python_modules( python/*.py )

atlas_add_test( TestABLineDump
                  SCRIPT  python -m MuonCondDump.DumpABLines && python -m MuonCondDump.DumpABLines --alignJsonFile ForkLiftTruckDrivingIsFun.json
                  PROPERTIES TIMEOUT 600
                  POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( TestMdtCablingDump
                SCRIPT python -m MuonCondDump.MdtMezzExtraction && python -m MuonCondTest.MdtCablingTester --output Cabling_json.log --mezzMap MezzMapping.json --cablingMap MdtCabling.json  && 
                       python -m MuonCondTest.MdtCablingTester --output Cabling_cool.log  && diff -q Cabling_cool.log Cabling_json.log 
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT nopost.sh) 
