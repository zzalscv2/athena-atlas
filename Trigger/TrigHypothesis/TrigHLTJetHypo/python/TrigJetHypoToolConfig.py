# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.SystemOfUnits import GeV

from TriggerMenuMT.HLT.Config.ControlFlow.HLTCFTools import NoHypoToolCreated
from TrigHLTJetHypo.hypoConfigBuilder import hypotool_from_chaindict
from TrigHLTJetHypo.TrigJetHypoMonitoringConfig import TrigJetHypoToolMonitoring

from AthenaCommon.Logging import logging
logger = logging.getLogger(__name__)


import os
debug = 'JETHYPODEBUG' in os.environ
if debug:
    from AthenaCommon.Constants import DEBUG
    logger.setLevel(DEBUG)


def trigJetHypoToolFromDict(flags, chain_dict):
    
    from DecisionHandling.TrigCompositeUtils import isLegId, getLegIndexInt
    chain_name = chain_dict['chainName']
    chain_mg   = chain_dict['monGroups']
    jet_signature_identifiers = ['Jet', 'Bjet']

    if isLegId(chain_name):
        # For multi-leg chains which include jet legs we have a -- SPECIAL BEHAVIOUR --
        # We instantiate a HypoTool only for the *first* jet leg, whichever leg that happens to be in the chain
        # This single HypoTool gets configured to perform the selection for _all_ of the jet legs, and to report
        # the per-jet passing status for all of these legs.
        #
        # Here we determine if this is the 2nd+ jet leg of a multi-leg chain which has jet legs, and return no tool if it is

        # Can we fetch this from elsewhere?

        leg_id = getLegIndexInt(chain_name)
        # CHECK: If we have called trigJetHypoToolFromDict, then the chain_dict['signatures'] list must contain at minimum one entry from the jet_signature_identifiers list. 
        if not any(signature in chain_dict['signatures'] for signature in jet_signature_identifiers):
            raise Exception("[trigJetHypoToolFromDict] No {} in {} for chain {}. Please update this list of jet signatures.".format(tuple(jet_signature_identifiers),tuple(chain_dict['signatures']),chain_name))

        # CHECK: All Jet and Bjet legs (i.e. signatures from jet_signature_identifiers) must be contiguous
        # (this check is probable best put somewhere else?)
        status = 0
        for entry in chain_dict['signatures']:
            if status == 0 and entry in jet_signature_identifiers:
                status = 1
            elif status == 1 and entry not in jet_signature_identifiers:
                status = 2
            elif status == 2 and entry in jet_signature_identifiers:
                raise Exception("[trigJetHypoToolFromDict] All {} legs should be contiguous in the signatures list, modify the ordering of the chain {}. Signatures:{}.".format(tuple(jet_signature_identifiers),chain_name, tuple(chain_dict['signatures'])))
        
        # CHECK: The leg_id must correspond to a Signature from jet_signature_identifiers. At the time of implementation, this is not guaranteed and can be affected by alignment.
        # If this check fails for any chain, then we need to look again at how the legXXX ordering maps to the chain_dict['signatures'] ordering.
        if not any(signature in chain_dict['signatures'][leg_id] for signature in jet_signature_identifiers):
            raise Exception("[trigJetHypoToolFromDict] For this code to work for chain {}, the signature at index {} must be one of {}. But the signature list is: {}".format(chain_name,leg_id,tuple(jet_signature_identifiers),tuple(chain_dict['signatures'])))

        # Locate the first index within chain_dict['signatures'] which contains an signature listed in jet_signature_identifiers
        first_leg_index = 999
        for signature in jet_signature_identifiers:
            if signature in chain_dict['signatures']:
                first_leg_index = min(first_leg_index, chain_dict['signatures'].index(signature))

        if leg_id > first_leg_index:
            logger.debug("Not returning a HypoTool for %s as this is not the first leg "
                         "with any of %s (leg signatures are %s)",
                         chain_name, tuple(jet_signature_identifiers), tuple(chain_dict['signatures']))
            raise NoHypoToolCreated("No HypoTool created for %s" % chain_name)

    logger.debug("Returning a HypoTool for %s as this is the first leg with any of %s (leg signatures are %s)",
                 chain_name, tuple(jet_signature_identifiers), tuple(chain_dict['signatures']))

    hypo_tool =  hypotool_from_chaindict(chain_dict, debug)

    #if menu has chain in an online monitoring group, unpack the recoalg(s) and hyposcenario(s) to configure monitoring
    if any('jetMon:online' in group for group in chain_mg):
        cpl = chain_dict["chainParts"] 
        histFlags = []
        for cp in cpl:
            histFlags += [ cp['recoAlg'] ] + [ cp['hypoScenario']] 
        hypo_tool.MonTool = TrigJetHypoToolMonitoring(flags, "HLTJetHypo/"+chain_name, histFlags)
    return hypo_tool

    
def  trigJetTLAHypoToolFromDict(chain_dict):
    return  CompFactory.TrigJetTLAHypoTool(chain_dict['chainName'])

def  trigJetEJsHypoToolFromDict(chain_dict):
    if len(chain_dict['chainParts']) > 1:
        raise Exception("misconfiguration of emerging jet chain")

    if len(chain_dict['chainParts'][0]['exotHypo']) > 0:
        exot_hypo = chain_dict['chainParts'][0]['exotHypo'][0]
    else:
        raise Exception("Unable to extract exotHypo emerging jet configuration from chain dict")

    if 'emerging' in exot_hypo:
        trackless = int(0)
        ptf = float(exot_hypo.split('PTF')[1].split('dR')[0].replace('p', '.'))
        dr  = float(exot_hypo.split('dR')[1].split('_')[0].replace('p', '.'))
    elif 'trackless' in exot_hypo:
        trackless = int(1)
        ptf = 0.0
        dr = float(exot_hypo.split('dR')[1].split('_')[0].replace('p', '.'))
    else:
        raise Exception("misconfiguration of emerging jet chain")

    chain_name = chain_dict['chainName']

    hypo = CompFactory.TrigJetEJsHypoTool(chain_name)
    hypo.PTF       = ptf
    hypo.dR        = dr
    hypo.Trackless = trackless

    return  hypo

def  trigJetCRHypoToolFromDict(chain_dict):
    chain_name = chain_dict['chainName']
 
    doBIBrm = int(0)
    if 'calratio' in chain_dict['chainParts'][0]['exotHypo'] or 'calratiormbib' in chain_dict['chainParts'][0]['exotHypo']:
        if 'calratiormbib' in chain_dict['chainParts'][0]['exotHypo']:
            doBIBrm = int(1)
    else:
        raise Exception("misconfiguration of Exotic jet chain")

    hypo = CompFactory.TrigJetCRHypoTool(chain_name)

    hypo.MinjetlogR      = 1.2
    hypo.MintrackPt      = 2*GeV
    hypo.MindeltaR       = 0.2

    hypo.countBIBcells   = 4

    hypo.doBIBremoval = doBIBrm


    return  hypo

import unittest
class TestStringMethods(unittest.TestCase):
    def testValidConfigs(self):
        from TriggerMenuMT.HLT.Config.Utility.DictFromChainName import (
            dictFromChainName,)

        chain_names = (
            'HLT_j0_FBDJNOSHARED10etXX20etXX34massXX50fbet_L1J20',)

        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()
        flags.Input.Files = []

        flags.lock()

        wid = max(len(c) for c in chain_names)
        for chain_name in chain_names:
            chain_dict = dictFromChainName(flags, chain_name)
            tool = trigJetHypoToolFromDict(flags, chain_dict)
            self.assertIsNotNone(tool)
            logger.debug(chain_name.rjust(wid), str(tool))



if __name__ == '__main__':
    unittest.main()

    # other local tests have been moved to testChainDictMaker.py
