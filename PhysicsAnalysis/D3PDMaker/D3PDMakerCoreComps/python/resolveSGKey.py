# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# @file D3PDMakerCoreComps/python/resolveSGKey.py
# @author scott snyder <snyder@bnl.gov>
# @date Jan, 2010
# @brief Pick proper SG key out of a list from ObjKeyStore.
#


from AthenaCommon.Logging    import logging


def resolveSGKey (flags, keystr):
    """Pick proper SG key out of a list from ObjKeyStore.
    
KEYSTR is a comma-separated list of StoreGate keys.
Return the first one from that list that exists in the input.
Raise an exception if none of them exist.
"""

    log = logging.getLogger ('D3PD')
    kl = keystr.split(',')
    for k in kl:
        if k in flags.Input.Collections:
            log.verbose ("Using SG key %s for type %s." % (k, type))
            return k
    if len (kl) == 1:
        # Just one, hope for the best.
        return k

    raise Exception ("No keys among `%s' for type `%s' in ObjKeyStore." %
                     (keystr, type))



def testSGKey (flags, keystr):
    """Test to see if SG keys are in the input.
    
KEYSTR is a comma-separated list of StoreGate keys.
Return true if any key from that list exists in the input.
"""

    kl = keystr.split(',')
    for k in kl:
        if k in flags.Input.Collections:
            return True
    return False

    
