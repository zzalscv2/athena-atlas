################################################################################
# Package: MuonCombinedConfig
################################################################################

# Declare the package name:
atlas_subdir( MuonCombinedConfig )

# Install files from the package:
atlas_install_python_modules( python/*.py  )

atlas_add_test( MuonCombinedRecoConfigTest
SCRIPT python -m MuonCombinedConfig.MuonCombinedReconstructionConfig --threads=1
PROPERTIES TIMEOUT 900
POST_EXEC_SCRIPT noerror.sh )
