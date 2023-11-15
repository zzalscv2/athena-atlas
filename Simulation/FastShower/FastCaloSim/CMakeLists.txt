# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FastCaloSim )

# External dependencies:
find_package( CLHEP )
find_package( Eigen )
find_package( HepPDT )
find_package( ROOT COMPONENTS Gpad Cint Core Tree MathCore Hist RIO pthread MathMore Minuit Minuit2 Matrix Physics HistPainter Rint Graf Graf3d Html Postscript Gui GX11TTF GX11 )


# Component(s) in the package:
atlas_add_library( FastCaloSimLib
                   src/CaloCellContainerFCSFinalizerTool.cxx
                   src/EmptyCellBuilderTool.cxx
                   PUBLIC_HEADERS FastCaloSim
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${HEPPDT_INCLUDE_DIRS}
                   PRIVATE_INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}  ${EIGEN_INCLUDE_DIRS}
                   PRIVATE_DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} CaloIdentifier CaloInterfaceLib Identifier
                   PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES}  ${EIGEN_LIBRARIES} ${HEPPDT_LIBRARIES} AthAllocators AthenaBaseComps AthenaKernel CaloEvent GaudiKernel TileEvent CaloDetDescrLib StoreGateLib )

atlas_add_component( FastCaloSim
                     src/components/*.cxx
                     LINK_LIBRARIES FastCaloSimLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
