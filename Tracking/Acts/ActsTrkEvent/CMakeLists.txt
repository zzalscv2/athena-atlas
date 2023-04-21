# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsTrkEvent )

# External dependencies:
find_package( Acts COMPONENTS Core )
find_package( Boost )
find_package( Eigen )

atlas_add_library( ActsTrkEventLib
                   Root/*.cxx
                   PUBLIC_HEADERS ActsTrkEvent
                   INCLUDE_DIRS ${Acts_INCLUDE_DIR}
                   LINK_LIBRARIES ${Boost_LIBRARIES} ${EIGEN_LIBRARIES} 
		   AthenaBaseComps GaudiKernel AtlasDetDescr CxxUtils 
		   xAODCore ActsCore ActsFatras ActsGeometryLib BeamSpotConditionsData
		   MagFieldConditions MagFieldElements GeoPrimitives xAODTracking xAODInDetMeasurement
		   )

atlas_add_dictionary( ActsTrkEventDict
		      ActsTrkEvent/ActsTrkEventDict.h
		      ActsTrkEvent/selection.xml
		      LINK_LIBRARIES xAODCore ActsTrkEventLib
		      DATA_LINKS 
		      ActsTrk::SeedContainer 
		      ActsTrk::BoundTrackParametersContainer
		      )

atlas_add_test( MultiTrajectoryBasic_test 
				SOURCES test/MultiTrajectoryBasic_test.cxx
				LINK_LIBRARIES TestTools ActsTrkEventLib
				POST_EXEC_SCRIPT nopost.sh)
