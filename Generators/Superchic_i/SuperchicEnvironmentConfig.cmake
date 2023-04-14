# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# This module is used to set up the environment for Superchic
# 
#

# Set the environment variable(s):
find_package( Superchic )

if( SUPERCHIC_FOUND )
  set( SUPERCHICENVIRONMENT_ENVIRONMENT 
        FORCESET SUPERCHICVER ${SUPERCHIC_LCGVERSION}
        FORCESET SUPERCHICPATH ${SUPERCHIC_LCGROOT} 
        APPEND LHAPATH
              /cvmfs/atlas.cern.ch/repo/sw/Generators/lhapdfsets/current
        APPEND LHAPDF_DATA_PATH
              /cvmfs/atlas.cern.ch/repo/sw/Generators/lhapdfsets/current )
endif()


# Silently declare the module found:
set( SUPERCHICENVIRONMENT_FOUND TRUE )


