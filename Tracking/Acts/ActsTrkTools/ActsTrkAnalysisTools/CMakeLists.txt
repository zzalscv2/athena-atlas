# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS coll

atlas_subdir(ActsTrkAnalysisTools)

atlas_add_component(ActsTrkAnalysisTools
		    src/*.cxx
		    src/components/*.cxx
		    LINK_LIBRARIES
		    AthenaBaseComps AthenaKernel GaudiKernel
		    xAODInDetMeasurement InDetIdentifier
		    AthenaMonitoringLib TrkValHistUtils
		    )

