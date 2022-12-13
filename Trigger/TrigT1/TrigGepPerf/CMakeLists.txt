# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package
atlas_subdir( TrigL0GepPerf )


atlas_add_component( TrigL0GepPerf 
		     src/*.cxx
		     src/components/*.cxx
  		     LINK_LIBRARIES
		     AthenaBaseComps
		     GaudiKernel
		     AthContainers
		     xAODEventInfo
		     xAODTrigL1Calo
		     xAODTruth
		     xAODMissingET
		     xAODJet
		     xAODEgamma
		     xAODTrigCalo
		     xAODBase
		     xAODCore
		     CaloDetDescrLib
		     xAODJet
		     xAODTrigger
		     CaloIdentifier
		     # AthAnalysisBaseCompsLib
		     PathResolver
		     TrigT1Interfaces
		     TrigEFMissingETLib
)

# Install python modules, joboptions, and share content
atlas_install_python_modules( python/*.py )
atlas_install_joboptions( share/*.py )



