# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonHitCsvDump )


# Component(s) in the package:
atlas_add_component( MuonHitCsvDump
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES  AthenaBaseComps GeoPrimitives MuonIdHelpersLib  
                                     MuonSimEvent xAODMuonSimHit StoreGateLib MdtCalibSvcLib
                                     xAODMuonPrepData  MuonStationGeoHelpersLib)
                                     
atlas_install_python_modules( python/*.py)

atlas_add_test( testCsvDumper
                SCRIPT python -m MuonGeoModelTestR4.testSensitiveDetectors --nEvents 10  && python -m MuonHitCsvDump.csvHitDump -i myMuonSimTestStream.pool.root
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT nopost.sh)


