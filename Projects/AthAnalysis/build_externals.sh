#!/bin/bash
#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# Script building all the externals necessary for AthAnalysis.
#

# Set up the variables necessary for the script doing the heavy lifting.
ATLAS_PROJECT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)
ATLAS_EXT_PROJECT_NAME="AthAnalysisExternals"
ATLAS_BUILDTYPE="RelWithDebInfo"
ATLAS_EXTRA_CMAKE_ARGS=(-DLCG_VERSION_NUMBER=102
                        -DLCG_VERSION_POSTFIX="b_ATLAS_9"
                        -DATLAS_GAUDI_SOURCE="URL;https://gitlab.cern.ch/atlas/Gaudi/-/archive/v36r9.000/Gaudi-v36r9.000.tar.gz;URL_MD5;b8e6bfdf2a997a5b50ae5b1c1df5aa52"
                        -DATLAS_CLHEP_SOURCE="URL;https://gitlab.cern.ch/atlas-sw-git/CLHEP/-/archive/CLHEP_2_4_1_3_atl04/CLHEP-CLHEP_2_4_1_3_atl04.tar.gz;URL_MD5;d2f9ea9f3368d2dd0321457a222c3f7e"
                        -DATLAS_ONNXRUNTIME_USE_CUDA=FALSE)
ATLAS_EXTRA_MAKE_ARGS=()

# Let "the common script" do all the heavy lifting.
source "${ATLAS_PROJECT_DIR}/../../Build/AtlasBuildScripts/build_project_externals.sh"
