################################################################################
# Package: MuonSensitiveDetectors
################################################################################

# Declare the package name:
atlas_subdir( MuonSensitiveDetectorsR4 )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )
find_package( XercesC )
find_package( GTest )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

