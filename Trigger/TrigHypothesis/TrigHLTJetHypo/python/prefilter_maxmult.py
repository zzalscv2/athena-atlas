# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from TrigHLTJetHypo.FastReductionAlgToolFactory import toolfactory

from AthenaCommon.Logging import logging
from AthenaCommon.Constants import DEBUG

import re

logger = logging.getLogger( __name__)
logger.setLevel(DEBUG)

pattern = r'^MAXMULT(?P<end>\d+)$'

rgx = re.compile(pattern)


def prefilter_maxmult(pf_string):
    """calculate the parameters needed to generate a RangeFilter config 
    AlgTool starting from the prefilter substring if it appears in the 
    chain dict"""

    assert pf_string.startswith('MAXMULT'),\
        'routing error, module %s: bad prefilter %s' % (__name__, pf_string)

    m = rgx.match(pf_string)
    groupdict = m.groupdict()

    vals = {}
    vals['end'] = int(groupdict['end']) 

    toolclass, name =  toolfactory('MaxMultFilterConfigTool')
    vals['name'] = name
    
    return toolclass(**vals)

