# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

import re
from AthenaCommon.Logging import logging

#Helper method to determine if the input release is in particular release range. 
#The boundaries rel1 and rel2 are inclusive.

def releaseInRange(flags,rel1,rel2):
    msg=logging.getLogger("releaseInRange")


    #For the records: This regex matches 3 and 4 digit release numbers
    #relPattern=re.compile("^Athena-(\d+(\.\d+){2,3}(\.\*)?)$")
    #But in reality we should require AOD fixes only for three-digit release, used at the Tier0, like: Athena-22.0.89
    relPattern=re.compile("^Athena-[0-9]{1,2}\.[0-9]{1,2}\.[0-9]{1,2}$")  # noqa: W605

    for r in (rel1,rel2):
        if not relPattern.match(r):
            raise RuntimeError("Release number %s doesn't match the expected format"%r)

    inputRelease=flags.Input.Release
    if not relPattern.match(inputRelease):
        raise RuntimeError("Input release number %s doesn't match the expected format"%inputRelease)
    

    #By Atlas convention, the first number denotes the major release, the second one the purpose (Tier0, Generation, ... ) 
    #and the third one is the running version number. 
    #We request that the first two numbers are identical and the third one in range

    i1=rel1.rfind(".")
    i2=rel2.rfind(".")
    if rel1[:i1] != rel2[:i2]:
        raise RuntimeError("Boundary releases not from the same release series, got %s and %s"%(rel1,rel2))

    iRel=inputRelease.rfind(".")
    if rel1[:i1] != inputRelease[:iRel]:
        msg.info("Input release not from the same release series.")
        return False

    #convert last number to into for comparison
    lower=int(rel1[i1+1])
    upper=int(rel2[i2+1])
    current=int(inputRelease[iRel+1])
    
    if (lower > upper): 
        raise RuntimeError("Lower boundary releases %s larger then upper boundary release %s" % (rel1,rel2))


    if (current >= lower and current <= upper):
        return True
    else:
        return False

    

