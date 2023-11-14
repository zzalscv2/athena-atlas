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
    "s3761": "v8",
    "s3779": "v2",
    "s4005": "v5",
    "s4006": "v8",
    "s4007": "v6",
    "s4008": "v1",
    "a913": "v3",
    # Overlay
    "d1590": "v2",
    "d1726": "v4",
    "d1759": "v6",
    # Reco
    "q442": "v32",
    "q443": "v24",
    "q445": "v37",
    "q449": "v41",
    # Derivations
    "data_PHYS_Run2": "v12",
    "data_PHYS_Run3": "v11",
    "mc_PHYS_Run2": "v15",
    "mc_PHYS_Run3": "v15",
}
