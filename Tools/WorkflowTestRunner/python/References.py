# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#####
# CI Reference Files Map
#####

# The top-level directory for the files is /eos/atlas/atlascerngroupdisk/data-art/grid-input/WorkflowReferences/
# Then the subfolders follow the format branch/test/version, i.e. for s3760 in master the reference files are under
# /eos/atlas/atlascerngroupdisk/data-art/grid-input/WorkflowReferences/master/s3760/v1 for v1 version

# Format is "test" : "version"
references_map = {
    # Simulation
    "s3761": "v5",
    "s3779": "v5",
    "s4005": "v3",
    "s4006": "v5",
    "s4007": "v5",
    "s4008": "v3",
    # Overlay
    "d1590": "v14",
    "d1726": "v10",
    "d1759": "v18",
    # Reco
    "q442": "v4",
    "q443": "v6",
    "q445": "v7",
    "q449": "v4",
}
