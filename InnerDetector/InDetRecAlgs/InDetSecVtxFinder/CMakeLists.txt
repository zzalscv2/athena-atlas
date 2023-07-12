# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetSecVtxFinder )


# Component(s) in the package:
atlas_add_component( InDetSecVtxFinder
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps xAODTracking GaudiKernel InDetRecToolInterfaces TrkTrack 
                     TrkParticleBase TrkVertexFitterInterfaces VxSecVertex)

atlas_install_joboptions( share/*.py POST_BUILD_CMD ${ATLAS_FLAKE8})
