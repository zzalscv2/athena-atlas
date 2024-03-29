# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TileByteStream )

# External dependencies:
find_package( tdaq-common COMPONENTS eformat )

# Component(s) in the package:
atlas_add_library( TileByteStreamLib
   TileByteStream/*.h TileByteStream/*.icc src/*.cxx
   PUBLIC_HEADERS TileByteStream
   INCLUDE_DIRS ${TDAQ-COMMON_INCLUDE_DIRS}
   LINK_LIBRARIES ${TDAQ-COMMON_LIBRARIES} AthenaBaseComps AthenaKernel
   ByteStreamData GaudiKernel TileEvent TileIdentifier ByteStreamCnvSvcBaseLib
   CaloDetDescrLib StoreGateLib ByteStreamCnvSvcLib TileConditionsLib
   TileL2AlgsLib TileRecUtilsLib CxxUtils
   PRIVATE_LINK_LIBRARIES CaloIdentifier TileCalibBlobObjs
   TileDetDescr )

atlas_add_component( TileByteStream
   src/components/*.cxx
   INCLUDE_DIRS ${TDAQ-COMMON_INCLUDE_DIRS}
   LINK_LIBRARIES TileByteStreamLib )


set( extra_patterns "LArDetectorToolNV|is still valid|no data retrieved|Database being retired|Reading file|Unable to locate catalog|Resolved path|DigitizationFlags|^Domain|created CondCont|no dictionary for class|^ +[+]|Reading LArPedestalMC|IOV callback|^DetectorStore|TileDetectorTool|Creating TileCondProxyFile|Cache alignment|INFO opening|loading cache|start processing|done processing|WARNING ../MuRcvDigitsCnt|WARNING ../MuRcvRawChCnt|TileL2Builder initialization completed|Initializing TileMuRcvContByteStreamTool|filled inputFileSummary from file|Picked valid file|No SimFlags|MetaReader|AutoConfiguration|INFO Initializing|IOVDbSvc[0-9 ]+INFO|Found [0-9]* algorithms" )

atlas_add_test( TileDigitsContByteStreamCnv_test
   SCRIPT test/TileDigitsContByteStreamCnv_test.sh
   PROPERTIES TIMEOUT 500
   LOG_IGNORE_PATTERN ${extra_patterns} )

atlas_add_test( TileRawChannelContByteStreamCnv_test
   SCRIPT test/TileRawChannelContByteStreamCnv_test.sh
   PROPERTIES TIMEOUT 500
   LOG_IGNORE_PATTERN ${extra_patterns} )

atlas_add_test( TileBeamElemContByteStreamCnv_test
   SCRIPT test/TileBeamElemContByteStreamCnv_test.sh
   PROPERTIES TIMEOUT 500
   LOG_IGNORE_PATTERN ${extra_patterns} )

atlas_add_test( TileL2ContByteStreamCnv_test
   SCRIPT test/TileL2ContByteStreamCnv_test.sh
   PROPERTIES TIMEOUT 500
   LOG_IGNORE_PATTERN ${extra_patterns} )

atlas_add_test( TileLaserObjByteStreamCnv_test
   SCRIPT test/TileLaserObjByteStreamCnv_test.sh
   PROPERTIES TIMEOUT 500
   LOG_IGNORE_PATTERN ${extra_patterns} )

atlas_add_test( TileMuRcvContByteStreamCnv_test
   SCRIPT test/TileMuRcvContByteStreamCnv_test.sh
   PROPERTIES TIMEOUT 500
   LOG_IGNORE_PATTERN ${extra_patterns} )


atlas_add_test( TileRawChannel2Bytes4_test
                SOURCES test/TileRawChannel2Bytes4_test.cxx
                LINK_LIBRARIES TileByteStreamLib )

atlas_add_test( TileROD_Decoder_test
                SOURCES test/TileROD_Decoder_test.cxx
                LINK_LIBRARIES TileByteStreamLib IdDictParser PathResolver TestTools
                ENVIRONMENT "JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/share" )


atlas_add_test( TileHid2RESrcIDConfig_test
                SCRIPT python -m TileByteStream.TileHid2RESrcIDConfig
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT nopost.sh)

# Install files from the package:
atlas_install_joboptions( share/*.py )
atlas_install_data( share/*.dump )
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
