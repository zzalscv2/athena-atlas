################################################################################
# Package: TrfTestsARTPlots
################################################################################

# Declare the package name:
atlas_subdir( TrfTestsARTPlots )

# Install files from the package:
atlas_install_runtime( test/*.sh )
atlas_install_scripts( test/*.sh test/*.py )
