# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( StripGeoModelXml )

find_package( XercesC )
find_package( GeoModel COMPONENTS GeoModelKernel GeoModelXml ExpressionEvaluator )

# Component(s) in the package:
atlas_add_component( StripGeoModelXml
                     src/*.cxx
                     src/components/*.cxx
                     PRIVATE_INCLUDE_DIRS ${XERCESC_INCLUDE_DIRS} ${GEOMODEL_INCLUDE_DIRS}
                     PRIVATE_LINK_LIBRARIES ${XERCESC_LIBRARIES} ${GEOMODEL_LIBRARIES} CxxUtils
                     GaudiKernel DetDescrConditions InDetGeoModelUtils InDetSimEvent SCT_ReadoutGeometry GeometryDBSvcLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Test(s) in the package:
atlas_add_test( ITkStripGMConfig_test
                SCRIPT test/ITkStripGMConfig_test.py
                POST_EXEC_SCRIPT nopost.sh )
