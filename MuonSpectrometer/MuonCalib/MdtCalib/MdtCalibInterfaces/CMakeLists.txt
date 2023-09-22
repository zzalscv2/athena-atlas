################################################################################
# Package: MdtCalibInterfaces
################################################################################

# Declare the package name:
atlas_subdir( MdtCalibInterfaces )

atlas_add_library( MdtCalibInterfacesLib
                   src/*.cxx
                   PUBLIC_HEADERS MdtCalibInterfaces
                   LINK_LIBRARIES AthenaKernel GaudiKernel GeoPrimitives
                                  MuonDigitContainer MuonReadoutGeometry MuonPrepRawData MuonReadoutGeometryR4
                   )

