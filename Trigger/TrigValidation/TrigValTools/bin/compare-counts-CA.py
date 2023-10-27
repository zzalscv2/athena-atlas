#!/usr/bin/env python3
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
import json
import argparse
from glob import glob

"""
This script can be used to diff the HLTChain.txt and HLTStep.txt count reports
taking into account which trigger chains have been partially or fully migrated
to ComponentAccumulator.

Input files can be taken e.g. from ART tests:
 - trigAna_RDOtoRDOTrig_v1Dev_build vs trigAna_RDOtoRDOTrig_v1Dev_newJO_build
 - test_trigP1_v1PhysP1_newJO_build vs test_trigP1_v1PhysP1_build
 - trigP1_v1Dev_newJO_build vs trigP1_v1Dev_build
See 'expert page' on https://test-atrvshft.web.cern.ch/test-atrvshft/ART_monitor/

The input dirs should contain a single HLT menu,
and the HLTChain.txt and HLTStep.txt files. 
"""

def add_args(parser):
    parser.add_argument('legacy', type=str, help='Directory holding legacy HLT menu and counts to read')
    parser.add_argument('CA', type=str, help='Directory holding CA HLT menu and counts to read')


def find_unique_file(dir,filename):
    fpath = glob(f'{dir}/{filename}')
    assert len(fpath)==1, f"Did not find unique matching {filename} in legacy dir"
    return fpath[0]


def extract_chainsteps(menu_path):
    menuf = open(menu_path)
    menu = json.load(menuf)

    def remove_empty(steps):
        return [s for s in steps if s]

    return {
        c: remove_empty(info['sequencers']) for c,info in menu['chains'].items()
    }


# Tries to additionally keep the first non-migrated legacy step
def get_common_steps(steps1,steps2,ignore_step_number):
    if ignore_step_number:
        # We need to map the legacy steps to the CA
        # because menu alignment in an incomplete menu
        # may result in offsets
        chopped1 = [s.split('_',1)[1] for s in steps1]
        chopped2steps = {s.split('_',1)[1]:s for s in steps2}
        merged1 = zip(chopped1,steps1)

        common_steps = {}
        first_missing_CA = True
        for short,long in merged1:
            if short in chopped2steps:
                common_steps[long] = chopped2steps[short]
            elif first_missing_CA:
                common_steps[long] = "N/A"
                first_missing_CA = False
            else:
                break
    else:
        # Simpler code for when we think everything lines up
        common_steps = []
        first_missing_CA = True
        for s in steps1:
            if s in steps2:
                common_steps.append(s)
            elif first_missing_CA:
                common_steps.append(s)
                first_missing_CA = False
            else:
                break
    return common_steps


def get_common_chainsteps(legacy,ca,ignore_step_number):
    # All CA chains should exist in the legacy menu
    for chain in ca:
        assert chain in legacy
    common_chainsteps = {}
    for chain, steps in legacy.items():
        if chain in ca:
            common_chainsteps[chain] = get_common_steps(steps,ca[chain],ignore_step_number)
    return common_chainsteps


def compare_chain_counts(chain_counts_path_leg,chain_counts_path_CA,common_chainsteps):

    fchain_counts_leg = open(chain_counts_path_leg)
    fchain_counts_CA = open(chain_counts_path_CA)

    def parse_chain_counts(fcounts, common_chainsteps):
        counts = {}
        for l in fcounts:
            try:
                chain, count = l.split()
                if chain in common_chainsteps:
                    counts[chain] = int(count)
            except ValueError:
                pass
        return counts

    chain_counts_leg = parse_chain_counts(fchain_counts_leg, common_chainsteps)
    chain_counts_CA = parse_chain_counts(fchain_counts_CA, common_chainsteps)
    
    # Merge the counts
    chain_counts_compared = {
        chain: {
            'legacy': chain_counts_leg[chain],
            'CA': chain_counts_CA[chain],
        } for chain in chain_counts_leg
    }
    return chain_counts_compared


def compare_step_counts(step_counts_path_leg,step_counts_path_CA,common_chainsteps):
    fstep_counts_leg = open(step_counts_path_leg)
    fstep_counts_CA = open(step_counts_path_CA)

    def parse_chain_counts(fcounts, common_chainsteps):
        counts = {}
        # Accumulate counts, indexed by step number in chain
        for l in fcounts:
            try:
                stepname, count = l.split()
                chain, step = stepname.rsplit('_',1)
                stepno = int(step[4:])
                if chain in common_chainsteps:
                    if chain not in counts:
                        counts[chain] = {}
                    counts[chain][stepno] = int(count)
            except ValueError:
                pass

        # Reorder the counts -- the lines are printed alphabetically
        # so that 11 precedes 2
        counts_condensed = {}
        for chain, stepcounts in counts.items():
            ordered_counts = []
            for stepno in range(len(stepcounts)):
                ordered_counts.append(stepcounts[stepno])
            # Retain just the nonzero counts, as absent or empty steps
            # render as 0's
            counts_condensed[chain] = [c for c in ordered_counts if c>0]
            
        return counts_condensed

    step_counts_leg = parse_chain_counts(fstep_counts_leg, common_chainsteps)
    step_counts_CA = parse_chain_counts(fstep_counts_CA, common_chainsteps)
    
    # Merge the counts
    step_counts_compared = {
        chain: {
            'legacy': step_counts_leg[chain],
            'CA': step_counts_CA[chain],
        } for chain in step_counts_leg
    }
    return step_counts_compared


def main():
    parser = argparse.ArgumentParser(
        description='Compare HLT menus to determine CA/Legacy chain differences'
    )
    add_args(parser)
    args = parser.parse_args()

    leg_menu = find_unique_file(args.legacy,"HLTMenu*.json")
    CA_menu = find_unique_file(args.CA,"HLTMenu*.json")

    chainsteps_leg = extract_chainsteps(leg_menu)
    chainsteps_ca = extract_chainsteps(CA_menu)

    # For now we will always assume that the legacy & CA steps
    # could get misaligned. The retrieved mapping also
    # contains the first legacy step not reflected in CA
    common_chainsteps = get_common_chainsteps(chainsteps_leg, chainsteps_ca, ignore_step_number=True)

    ########### Compare chain event counts
    chain_counts_path_leg = f'{args.legacy}/HLTChain.txt'
    chain_counts_path_CA = f'{args.CA}/HLTChain.txt'

    chain_counts_compared = compare_chain_counts(chain_counts_path_leg,chain_counts_path_CA,common_chainsteps)
    json.dump(
        chain_counts_compared,
        open('chain_count_legacy_CA.json','w'),
        indent=4
    )
    
    # Print a text summary of mismatches
    fchaindiff = open('HLTChain_diff.txt','w')
    print("Chain counts disagreeing between CA and legacy configs:\n-----",file=fchaindiff)
    print(f"{'Chain name':<120} | legacy | CA",file=fchaindiff)
    for chain, comp in chain_counts_compared.items():
        if comp['legacy'] != comp['CA']:
            print(f"{chain:<120} | {comp['legacy']:6} | {comp['CA']}",file=fchaindiff)

    ########### Compare step event counts
    step_counts_path_leg = f'{args.legacy}/HLTStep.txt'
    step_counts_path_CA = f'{args.CA}/HLTStep.txt'

    step_counts_compared = compare_step_counts(step_counts_path_leg,step_counts_path_CA,common_chainsteps)
    json.dump(
        step_counts_compared,
        open('step_count_legacy_CA.json','w'),
        indent=4
    )
    
    # Print a display of mismatches
    fstepdiff = open('HLTStep_diff.txt','w')
    print("Step counts disagreeing between CA and legacy configs:\n-----",file=fstepdiff)
    print(f"{'Chain name':<80} | {'Legacy step':<50} | legacy | {'CA step':<50} | CA",file=fstepdiff)
    for chain, comp in step_counts_compared.items():
        # Need to a
        if comp['legacy'] != comp['CA']:
            lengthdiff = len(comp['legacy'])-len(comp['CA'])
            if lengthdiff==0:
                leg_padded = comp['legacy']
                CA_padded = comp['CA']
            elif lengthdiff>0:
                leg_padded = comp['legacy']
                CA_padded = comp['CA'] + [0]*lengthdiff
            elif lengthdiff<0:
                leg_padded = comp['legacy'] + [0]*abs(lengthdiff)
                CA_padded = comp['CA']
            stepnames = [(k,v) for k,v in common_chainsteps[chain].items()]
            for index, (legcount, CAcount) in enumerate(zip(leg_padded,CA_padded)):
                step_leg, step_CA = stepnames[index] if index < len(stepnames) else ('...', '...')
                print(f"{chain if index==0 else '':<80} | {step_leg:<50} | {legcount:6} | {step_CA:<50} | {CAcount}",file=fstepdiff)


if __name__=='__main__':
    main()
