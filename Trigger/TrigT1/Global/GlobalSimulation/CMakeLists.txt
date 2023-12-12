# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( GlobalSimulation )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist)

atlas_add_component (GlobalSimulation
   src/*.h src/*.cxx src/components/*.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} GaudiKernel L1TopoSimulationLib
   TrigConfData L1TopoEvent L1TopoInterfaces L1TopoCoreSim L1TopoHardware
   AthenaBaseComps)


# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
