# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LArMonitoring )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread Graf Graf3d Gpad Html Postscript Gui GX11TTF GX11 )

atlas_add_library( LArStrHelper
                   src/StrHelper/LArStrHelper.cxx LArMonitoring/LArStrHelper.h
                   PUBLIC_HEADERS LArMonitoring
                   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${CLHEP_LIBRARIES} ${ROOT_LIBRARIES})


# Component(s) in the package:
atlas_add_component( LArMonitoring
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} ${CLHEP_LIBRARIES} CaloDetDescrLib CaloGeoHelpers CaloIdentifier AthenaMonitoringLib  Identifier LArIdentifier LArRawEvent LArRecConditions LArCablingLib TrigDecisionToolLib LumiBlockCompsLib CaloConditions AthenaKernel AthenaPoolUtilities xAODEventInfo LArCOOLConditions LArRawConditions LArRecEvent LArStrHelper )

atlas_add_dictionary( LArStrHelperDict
                      LArMonitoring/LArStrHelperDict.h
                      LArMonitoring/selection.xml
                      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS} 
                      LINK_LIBRARIES ${ROOT_LIBRARIES} ${CLHEP_LIBRARIES} LArStrHelper)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/LArMonitoring_jobOption.py share/LArAffectedRegionsAlg_jobOptions.py share/LArCollisionTimeMonAlg_jobOptions.py )
atlas_install_scripts( share/StateLessPT_TestFile_NewConfig.py share/StateLessPT_NewConfig.py )
