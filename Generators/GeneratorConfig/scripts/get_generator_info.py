#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from PyJobTransforms.trfExceptions import TransformAMIException
from PyJobTransforms.trfAMI import AMIerrorCode, getAMIClient, get_ami_tag

import logging
msg = logging.getLogger(__name__)

import os
import sys
tag = sys.argv[1]

try:
    import pyAMI.exception
except ImportError as e:
    raise TransformAMIException(AMIerrorCode, 'Import of pyAMI modules failed ({0})'.format(e))

try:
    amiclient = getAMIClient()
    result = get_ami_tag(amiclient, tag, True)
except pyAMI.exception.Error as e:
    msg.warning('An exception occured when connecting to primary AMI: {0}'.format(e))
    msg.error('Exception: {0}'.format(e))
    if 'please login' in e.message or 'certificate expired' in e.message:
        raise TransformAMIException(AMIerrorCode, 'Getting tag info from AMI failed with credential problem. '
                                    'Please check your AMI account status.')
    if 'Invalid amiTag' in e.message:
        raise TransformAMIException(AMIerrorCode, 'Invalid AMI tag ({0}).'.format(tag))
        
    msg.error("Error may not be fatal - will try AMI replica catalog")

build, version = result[0]["SWReleaseCache"].split("_")
print(build)
print(version)

folder_path = os.path.dirname(os.path.realpath(__file__))
command = f"{folder_path}/get_generator_versions.sh {build} {version} {tag}"
import subprocess
output, error = subprocess.Popen(["/bin/bash", "-c", command], stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
output, error = output.decode("utf-8"), error.decode("utf-8")
# sanitize error
error = [l for l in error.strip().split("\n") if l and "manpath" not in l]

output = output.strip()
print(output)

if error:
    print(error)
    sys.exit(1)
