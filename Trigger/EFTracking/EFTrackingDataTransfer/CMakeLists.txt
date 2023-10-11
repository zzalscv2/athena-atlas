# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( EFTrackingDataTransfer )

# External dependencies:
atlas_add_test( TransferBenchmark_test 
				SOURCES test/TransferBenchmark_test.cxx
				LINK_LIBRARIES TestTools ActsEventLib
				POST_EXEC_SCRIPT nopost.sh)

