# $Id: CMakeLists.txt 770003 2016-08-25 08:54:14Z krasznaa $
################################################################################
# Package: IsolationTool
################################################################################

# Declare the package name:
atlas_subdir( IsolationTool )

# Extra dependencies based on the build environment:
set( extra_lib )
set( extra_lib_private )
if( NOT XAOD_STANDALONE )
  set( extra_lib TrkParametersIdentificationHelpers CaloUtilsLib ParticlesInConeToolsLib)
  set( extra_lib_private CaloEvent CaloUtilsLib TrkCaloExtension TrkParameters TrackToCaloLib )
endif()

# Component(s) in the package:
atlas_add_library( IsolationToolLib
   IsolationTool/*.h Root/*.cxx
   PUBLIC_HEADERS IsolationTool
   LINK_LIBRARIES AsgTools xAODBase xAODCaloEvent xAODCore xAODEgamma
   xAODEventShape xAODMuon xAODPFlow xAODPrimitives xAODTracking
   AsgDataHandlesLib InDetTrackSelectionToolLib
   IsolationCorrectionsLib RecoToolInterfaces
   TrackVertexAssociationToolLib
   ${extra_lib}
   PRIVATE_LINK_LIBRARIES CaloGeoHelpers FourMomUtils PathResolver
   ${extra_lib_private}
   )

atlas_add_dictionary( IsolationToolDict
    IsolationTool/IsolationToolDict.h IsolationTool/selection.xml
    LINK_LIBRARIES IsolationToolLib )


if( NOT XAOD_STANDALONE )
  atlas_add_component( IsolationTool
    src/*.h src/*.cxx src/components/*.cxx
    LINK_LIBRARIES AthenaBaseComps GaudiKernel IsolationToolLib )
endif()
