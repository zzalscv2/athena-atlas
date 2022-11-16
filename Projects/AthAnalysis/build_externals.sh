#!/bin/bash
#
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# Script building all the externals necessary for AthAnalysis.
#

# Set up the variables necessary for the script doing the heavy lifting.
ATLAS_PROJECT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)
ATLAS_EXT_PROJECT_NAME="AthAnalysisExternals"
ATLAS_BUILDTYPE="RelWithDebInfo"
ATLAS_EXTRA_CMAKE_ARGS=(-DLCG_VERSION_NUMBER=101
                        -DLCG_VERSION_POSTFIX="_ATLAS_26"
                        -DATLAS_GAUDI_SOURCE="URL;https://gitlab.cern.ch/atlas/Gaudi/-/archive/v36r6.003/Gaudi-v36r6.003.tar.gz;URL_MD5;e7b279804438a7c68b190084fe2592a7"
                        -DATLAS_ONNXRUNTIME_USE_CUDA=FALSE)
ATLAS_EXTRA_MAKE_ARGS=()

# Let "the common script" do all the heavy lifting.
source "${ATLAS_PROJECT_DIR}/../../Build/AtlasBuildScripts/build_project_externals.sh"
