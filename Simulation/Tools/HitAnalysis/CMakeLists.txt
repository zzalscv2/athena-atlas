# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: HitAnalysis
################################################################################

# Declare the package name:
atlas_subdir( HitAnalysis )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread Table MathMore Minuit Minuit2 Matrix Physics HistPainter Rint Graf Graf3d Gpad Html Postscript Gui GX11TTF GX11 )

# tag ROOTBasicLibs was not recognized in automatic conversion in cmt2cmake

# tag ROOTSTLDictLibs was not recognized in automatic conversion in cmt2cmake

# Component(s) in the package:
atlas_add_component( HitAnalysis
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} ${CLHEP_LIBRARIES} AtlasHepMCLib TruthUtils GaudiKernel CaloDetDescrLib CaloIdentifier CaloSimEvent AthenaBaseComps GeoAdaptors AFP_SimEv ALFA_SimEv LUCID_SimEvent ZDC_SimEvent ZdcIdentifier GeneratorObjects InDetSimEvent LArSimEvent MuonSimEvent TileDetDescr TileSimEvent TrackRecordLib )

# Install files from the package:
atlas_install_joboptions( share/*.py )
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_runtime( tools/RunHitAnalysis.py )
