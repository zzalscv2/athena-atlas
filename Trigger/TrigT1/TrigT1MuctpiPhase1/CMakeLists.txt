# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigT1MuctpiPhase1 )

# External dependencies:
find_package( Boost )

# Component(s) in the package:
atlas_add_library( TrigT1MuctpiL1TopoLib
		   src/L1TopoLUT.cxx
                   PUBLIC_HEADERS TrigT1MuctpiPhase1
		   INCLUDE_DIRS ${Boost_INCLUDE_DIRS} )

atlas_add_component( TrigT1MuctpiPhase1
                     src/*.cxx src/components/*.cxx
                     INCLUDE_DIRS ${Boost_INCLUDE_DIRS} 
                     LINK_LIBRARIES ${Boost_LIBRARIES} AnalysisTriggerEvent AthenaBaseComps GaudiKernel HLTSeedingLib PathResolver StoreGateLib TrigConfData 
		     		    TrigConfMuctpi TrigT1Interfaces TrigT1Result xAODTrigger TrigT1MuctpiL1TopoLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
