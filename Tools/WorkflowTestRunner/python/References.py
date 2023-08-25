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
    "s3761": "v2",
    "s3779": "v1",
    "s4005": "v1",
    "s4006": "v1",
    "s4007": "v1",
    "s4008": "v1",
    # Overlay
    "d1590": "v1",
    "d1726": "v1",
    "d1759": "v3",
    # Reco
    "q442": "v13",
    "q443": "v12",
    "q445": "v16",
    "q449": "v15",
    # Derivations
    "data_PHYS_Run2": "v8",
    "data_PHYS_Run3": "v8",
    "mc_PHYS_Run2": "v9",
    "mc_PHYS_Run3": "v10",
}
