# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetAlignGenAlgs )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_component( InDetAlignGenAlgs
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} ${CLHEP_LIBRARIES} AthContainers AthenaBaseComps GeoPrimitives Identifier EventPrimitives xAODEventInfo GaudiKernel StoreGateLib TRT_ConditionsServicesLib TrkParameters TrkFitterUtils AthenaKernel AtlasDetDescr DetDescrConditions IdDictDetDescr SCT_ReadoutGeometry InDetIdentifier InDetReadoutGeometry TRT_ReadoutGeometry InDetPrepRawData InDetRIO_OnTrack TrkPseudoMeasurementOnTrack TrkRIO_OnTrack TrkTrack TrkTrackSummary TrkExInterfaces TrkFitterInterfaces TrkToolInterfaces InDetAlignGenToolsLib RegistrationServicesLib )

# Install files from the package:
atlas_install_joboptions( share/*.py )
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_runtime( share/CreateMisalignment*.py )
