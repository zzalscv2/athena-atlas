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
    "s3761": "v13",
    "s3779": "v8",
    "s4005": "v9",
    "s4006": "v12",
    "s4007": "v12",
    "s4008": "v4",
    # Overlay
    "d1590": "v16",
    "d1726": "v12",
    "d1759": "v24",
    # Reco
    "q442": "v17",
    "q443": "v17",
    "q445": "v23",
    "q449": "v29",
}
