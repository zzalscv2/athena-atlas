## automatically generated CMakeLists.txt file

# Declare the package
atlas_subdir( TrackHitAssignement )

# Declare external dependencies ... default here is to include ROOT
find_package( ROOT COMPONENTS MathCore RIO Core Tree Hist )

# Declare package as a library
atlas_add_library( TrackHitAssignementLib src/*.cxx
                   src/*.h
                   PUBLIC_HEADERS TrackHitAssignement
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES}
                                    AthLinks
                                    AthAnalysisBaseCompsLib
                                    xAODTracking
                                    TrkRIO_OnTrack
                                    TrkTrack
                                    TrkPrepRawData
)

atlas_add_component( TrackHitAssignement src/components/*.cxx
                      src/*.cxx
                      NOCLIDDB
                      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                      LINK_LIBRARIES TrackHitAssignementLib 
)

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
