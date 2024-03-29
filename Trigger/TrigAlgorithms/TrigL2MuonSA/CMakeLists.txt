# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigL2MuonSA )

# External dependencies:
find_package( CLHEP )
find_package( GSL )
find_package( ROOT COMPONENTS Core MathCore )
find_package( tdaq-common COMPONENTS DataReader EventStorage eformat ers MuCalDecode circ_proc)

# Component(s) in the package:
atlas_add_component( TrigL2MuonSA
		             src/*.cxx
                     src/components/*.cxx
		             INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${GSL_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${TDAQ-COMMON_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} ${GSL_LIBRARIES} ${ROOT_LIBRARIES} ${TDAQ-COMMON_LIBRARIES}  AthenaBaseComps AthenaInterprocess RPC_CondCablingLib AthenaMonitoringKernelLib CxxUtils GaudiKernel GeoModelUtilities GeoPrimitives IRegionSelector Identifier MdtCalibSvcLib MuonCablingData MuonCalibEvent MuonCnvToolInterfacesLib MuonIdHelpersLib MuonPrepRawData MuonRDO MuonReadoutGeometry MuonRecToolInterfaces PathResolver StoreGateLib TrigMuonToolInterfaces TrigSteeringEvent TrigT1Interfaces TrkExInterfaces xAODEventInfo xAODTracking xAODTrigMuon xAODTrigger )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_runtime( share/*.lut)

# Tests:
atlas_add_test( MdtDataPreparator_test
                SCRIPT python -m TrigL2MuonSA.MdtDataPreparator_test
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( TrigL2MuonSAConfig
                SCRIPT python -m TrigL2MuonSA.TrigL2MuonSAConfig
                POST_EXEC_SCRIPT noerror.sh )
