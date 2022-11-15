#!/bin/bash
#
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# Script building all the externals necessary for AthDerivation.
#

# Set up the variables necessary for the script doing the heavy lifting.
ATLAS_PROJECT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)
ATLAS_EXT_PROJECT_NAME="AthDerivationExternals"
ATLAS_BUILDTYPE="RelWithDebInfo"
ATLAS_EXTRA_CMAKE_ARGS=(-DLCG_VERSION_NUMBER=101
                        -DLCG_VERSION_POSTFIX="_ATLAS_26"
                        -DATLAS_GAUDI_SOURCE="URL;https://gitlab.cern.ch/atlas/Gaudi/-/archive/v36r6.003/Gaudi-v36r6.003.tar.gz;URL_MD5;e7b279804438a7c68b190084fe2592a7"
                        -DATLAS_ACTS_SOURCE="URL;https://github.com/acts-project/acts/archive/refs/tags/v20.3.0.tar.gz;URL_MD5;2f4d7c2b04495ec5f4ea2ed30c419770"
                        -DATLAS_GEOMODEL_SOURCE="URL;https://gitlab.cern.ch/GeoModelDev/GeoModel/-/archive/4.4.0/GeoModel-4.4.0.tar.bz2;URL_MD5;c339b961e1d2f0c48b9807314568c8c2")
ATLAS_EXTRA_MAKE_ARGS=()

# Let "the common script" do all the heavy lifting.
source "${ATLAS_PROJECT_DIR}/../../Build/AtlasBuildScripts/build_project_externals.sh"
