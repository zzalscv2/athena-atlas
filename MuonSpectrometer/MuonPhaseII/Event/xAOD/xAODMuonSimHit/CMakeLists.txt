# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODMuonSimHit )

# External dependencies:
find_package( xAODUtilities )

atlas_add_library( xAODMuonSimHit
    xAODMuonSimHit/*.h xAODMuonSimHit/versions/*.h Root/*.cxx
    PUBLIC_HEADERS xAODMuonSimHit
    LINK_LIBRARIES xAODCore GeoPrimitives MuonPrepRawData 
                   xAODMeasurementBase Identifier MuonReadoutGeometryR4)

  
atlas_add_xaod_smart_pointer_dicts(
    INPUT xAODMuonSimHit/selection.xml
    OUTPUT _selectionFile
    CONTAINERS "xAOD::MdtDriftCircleContainer_v1" )

atlas_add_dictionary( xAODMuonSimHitDict
    xAODMuonSimHit/xAODMuonSimHitDict.h
    ${_selectionFile}
    LINK_LIBRARIES xAODCore xAODMuonSimHit
    EXTRA_FILES Root/dict/*.cxx )

