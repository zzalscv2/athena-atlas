# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Production r-tags for different campaigns
reco_campaigns = { 
    'mc16a': ['r9280', 'r9287', 'r9364'],
    'mc16d': ['r10069', 'r10201', 'r10211','r10212'],
    'mc16e': ['r10724'],
    'mc20a': ['r13167'],
    'mc20d': ['r13144'],
    'mc20e': ['r13145'],
    'mc21a': ['r13768', 'r13829'],
}

# Function to get the campaign
def getMCCampaign(ami_tag=None, project_name=None, files=None):
    # Auto-configure from file
    if files is not None and ami_tag is None and project_name is None:
        from AthenaConfiguration.AutoConfigFlags import GetFileMD
        metadata = GetFileMD(files)
        ami_tag = metadata.get('AMITag', '')
        project_name = metadata.get('project_name', '')

    assert ami_tag is not None
    assert project_name is not None

    for c in reco_campaigns:
        for r in reco_campaigns[c]:
            if r in ami_tag:
                return c

    # MC-equivalent projects for data
    if 'data18' in project_name:
        return 'mc20e'
    elif 'data17' in project_name:
        return 'mc20d'
    elif 'data16' in project_name or 'data15' in project_name:
        return 'mc20a'

    return None
