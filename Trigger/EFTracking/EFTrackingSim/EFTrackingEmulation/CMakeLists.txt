# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

atlas_subdir ( EFTrackingEmulation )

atlas_add_library (EFTrackingEmulationLib
  EFTrackingEmulation/*.h src/*.cxx src/FitFunctions/L1TT/* 
  NO_PUBLIC_HEADERS
  LINK_LIBRARIES AnaAlgorithmLib xAODTracking TrkEventTPCnv TrkEventPrimitives)


atlas_add_component (EFTrackingEmulation
        src/components/*.cxx
		    LINK_LIBRARIES EFTrackingEmulationLib )


atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-select=ATL900,ATL901 )

atlas_add_test( EFTrackingSmearingConfig
   SCRIPT python -m EFTrackingEmulation.EFTrackingSmearingConfig
   POST_EXEC_SCRIPT noerror.sh )
