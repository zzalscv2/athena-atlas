################################################################################
# Package: MuonCalibStreamCnv
################################################################################

# Declare the package name:
atlas_subdir( MuonCalibStreamCnv )

# External dependencies:
find_package( COOL COMPONENTS CoolKernel )
find_package( CORAL COMPONENTS CoralBase CoralKernel RelationalAccess )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( tdaq-common COMPONENTS MuCalDecode )

# Component(s) in the package:
# OLD: RPCcablingInterfaceLib
atlas_add_library( MuonCalibStreamCnvLib
                   src/*.cxx
                   PUBLIC_HEADERS MuonCalibStreamCnv 
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${CORAL_INCLUDE_DIRS} 
                   LINK_LIBRARIES ${ROOT_LIBRARIES} ${CORAL_LIBRARIES} ${COOL_LIBRARIES} ${TDAQ-COMMON_LIBRARIES} 
                   AthenaBaseComps AthenaKernel StoreGateLib SGtests GaudiKernel CSCcablingLib  MuonReadoutGeometry MuonIdHelpersLib 
                   MuonRDO MuonPrepRawData SGTools CoraCool EventInfo MdtCalibSvcLib MuonCalibEvent MuonCSC_CnvToolsLib MuonCnvToolInterfacesLib 
                   MuonCalibStreamCnvSvcLib RPC_CondCablingLib MuonCondData MuonTrigCoinData)

atlas_add_component( MuonCalibStreamCnv
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${CORAL_INCLUDE_DIRS} ${COOL_INCLUDE_DIRS} ${TDAQ-COMMON_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} ${CORAL_LIBRARIES} ${COOL_LIBRARIES} ${TDAQ-COMMON_LIBRARIES} 
                     AthenaBaseComps AthenaKernel StoreGateLib SGtests GaudiKernel CSCcablingLib  MuonReadoutGeometry MuonIdHelpersLib
                     MuonRDO MuonPrepRawData SGTools CoraCool EventInfo MdtCalibSvcLib MuonCalibEvent MuonCSC_CnvToolsLib MuonCnvToolInterfacesLib 
                     MuonCalibStreamCnvSvcLib RPC_CondCablingLib MuonCondData MuonTrigCoinData)

# atlas_install_joboptions( share/*.py )
# atlas_install_runtime( share/calibtest.dat )
# Install files from the package:
atlas_install_python_modules( python/*.py )


