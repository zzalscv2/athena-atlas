# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#####
# CI Reference Files Map
#####

# The top-level directory for the files is /eos/atlas/atlascerngroupdisk/data-art/grid-input/WorkflowReferences/
# Then the subfolders follow the format branch/test/version, i.e. for s3760 in master the reference files are under
# /eos/atlas/atlascerngroupdisk/data-art/grid-input/WorkflowReferences/main/s3760/v1 for v1 version

# Format is "test" : "version"
references_map = {
    # Simulation
    "s3761": "v9",
    "s3779": "v2",
    "s4005": "v6",
    "s4006": "v9",
    "s4007": "v7",
    "s4008": "v1",
    "a913": "v4",
    # Overlay
    "d1590": "v3",
    "d1726": "v5",
    "d1759": "v9",
    # Reco
    "q442": "v38",
    "q443": "v29",
    "q445": "v49",
    "q449": "v53",
    # Derivations
    "data_PHYS_Run2": "v14",
    "data_PHYS_Run3": "v13",
    "mc_PHYS_Run2": "v17",
    "mc_PHYS_Run3": "v17",
}
