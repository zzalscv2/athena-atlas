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
    "s3761": "v14",
    "s3779": "v10",
    "s4005": "v10",
    "s4006": "v15",
    "s4007": "v15",
    "s4008": "v5",
    # Overlay
    "d1590": "v17",
    "d1726": "v13",
    "d1759": "v25",
    # Reco
    "q442": "v20",
    "q443": "v19",
    "q445": "v26",
    "q449": "v34",
}
