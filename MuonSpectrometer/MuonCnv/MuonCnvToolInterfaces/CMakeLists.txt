################################################################################
# Package: MuonCnvToolInterfaces
################################################################################

# Declare the package name:
atlas_subdir( MuonCnvToolInterfaces )

# External dependencies:
find_package( tdaq-common )
find_package( ROOT )

atlas_add_library( MuonCnvToolInterfacesLib
                   MuonCnvToolInterfaces/*.h
                   INTERFACE
                   PUBLIC_HEADERS MuonCnvToolInterfaces
                   INCLUDE_DIRS ${TDAQ-COMMON_INCLUDE_DIRS}
                   LINK_LIBRARIES GaudiKernel ByteStreamCnvSvcBaseLib Identifier AthContainers AthenaKernel ByteStreamData ${TDAQ-COMMON_LIBRARIES} )


atlas_add_dictionary( MuonCnvToolInterfacesDict
                      MuonCnvToolInterfaces/MuonCnvToolInterfacesDict.h
                      MuonCnvToolInterfaces/selection.xml
                      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                      LINK_LIBRARIES MuonCnvToolInterfacesLib ${ROOT_LIBRARIES})

