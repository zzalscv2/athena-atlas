# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# This module is used to set up the runtime environment for Contur.
# Set the environment variable(s):

find_package( contur )
find_package( Rivet )

set( conturEnvironment_ENVIRONMENT 
   FORCESET RIVETVER ${RIVET_LCGVERSION} 
   FORCESET CONTURVER ${CONTUR_LCGVERSION}
   FORCESET CONTUR_PATH ${CONTUR_LCGROOT} )

# Silently declare the module found.
if( CONTUR_FOUND AND RIVET_FOUND)
    set( conturEnvironment_FOUND TRUE )
else()
    set( conturEnvironment_FOUND FALSE )
endif()

