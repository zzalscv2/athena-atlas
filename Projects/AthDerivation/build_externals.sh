#!/bin/bash
#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# Script building all the externals necessary for AthDerivation.
#

# Set up the variables necessary for the script doing the heavy lifting.
ATLAS_PROJECT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)
ATLAS_EXT_PROJECT_NAME="AthDerivationExternals"
ATLAS_BUILDTYPE="RelWithDebInfo"
ATLAS_EXTRA_CMAKE_ARGS=(-DLCG_VERSION_NUMBER=102
                        -DLCG_VERSION_POSTFIX="b_ATLAS_21"
                        -DATLAS_GAUDI_SOURCE="URL;https://gitlab.cern.ch/atlas/Gaudi/-/archive/v36r14.000/Gaudi-v36r14.000.tar.gz;URL_MD5;1cc75461a7cdc87daf1f9b6cb48725b4"
			-DATLAS_ACTS_SOURCE="URL;https://github.com/acts-project/acts/archive/refs/tags/v26.0.0.tar.gz;URL_MD5;6848960f4b75711144e163c4d1b04c8b")
ATLAS_EXTRA_MAKE_ARGS=()

# Let "the common script" do all the heavy lifting.
source "${ATLAS_PROJECT_DIR}/../../Build/AtlasBuildScripts/build_project_externals.sh"
