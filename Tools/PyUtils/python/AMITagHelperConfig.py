# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""Utilities to get and set the AMITag in the metadata

The input AMITag is contained in the AMITag attribute of the /TagInfo in-file
metadata object.

The AMITag of the current processing step is provided by command line argument
to the transform.

The tool provides a function to get and check the AMITags from the input and
add the output AMITag, configuring the TagInfoMgr with the information.

    Usage:
        AMITagHelperConfigg.SetAMITag(ConfigFlags, runArgs)
"""
from AthenaCommon.Logging import logging
log = logging.getLogger('AMITagHelper')

def InputAMITags(ConfigFlags):
    """Returns AMITag of input, split in a list, e.g. ['e6337','s3681','r13145']

    Looks up AMITag of input from ConfigFlag input file's in-file
    metadta. If nothing can be retrieved from the in-file metadata return empty
    list, otherwise return value from metadata. Inform about differences.
    """
    from AthenaConfiguration.AutoConfigFlags import GetFileMD
    tags = GetFileMD(ConfigFlags.Input.Files).get('AMITag', '')

    if len(tags)>0:
        tags = tags.split('_')
        log.info(f'Read AMITag from metadata: {tags}')
    else:
        log.info("Cannot access /TagInfo/AMITag from in-file metadata")
        # Set it to an empty list
        tags = []

    return tags


def SetAMITag(ConfigFlags, runArgs=None):
    """Add input and output AMITag values and set result in in-file metadata

    The AMITags will be combined with '_' as delimiters. The result is set in
    the in-file metadata.
    """
    tags = InputAMITags(ConfigFlags)
    if runArgs and hasattr(runArgs,'AMITag'):
        if runArgs.AMITag not in tags:
            tags += [runArgs.AMITag]
            log.info(f'Adding AMITag from execution: {runArgs.AMITag}')

    amitag = '_'.join(tags)

    from EventInfoMgt.TagInfoMgrConfig import TagInfoMgrCfg
    return TagInfoMgrCfg(ConfigFlags, tagValuePairs={'AMITag':amitag})
