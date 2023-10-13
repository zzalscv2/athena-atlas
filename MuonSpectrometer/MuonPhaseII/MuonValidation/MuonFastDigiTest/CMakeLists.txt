################################################################################
# Package: MuonFastDigiTest
################################################################################

# Declare the package name:
atlas_subdir( MuonFastDigiTest )

atlas_add_component( MuonFastDigiTest
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES AthenaKernel StoreGateLib MuonTesterTreeLib
                                    xAODMuonSimHit xAODMuonPrepData  )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
