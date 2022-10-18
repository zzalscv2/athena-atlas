################################################################################
# Package: ISF_PunchThroughTools
################################################################################

# Declare the package name:
atlas_subdir( ISF_PunchThroughTools )

# External dependencies:
find_package( CLHEP )
find_package( Eigen )
find_package( HepPDT )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( LibXml2 REQUIRED)


# Component(s) in the package:
atlas_add_component( ISF_PunchThroughTools
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${HEPPDT_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}  ${EIGEN_INCLUDE_DIRS} ${LWTNN_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIR}
                     LINK_LIBRARIES ${HEPPDT_LIBRARIES} ${ROOT_LIBRARIES} ${CLHEP_LIBRARIES} AtlasHepMCLib ${EIGEN_LIBRARIES} ${LWTNN_LIBRARIES} ${LIBXML2_LIBRARIES} AthenaBaseComps AthenaKernel BarcodeInterfacesLib GeoPrimitives ISF_FastCaloSimInterfaces ISF_FastCaloSimEvent AthContainers GaudiKernel ISF_Event ISF_InterfacesLib PathResolver SubDetectorEnvelopesLib )
