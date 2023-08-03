# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# CI test definitions for the AnalysisBase project
# --> README.md before you modify this file
#

atlas_add_citest( AnalysisTop_EMPFlowData
   LOG_IGNORE_PATTERN "Cannot interpolate outside histogram domain" # ANALYSISTO-1165
   SCRIPT CI_EMPFlowDatatest.py )

atlas_add_citest( AnalysisTop_EMPFlowMC
   SCRIPT CI_EMPFlowMCtest.py )
