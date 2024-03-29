# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( RPC_Digitization )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core )

# Component(s) in the package:
atlas_add_component( RPC_Digitization
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} ${ROOT_LIBRARIES} AtlasHepMCLib AthenaBaseComps PileUpToolsLib xAODEventInfo GaudiKernel MuonSimData MuonSimEvent HitManagement AthenaKernel GeneratorObjects MuonCondData MuonReadoutGeometry MuonDigitContainer MuonIdHelpersLib PathResolver GeoModelInterfaces GeometryDBSvcLib EventInfoMgtLib RDBAccessSvcLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/RPC_Digitization_jobOptions.txt share/*.py )
atlas_install_runtime( share/G4RPC_Digitizer.txt share/PermanentDeadPanels.txt share/PermanentGoodPanels.txt )

