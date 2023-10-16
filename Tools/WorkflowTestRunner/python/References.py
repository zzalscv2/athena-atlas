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
    "s3761": "v5",
    "s3779": "v2",
    "s4005": "v3",
    "s4006": "v3",
    "s4007": "v3",
    "s4008": "v1",
    "a913": "v1",
    # Overlay
    "d1590": "v1",
    "d1726": "v1",
    "d1759": "v3",
    # Reco
    "q442": "v25",
    "q443": "v22",
    "q445": "v30",
    "q449": "v34",
    # Derivations
    "data_PHYS_Run2": "v11",
    "data_PHYS_Run3": "v10",
    "mc_PHYS_Run2": "v14",
    "mc_PHYS_Run3": "v14",
}
