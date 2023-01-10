# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package
atlas_subdir( TrigGepPerf )

# External package dependencies:
find_package( ROOT COMPONENTS Core Math Physics )

atlas_add_component( TrigGepPerf
		     src/*.cxx
		     src/components/*.cxx
             INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
             LINK_LIBRARIES ${ROOT_LIBRARIES} AthContainers AthenaBaseComps CaloConditions CaloDetDescrLib CaloEvent CaloIdentifier GaudiKernel TrigEFMissingETLib xAODCaloEvent xAODEventInfo xAODJet xAODTrigger )

# Install python modules, joboptions, and share content
atlas_install_python_modules( python/*.py )
atlas_install_joboptions( share/*.py )



