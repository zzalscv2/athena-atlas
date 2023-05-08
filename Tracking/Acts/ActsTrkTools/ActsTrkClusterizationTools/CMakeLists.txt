# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

atlas_subdir(ActsTrkClusterizationTools)

find_package(Acts COMPONENTS Core)
find_package(Eigen)

atlas_add_component(ActsTrkClusterizationTools
                    src/*.cxx
                    src/components/*.cxx
		    INCLUDE_DIRS
		    ${EIGEN_INCLUDE_DIRS}
		    LINK_LIBRARIES
		    ${EIGEN_LIBRARIES}
		    ActsCore
		    ActsTrkToolInterfacesLib
		    InDetRawData
		    InDetReadoutGeometry
		    SiClusterizationToolLib
		    xAODInDetMeasurement)
