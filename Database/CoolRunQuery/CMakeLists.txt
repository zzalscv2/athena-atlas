#
# CMake description for the package.
#

# Declare the package's name:
atlas_subdir( CoolRunQuery )

# Necessary external(s):
find_package( cx_Oracle )

# Declare the python modules of the package:
atlas_install_python_modules( python/*.py python/html python/selector
   python/utils python/output 
   POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-ignore ATL9 ) # ARQ is not using Athena logging


# Declare the files from the package's html/ subdirectory:
atlas_install_xmls( html/*.html html/*.dtd )
atlas_install_docs( html/*.html html/*.dtd html/*.css )

# Declare the scripts of the package:
atlas_install_scripts( share/*.py )
