# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PersistencySvc )

# External dependencies:
find_package( CORAL COMPONENTS CoralBase )
find_package( ROOT COMPONENTS Core )

# Component(s) in the package:
atlas_add_library( PersistencySvc
   PersistencySvc/*.h PersistencySvc/*.inl
   src/*.cpp src/*.h src/*.inl
   PUBLIC_HEADERS PersistencySvc
   PRIVATE_INCLUDE_DIRS ${CORAL_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} GaudiUtils FileCatalog
   LINK_LIBRARIES CxxUtils DataModelRoot POOLCore StorageSvc
   PRIVATE_LINK_LIBRARIES ${CORAL_LIBRARIES} ${ROOT_LIBRARIES} AthenaKernel FileCatalog
   PersistentDataModel )

atlas_add_dictionary( PersistencySvcDict PersistencySvc/dict.h PersistencySvc/dict.xml
   LINK_LIBRARIES PersistencySvc )
   
atlas_add_executable( pool_extractFileIdentifier
   utilities/pool_extractFileIdentifier.cpp
   LINK_LIBRARIES CxxUtils PersistencySvc )

atlas_add_executable( pool_insertFileToCatalog
   utilities/pool_insertFileToCatalog.cpp
   LINK_LIBRARIES CxxUtils FileCatalog POOLCore PersistencySvc )
