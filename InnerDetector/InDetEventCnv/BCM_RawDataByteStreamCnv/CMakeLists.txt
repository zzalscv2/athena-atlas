# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( BCM_RawDataByteStreamCnv )

# External dependencies:
find_package( tdaq-common COMPONENTS eformat )

# Component(s) in the package:
atlas_add_component( BCM_RawDataByteStreamCnv
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${TDAQ-COMMON_INCLUDE_DIRS}
                     LINK_LIBRARIES ${TDAQ-COMMON_LIBRARIES} AthenaBaseComps AthenaKernel ByteStreamCnvSvcBaseLib ByteStreamCnvSvcLib ByteStreamData GaudiKernel InDetBCM_RawData StoreGateLib )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
