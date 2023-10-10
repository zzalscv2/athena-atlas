################################################################################
# Package: MuonSensitiveDetectors
################################################################################

# Declare the package name:
atlas_subdir( MuonSimHitSorting )


atlas_add_library( MuonSimHitSortingLib
                   src/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS MuonSimHitSorting
                   LINK_LIBRARIES GaudiKernel StoreGateLib xAODMuonSimHit MuonIdHelpersLib)

# Component(s) in the package:
atlas_add_component( MuonSimHitSorting
                     src/components/*.cxx                  
                     PRIVATE_LINK_LIBRARIES MuonSimHitSortingLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

