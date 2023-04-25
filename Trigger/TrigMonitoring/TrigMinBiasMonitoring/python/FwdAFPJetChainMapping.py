# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

afp_plus_jet_chains = [
    # Phase-I
    'HLT_2j120_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_jJ90',
    'HLT_2j175_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_jJ125',
    'HLT_2j120_mb_afprec_afpdijet_L1CEP-CjJ90',
    'HLT_2j135_mb_afprec_afpdijet_L1CEP-CjJ100',
    'HLT_j20_L1AFP_A_OR_C_jJ20',
    'HLT_j20_L1AFP_A_OR_C_jJ30',
    'HLT_j20_L1AFP_A_AND_C_jJ20',
    'HLT_j20_L1AFP_A_AND_C_jJ30',
    'HLT_mb_afprec_L1AFP_A_AND_C_TOF_jJ50',
    'HLT_mb_afprec_L1AFP_A_AND_C_TOF_jJ60',
    'HLT_mb_afprec_L1AFP_A_AND_C_TOF_jJ90',
    'HLT_mb_afprec_L1AFP_A_AND_C_TOF_jJ125',
    'HLT_mb_afprec_L1AFP_A_AND_C_TOF_TOT1_jJ50',
    'HLT_mb_afprec_L1AFP_A_AND_C_TOF_TOT1_jJ60',
    'HLT_mb_afprec_L1AFP_A_AND_C_TOF_TOT1_jJ90',
    'HLT_mb_afprec_L1AFP_A_AND_C_TOF_TOT1_jJ125',
    'HLT_mb_afprec_L1CEP-CjJ90',
    'HLT_mb_afprec_L1CEP-CjJ100',
    'HLT_noalg_L1AFP_A_AND_C_jJ20',
    'HLT_noalg_L1AFP_A_AND_C_jJ30',
    'HLT_noalg_L1AFP_A_OR_C_jJ20',
    'HLT_noalg_L1AFP_A_OR_C_jJ30',

    # Legacy
    'HLT_2j120_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_J50',
    'HLT_2j120_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_J75',
    'HLT_j20_L1AFP_A_OR_C_J12',
    'HLT_j20_L1AFP_A_AND_C_J12',
    'HLT_mb_afprec_L1AFP_A_AND_C_TOF_J20',
    'HLT_mb_afprec_L1AFP_A_AND_C_TOF_J30',
    'HLT_mb_afprec_L1AFP_A_AND_C_TOF_J50',
    'HLT_mb_afprec_L1AFP_A_AND_C_TOF_J75',
    'HLT_mb_afprec_L1AFP_A_AND_C_TOF_TOT1_J20',
    'HLT_mb_afprec_L1AFP_A_AND_C_TOF_TOT1_J30',
    'HLT_mb_afprec_L1AFP_A_AND_C_TOF_TOT1_J50',
    'HLT_mb_afprec_L1AFP_A_AND_C_TOF_TOT1_J75',
    'HLT_noalg_L1AFP_A_AND_C_TOF_J50',
    'HLT_noalg_L1AFP_A_AND_C_TOF_J75',
    'HLT_noalg_L1AFP_A_AND_C_TOF_TOT1_J50',
    'HLT_noalg_L1AFP_A_AND_C_TOF_TOT1_J75',
    'HLT_noalg_L1AFP_A_OR_C_J12',
    'HLT_noalg_L1AFP_A_AND_C_J12',
]

# Reference chains. AFP OR triggers will be replaced with AND if probed chain is also AND.
reference_chains = [
    'HLT_mb_sptrk_pt2_L1RD0_FILLED', 'HLT_mb_sptrk_L1RD0_FILLED', 'HLT_noalg_L1RD0_FILLED',
    'HLT_mb_sptrk_pt2_L1AFP_A_OR_C', 'HLT_noalg_L1AFP_A_OR_C'
]

# Dedicated trigger-reference chain pairs
afp_plus_jet_trig_ref_chains = [
    # Phase-I
    {'trig': 'HLT_2j120_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_jJ90', 'ref': 'HLT_noalg_L1AFP_A_AND_C_jJ30'},
    {'trig': 'HLT_2j175_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_jJ125', 'ref': 'HLT_noalg_L1AFP_A_AND_C_jJ30'},
    {'trig': 'HLT_2j120_mb_afprec_afpdijet_L1CEP-CjJ90', 'ref': 'HLT_noalg_L1CEP-CjJ90'},
    {'trig': 'HLT_2j135_mb_afprec_afpdijet_L1CEP-CjJ100', 'ref': 'HLT_noalg_L1CEP-CjJ100'},
    {'trig': 'HLT_2j120_mb_afprec_afpdijet_L1CEP-CjJ90', 'ref': 'HLT_mb_afprec_L1CEP-CjJ90'},
    {'trig': 'HLT_2j135_mb_afprec_afpdijet_L1CEP-CjJ100', 'ref': 'HLT_mb_afprec_L1CEP-CjJ100'},

    # Legacy
    {'trig': 'HLT_2j120_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_J50', 'ref': 'HLT_noalg_L1AFP_A_AND_C_TOF_J50'},
    {'trig': 'HLT_2j175_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_J75', 'ref': 'HLT_noalg_L1AFP_A_AND_C_TOF_J75'},
    {'trig': 'HLT_2j120_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_J50', 'ref': 'HLT_noalg_L1J50'},
    {'trig': 'HLT_2j175_mb_afprec_afpdijet_L1AFP_A_AND_C_TOF_J75', 'ref': 'HLT_noalg_L1J75'},
]


def GetAFPJetChains(hlt_chains):
    """ Obtain list of AFP+jet chains and trigger-reference pairs which are present in HLT menu. """
    chains = {'all': [], 'trig': [], 'ref': []}

    for chain in afp_plus_jet_chains:
        if chain not in hlt_chains:
            continue

        chains['all'].append(chain)
        for ref in reference_chains:
            if 'A_AND_C' in chain:
                ref = ref.replace('A_OR_C', 'A_AND_C')

            if ref not in hlt_chains:
                continue

            chains['trig'].append(chain)
            chains['ref'].append(ref)

    for chain_ref in afp_plus_jet_trig_ref_chains:
        if chain_ref['trig'] in hlt_chains and chain_ref['ref'] in hlt_chains:
            chains['trig'].append(chain_ref['trig'])
            chains['ref'].append(chain_ref['ref'])

    return chains
