# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ISF_FatrasTools )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_component( ISF_FatrasTools
                     src/*.cxx src/components/*.cxx
                     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} ${ROOT_LIBRARIES} TruthUtils AthenaBaseComps AthenaKernel AtlasDetDescr BarcodeEventLib CxxUtils EventPrimitives FastSimulationEventLib GaudiKernel GeoPrimitives ISF_Event ISF_FatrasInterfaces ISF_InterfacesLib StoreGateLib TrkDetDescrInterfaces TrkDetDescrUtils TrkEventPrimitives TrkExInterfaces TrkExUtils TrkGeometry TrkMaterialOnTrack TrkNeutralParameters TrkParameters TrkSurfaces TrkTrack TrkVolumes )

foreach( name
         EnergyLossSamplerBetheHeitler_test
         McEnergyLossUpdator_test
         MultipleScatteringSamplerGaussianMixture_test
         MultipleScatteringSamplerGeneralMixture_test
         MultipleScatteringSamplerHighland_test
         PhotonConversionTool_test
       )
 
    atlas_add_test( ${name}
                    SOURCES test/${name}.cxx
                    LINK_LIBRARIES AthenaKernel GaudiKernel ISF_FatrasInterfaces ISF_InterfacesLib TestTools TrkExInterfaces TrkGeometry TrkMaterialOnTrack
                    ENVIRONMENT "JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/share"
                    POST_EXEC_SCRIPT nopost.sh
                    PROPERTIES TIMEOUT 300 )
endforeach()
