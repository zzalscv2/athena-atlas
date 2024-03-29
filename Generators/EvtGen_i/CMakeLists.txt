# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( EvtGen_i )

# External dependencies:
find_package( CLHEP )
find_package( HepPDT )
if (HEPMC3_USE)
  find_package( Photospp COMPONENTS Photospp PhotosppHepMC3 )
  find_package( Tauolapp COMPONENTS TauolaHepMC3 )
else()
  find_package( Photospp COMPONENTS Photospp PhotosppHepMC )
  find_package( Tauolapp COMPONENTS TauolaHepMC )
endif()
#The earlier releases used EvtGen 1.7 and a lot of custom code
#The EvtGen 2.0+ is modern enough so the external libraries from EvtGen can be used as is.
find_package( EvtGen  COMPONENTS EvtGen EvtGenExternal )
find_package( Pythia8 )
find_package( ROOT COMPONENTS Core PyROOT Tree MathCore Hist RIO pthread )

# Remove the --as-needed linker flags:
atlas_disable_as_needed()

#Use only the codes absent in the modern external EvtGen libraries

# Previously  own interfaces/externals were used
# Keep the list as a documentation
set(EvtGen_iLib_SOURCES
                   src/EvtPythia.cxx
                   src/EvtTauola.cxx
                   src/EvtExternalGenFactory.cxx
                   src/EvtPHOTOS.cxx
                   src/EvtPythia6CommandConverter.cxx
                   src/EvtTauolaEngine.cxx
                   src/EvtExternalGenList.cxx
                   src/EvtPhotosEngine.cxx
                   src/EvtPythiaEngine.cxx
                   src/EvtInclusiveDecay.cxx
                   src/Belle2/EvtB0toKsKK.cc
                   src/Belle2/EvtBCL.cc
                   src/Belle2/EvtBCLFF.cc
                   src/Belle2/EvtBGL.cc
                   src/Belle2/EvtBGLFF.cc
                   src/Belle2/EvtBSemiTauonic.cc
                   src/Belle2/EvtBSemiTauonic2HDMType2.cc
      		   src/Belle2/EvtBSemiTauonicAmplitude.cc
		   src/Belle2/EvtBSemiTauonicHelicityAmplitudeCalculator.cc
		   src/Belle2/EvtBSemiTauonicScalarMesonAmplitude.cc
		   src/Belle2/EvtBSemiTauonicVectorMesonAmplitude.cc
		   src/Belle2/EvtEtaFullDalitz.cc
 		   src/Belle2/EvtEtaPi0Dalitz.cc
 		   src/Belle2/EvtEtaPrimeDalitz.cc
 		   src/Belle2/EvtHQET3.cc
		   src/Belle2/EvtHQET3FF.cc
 		   src/Belle2/EvtLLSW.cc
		   src/Belle2/EvtLLSWFF.cc
 		   src/Belle2/EvtPHSPBMix.cc
 		   src/Belle2/EvtYmSToYnSpipiCLEOboost.cc
)
#endif()
# Component(s) in the package:
if (HEPMC3_USE)
  atlas_add_library( EvtGen_iLib
                   ${EvtGen_iLib_SOURCES}
                   PUBLIC_HEADERS EvtGen_i
                   DEFINITIONS -DEVTGEN_HEPMC3 -DEVTGEN_PYTHIA -DEVTGEN_PHOTOS -DEVTGEN_TAUOLA
                   INCLUDE_DIRS ${HEPPDT_INCLUDE_DIRS} ${PYTHIA8_INCLUDE_DIRS} ${PHOTOSPP_INCLUDE_DIRS} ${TAUOLAPP_INCLUDE_DIRS}  ${EVTGEN_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                   PRIVATE_INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                   PRIVATE_DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${HEPPDT_LIBRARIES} ${PYTHIA8_LIBRARIES} ${PHOTOSPP_LIBRARIES} ${TAUOLAPP_LIBRARIES} AtlasHepMCLib AtlasHepMCsearchLib CxxUtils ${EVTGEN_LIBRARIES} GaudiKernel GeneratorObjects ${ROOT_LIBRARIES} StoreGateLib GeneratorModulesLib
                   PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} AthenaKernel PathResolver )
else()
  atlas_add_library( EvtGen_iLib
                   ${EvtGen_iLib_SOURCES}
                   PUBLIC_HEADERS EvtGen_i
                   DEFINITIONS -DEVTGEN_PYTHIA -DEVTGEN_PHOTOS -DEVTGEN_TAUOLA
                   INCLUDE_DIRS ${HEPPDT_INCLUDE_DIRS} ${PYTHIA8_INCLUDE_DIRS} ${PHOTOSPP_INCLUDE_DIRS} ${TAUOLAPP_INCLUDE_DIRS}  ${EVTGEN_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                   PRIVATE_INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                   PRIVATE_DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${HEPPDT_LIBRARIES} ${PYTHIA8_LIBRARIES} ${PHOTOSPP_LIBRARIES} ${TAUOLAPP_LIBRARIES} AtlasHepMCLib CxxUtils ${EVTGEN_LIBRARIES} GaudiKernel GeneratorObjects ${ROOT_LIBRARIES} StoreGateLib GeneratorModulesLib
                   PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} AthenaKernel PathResolver )
endif()


atlas_add_component( EvtGen_i
                     src/components/*.cxx
                     LINK_LIBRARIES EvtGen_iLib )

# Install files from the package:
atlas_install_joboptions( share/common/*.py )
atlas_install_runtime( share/file/*.DEC share/file/*.table share/file/*.dec share/file/*.pdt share/file/*.dat )

set( EvtGenEnvironment_DIR ${CMAKE_CURRENT_SOURCE_DIR}
   CACHE PATH "Location of EvtGenEnvironment.cmake" )
find_package( EvtGenEnvironment )
